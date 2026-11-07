#include <Wire.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "gesture_engine.h"
#include "led_feedback.h"
#include "sony_output.h"
#include "usb_maintenance.h"
#include "output_trace.h"
#include <USB.h>
#include <SPI.h>
#include <hardware/watchdog.h>
#include <hardware/gpio.h>
#include <pico/bootrom.h>
#include <string.h>
#include <stdio.h>

#ifndef ENABLE_PICOTOOL_USB
#error "Build controller with uploadmethod=picotool to retain USB maintenance."
#endif

constexpr uint8_t kCs = 26, kRelay = 27, kCamera = 28;
bool interfaceReady = false, suspended = false, learningMode = false, serialLearning = false;
char serialLine[48];
size_t serialLength = 0;
bool serialOverflow = false, hadSerial = false;
uint32_t startupAt = 0;
knob::BootRequest bootRequest;
knob::OutputTrace outputTrace;
uint32_t lastAudioInputAt = 0;
bool audioInputSeen = false;
void recordRelayEdge(bool on);
void reportTiming();

void writePot(uint8_t code) {
  digitalWrite(kRelay, LOW);
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(kCs, LOW);
  SPI.transfer(0x11); SPI.transfer(code);
  digitalWrite(kCs, HIGH);
  SPI.endTransaction();
}
void setRelay(bool on) { digitalWrite(kRelay, on ? HIGH : LOW); recordRelayEdge(on); }
void setCamera(bool on) { digitalWrite(kCamera, on ? HIGH : LOW); }
knob::SonyOutput output(writePot, setRelay, setCamera);
void recordRelayEdge(bool on) {
  const uint32_t now = millis();
  outputTrace.record({now, audioInputSeen ? now - lastAudioInputAt : UINT32_MAX,
                      output.code(), output.pending(), on});
}
TWIST twist;
struct Sample { uint16_t count; bool down; uint8_t major, minor; };
// Explicit declarations prevent Arduino's generated prototypes preceding Sample.
bool readSample(Sample &sample);
void inputEvent(const Sample &sample);
Sample lastSample = {};
const char *sampleError = "none";
uint8_t sampleDetail = 0;
bool connected = false;
knob::LedFeedback feedback;
knob::Color lastLed = knob::kOff;
bool ledKnown = false;
uint32_t lastAttempt = 0, lastPoll = 0, lastHeartbeat = 0, sequence = 0;

bool emitEvent(const char *type, const char *fields) {
  if (!Serial) return false;
  char record[256];
  const int length = snprintf(record, sizeof(record),
      "{\"v\":1,\"type\":\"%s\",\"seq\":%lu,\"ms\":%lu%s}\n",
      type, static_cast<unsigned long>(sequence + 1),
      static_cast<unsigned long>(millis()), fields);
  if (length <= 0 || static_cast<size_t>(length) >= sizeof(record) ||
      Serial.availableForWrite() < length) return false;
  ++sequence;
  return Serial.write(reinterpret_cast<const uint8_t *>(record), length) == static_cast<size_t>(length);
}

void serviceOutputTrace() {
  if (!Serial) { outputTrace.discard(); return; }
  const knob::OutputEdge *edge = outputTrace.front();
  if (!edge) return;
  char fields[180];
  snprintf(fields, sizeof(fields),
      ",\"edge_ms\":%lu,\"on\":%s,\"code\":%u,\"pending\":%lu,\"input_age_ms\":%lu,\"trace_lost\":%lu",
      static_cast<unsigned long>(edge->at), edge->on ? "true" : "false", edge->code, static_cast<unsigned long>(edge->pending),
      static_cast<unsigned long>(edge->inputAge), static_cast<unsigned long>(outputTrace.lost()));
  if (emitEvent("output", fields)) outputTrace.pop();
}

const char *actionName(knob::Action action) {
  switch (action) {
    case knob::Action::VolumeUp: return "volume_up";
    case knob::Action::VolumeDown: return "volume_down";
    case knob::Action::PlayPause: return "play_pause";
    case knob::Action::MuteAtt: return "mute_att";
    case knob::Action::NextTrack: return "next_track";
    case knob::Action::PreviousTrack: return "previous_track";
    case knob::Action::CameraOn: return "camera_on";
    case knob::Action::CameraOff: return "camera_off";
    case knob::Action::LongPressIgnored: return "long_press_ignored";
    case knob::Action::LearningToggle: return "learning_toggle";
  }
  return "unknown";
}

void gestureEvent(const knob::Event &event, void *) {
  uint8_t audioCode;
  if (output.codeFor(event.action, audioCode)) {
    lastAudioInputAt = millis(); audioInputSeen = true;
  }
  // Feedback must work with no serial monitor; fault-cancel events are not gestures.
  if (connected) feedback.onGesture(event, millis());
  bool accepted = false;
  if (event.action == knob::Action::LearningToggle && connected && interfaceReady) {
    learningMode = !learningMode;
    suspended = serialLearning = false;
    output.release();
    if (!learningMode) output.enable();
    feedback.setLearning(learningMode, millis());
    accepted = true;
  } else if (learningMode) {
    uint8_t ignored;
    if (!output.learning() && output.codeFor(event.action, ignored)) {
      output.enable();
      accepted = output.learn(event.action, millis());
      if (accepted) feedback.learningCommand(millis());
    }
  } else accepted = output.submit(event, millis());
  uint8_t requestedCode = 0;
  const int code = output.codeFor(event.action, requestedCode) ? requestedCode : -1;
  char fields[170];
  snprintf(fields, sizeof(fields),
      ",\"action\":\"%s\",\"steps\":%lu,\"simulated\":false,\"accepted\":%s,\"pot_code\":%d",
      actionName(event.action), static_cast<unsigned long>(event.steps), accepted ? "true" : "false", code);
  emitEvent("gesture", fields);
}
knob::Gestures gestures(gestureEvent);

void status(const char *message) {
  char fields[192];
  // Messages are fixed firmware literals, not user input.
  snprintf(fields, sizeof(fields), ",\"message\":\"%s\"", message);
  emitEvent("status", fields);
}

void heartbeat() {
  char fields[195];
  snprintf(fields, sizeof(fields),
      ",\"connected\":%s,\"camera_preview\":%s,\"simulated\":false,\"outputs_enabled\":%s,\"relay_requested\":%s,\"learning_mode\":%s",
      connected ? "true" : "false", gestures.cameraRequested() ? "true" : "false",
      output.enabled() ? "true" : "false", output.relayOn() ? "true" : "false", learningMode ? "true" : "false");
  emitEvent("heartbeat", fields);
}

void help() {
  status("Gesture controller v3.6; USB maintenance: Picotool + boot command; Sony/camera hardware backend");
  status("Turn=volume; click=play/pause; held turn=track; hold/release >=0.7s=mute; double click=Camera 1");
  status("Debounce=30ms; double click=300ms from first release to second press; long hold=700ms");
  status("Sony codes: volume=255/0; play/pause=238; mute=230; next=248; previous=215");
  status("LED: full blue idle; purple held; white volume pulse; green click; amber camera-gesture pulse only");
  status("Hold still for 10s: toggle learning. Entry=3 amber flashes; exit=3 blue; assignment=2 green flashes");
  status("Calibration: codes; set ACTION 0..255; probe 0..255; learn ACTION; RAM-only settings; holds 6s");
  status("Volume runs retain detents; timing PRESS GAP adjusts normal pulses in RAM; defaults 55 55 ms");
  reportTiming();
  heartbeat();
}

bool readSample(Sample &sample) {
  Wire.beginTransmission(QWIIC_TWIST_ADDR);
  Wire.write(TWIST_ID);
  sampleDetail = Wire.endTransmission();
  if (sampleDetail != 0) { sampleError = "register-pointer write"; return false; }
  sampleDetail = Wire.requestFrom(QWIIC_TWIST_ADDR, static_cast<uint8_t>(7));
  if (sampleDetail != 7) {
    sampleError = "snapshot byte count";
    while (Wire.available()) Wire.read();
    return false;
  }
  uint8_t bytes[7];
  for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = Wire.read();
  if (bytes[0] != 0x5C) { sampleError = "device ID"; sampleDetail = bytes[0]; return false; }
  sampleError = "none";
  sample.count = static_cast<uint16_t>(bytes[5]) | (static_cast<uint16_t>(bytes[6]) << 8);
  sample.down = bytes[1] & (1 << statusButtonPressedBit);
  sample.major = bytes[2]; sample.minor = bytes[3];
  return true;
}

void inputEvent(const Sample &sample) {
  char fields[70];
  const int32_t count = sample.count <= 32767 ? static_cast<int32_t>(sample.count)
                                             : static_cast<int32_t>(sample.count) - 65536;
  snprintf(fields, sizeof(fields), ",\"count\":%ld,\"pressed\":%s",
      static_cast<long>(count), sample.down ? "true" : "false");
  emitEvent("input", fields);
}

void loseTwist(const char *message, bool tryOff = true) {
  output.release();
  learningMode = serialLearning = false;
  connected = false;
  feedback.clear();
  ledKnown = false;
  if (tryOff) (void)twist.setColor(0, 0, 0);  // Best effort; a failed bus cannot guarantee LED-off.
  gestures.cancel();
  lastAttempt = millis();
  status(message);
  heartbeat();
}

void applyFeedback(uint32_t now) {
  if (!connected) return;
  const knob::Color color = feedback.colorAt(now, gestures.buttonHeld(), true);
  if (ledKnown && color == lastLed) return;  // Write only on actual color transitions.
  if (!twist.setColor(color.red, color.green, color.blue)) {
    loseTwist("Twist LED write failed; pending gestures cancelled; retrying in 2s", false);
    return;
  }
  lastLed = color;
  ledKnown = true;
}

void attachTwist() {
  lastAttempt = millis();
  const bool detected = twist.begin(Wire);
  Wire.setClock(100000);
  if (!detected) {
    Wire.beginTransmission(QWIIC_TWIST_ADDR);
    const uint8_t error = Wire.endTransmission();
    char message[170];
    snprintf(message, sizeof(message),
        "Twist begin failed at 0x3F; re-probe I2C error=%u, SDA=%d SCL=%d timeout=%d; retrying in 2s",
        error, digitalRead(6), digitalRead(7), Wire.getTimeoutFlag());
    status(message); return;
  }
  if (!readSample(lastSample)) {
    char message[170];
    snprintf(message, sizeof(message), "Twist ACKs but %s failed (detail=%u); retrying in 2s", sampleError, sampleDetail);
    status(message); return;
  }
  if (!twist.connectColor(0, 0, 0)) {
    status("Twist snapshot OK; connectColor write failed; retrying in 2s"); return;
  }
  if (!twist.setLimit(0)) {
    status("Twist snapshot/color setup OK; setLimit write failed; retrying in 2s"); return;
  }
  gestures.begin(millis(), lastSample.count, lastSample.down);
  feedback.clear();
  ledKnown = false;
  connected = true;
  if (!suspended) output.enable();
  applyFeedback(millis());
  if (!connected) return;
  status("Twist connected; gesture baseline established; full-brightness LED feedback active");
  inputEvent(lastSample);
  heartbeat();
}

void maintenanceStatus() {
  // Preserve the uploader's known identification and explicit boot path.
  if (!Serial) return;
  char record[128];
  const int length = snprintf(record, sizeof(record),
      "ready=%d armed=%d pot_requested=%u relay_requested=%d camera_requested=%d\n",
      interfaceReady, output.enabled(), output.code(), output.relayOn(), output.cameraOn());
  if (length > 0 && static_cast<size_t>(length) < sizeof(record) && Serial.availableForWrite() >= length)
    Serial.write(reinterpret_cast<const uint8_t *>(record), length);
}

void reportCodes() {
  char fields[190];
  uint8_t values[knob::kCommandCount] = {};
  for (uint8_t i = 0; i < knob::kCommandCount; ++i)
    output.codeFor(knob::kDefaultCommands[i].action, values[i]);
  snprintf(fields, sizeof(fields),
      ",\"codes\":{\"volume_up\":%u,\"volume_down\":%u,\"play_pause\":%u,\"mute_att\":%u,\"next_track\":%u,\"previous_track\":%u},\"persistent\":false",
      values[0], values[1], values[2], values[3], values[4], values[5]);
  emitEvent("calibration", fields);
}

void reportTiming() {
  char fields[100];
  snprintf(fields, sizeof(fields), ",\"press_ms\":%lu,\"gap_ms\":%lu,\"persistent\":false",
      static_cast<unsigned long>(output.pressMs()), static_cast<unsigned long>(output.gapMs()));
  emitEvent("timing", fields);
}

void serialCommand(const char *text) {
  if (!strcmp(text, "boot")) {
    suspended = true;
    learningMode = serialLearning = false;
    output.release(); watchdog_disable();
    bootRequest.request(millis());
    status("BOOTSEL requested; waiting for serial close, then disconnecting USB");
  } else if (!strcmp(text, "help") || !strcmp(text, "h") || !strcmp(text, "?")) help();
  else if (!strcmp(text, "status") || !strcmp(text, "s")) { maintenanceStatus(); heartbeat(); }
  else if (!strcmp(text, "codes")) reportCodes();
  else if (!strcmp(text, "timing")) reportTiming();
  else if (!strncmp(text, "timing ", 7)) {
    uint8_t press, gap;
    if (knob::parsePulseTiming(text + 7, press, gap)) {
      output.setTiming(press, gap); serialLearning = false; reportTiming();
    } else {
      suspended = true; learningMode = serialLearning = false;
      feedback.clear(); output.release(); gestures.cancel();
      status("Invalid timing; use timing PRESS GAP, each 5..250 ms. Outputs suspended; enable resumes");
    }
  }
  else if (!strncmp(text, "set ", 4)) {
    knob::Action action; uint8_t value;
    if (knob::parseCodeSetting(text + 4, action, value)) {
      output.setCode(action, value);
      serialLearning = false;
      reportCodes();
    } else {
      suspended = true; learningMode = serialLearning = false;
      feedback.clear(); output.release(); gestures.cancel();
      status("Invalid setting; use set ACTION 0..255. Outputs suspended; enable resumes");
    }
  }
  else if (!strncmp(text, "probe ", 6)) {
    suspended = true; learningMode = false; serialLearning = true;
    output.release(); gestures.cancel(); feedback.setLearning(false, millis());
    uint8_t code;
    if (interfaceReady && connected && knob::parsePotCode(text + 6, code)) {
      output.enable(); output.learnCode(code, millis());
      status("Probe code held for 6s, then released. Knob suspended; enable resumes");
    } else status("Invalid probe or Twist unavailable; outputs released");
  }
  else if (!strncmp(text, "learn ", 6)) {
    suspended = true; learningMode = false; serialLearning = true;
    feedback.setLearning(false, millis()); output.release(); gestures.cancel();
    for (const auto &entry : knob::kDefaultCommands) {
      const knob::Action action = entry.action;
      if (!strcmp(text + 6, entry.name) && interfaceReady && connected) {
        output.enable();
        output.learn(action, millis());
        status("Learning resistance held for 6s, then released. Knob suspended; enable resumes");
        return;
      }
    }
    status("Invalid learning action or Twist unavailable; outputs released");
  }
  else if (!strcmp(text, "release")) {
    suspended = true; learningMode = serialLearning = false;
    feedback.clear(); output.release(); gestures.cancel();
    status("Outputs released; enter enable to resume");
  } else if (!strcmp(text, "enable")) {
    suspended = learningMode = serialLearning = false;
    feedback.clear();
    if (connected && interfaceReady) {
      gestures.begin(millis(), lastSample.count, lastSample.down);
      output.enable();
    }
    heartbeat();
  } else {
    suspended = true; learningMode = serialLearning = false;
    feedback.clear(); output.release(); gestures.cancel();
    status("Unknown command; outputs released. Enter enable to resume");
  }
}

void serviceSerial() {
  if (!Serial) {
    // A learning hold requires the terminal; normal operation does not.
    if (serialLearning && output.learning()) output.release();
    serialLength = 0; serialOverflow = false; hadSerial = false; return;
  }
  if (!hadSerial) { hadSerial = true; status("Controller ready. Enter help or status."); }
  for (unsigned n = 0; n < 48 && Serial.available(); ++n) {
    const char ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (!serialOverflow && serialLength) {
        serialLine[serialLength] = '\0'; serialCommand(serialLine);
        if (bootRequest.pending()) { serialLength = 0; return; }
      }
      serialLength = 0; serialOverflow = false;
    } else if (!serialOverflow) {
      if (ch >= 32 && ch <= 126 && serialLength + 1 < sizeof(serialLine))
        serialLine[serialLength++] = ch;
      else {
        suspended = true; learningMode = serialLearning = false;
        feedback.clear(); output.release(); gestures.cancel();
        serialLength = 0; serialOverflow = true;
        status("Invalid/oversized input; outputs released");
      }
    }
  }
}

void setup() {
  digitalWrite(kRelay, LOW); pinMode(kRelay, OUTPUT);
  digitalWrite(kCamera, LOW); pinMode(kCamera, OUTPUT);
  digitalWrite(kCs, HIGH); pinMode(kCs, OUTPUT);
  gpio_set_drive_strength(kRelay, GPIO_DRIVE_STRENGTH_4MA);
  Serial.begin(115200);
  startupAt = millis();
  // Service maintenance before touching SPI/I2C, as in the cold-start fix.
}

void loop() {
  if (interfaceReady) watchdog_update();
  if (!bootRequest.pending()) serviceSerial();
  if (bootRequest.pending()) {
    if (bootRequest.ready(millis(), static_cast<bool>(Serial))) {
      // Core API signals USB detach and allows host detection before ROM reset.
      // Run in loop context, with outputs released and the watchdog disabled.
      USB.disconnect();
      reset_usb_boot(0, 0);
    }
    delay(1);
    return;
  }
  uint32_t now = millis();
  if (!interfaceReady) {
    if (now - startupAt < 2000) { delay(1); return; }
    watchdog_enable(5000, true);
    SPI.setSCK(2); SPI.setTX(3); SPI.setRX(4); SPI.begin();
    writePot(128);
    Wire.setSDA(6); Wire.setSCL(7);
    Wire.begin(); Wire.setClock(100000); Wire.setTimeout(25, true);
    interfaceReady = true;
    help(); attachTwist();
    now = millis();
  }
  // Release expired pulses before input or USB reporting work.
  output.tick(millis());
  if (!connected && now - lastAttempt >= 2000) attachTwist();
  if (connected && now - lastPoll >= 10) {
    lastPoll = now;
    Sample sample;
    if (!readSample(sample)) {
      loseTwist("Twist read failed; pending gestures cancelled and camera preview released");
    } else {
      gestures.update(now, sample.count, sample.down);
      if (sample.count != lastSample.count || sample.down != lastSample.down) inputEvent(sample);
      lastSample = sample;
    }
  }
  output.tick(millis());
  applyFeedback(millis());
  serviceOutputTrace();  // Report captured edges after servicing time-critical outputs.
  if (now - lastHeartbeat >= 2000) { lastHeartbeat = now; heartbeat(); }
  delay(1);
}
