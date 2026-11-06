#include <Wire.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "gesture_engine.h"
#include "led_feedback.h"
#include "sony_output.h"
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
bool interfaceReady = false, suspended = false;
char serialLine[48];
size_t serialLength = 0;
bool serialOverflow = false, hadSerial = false;
uint32_t startupAt = 0;

void writePot(uint8_t code) {
  digitalWrite(kRelay, LOW);
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(kCs, LOW);
  SPI.transfer(0x11); SPI.transfer(code);
  digitalWrite(kCs, HIGH);
  SPI.endTransaction();
}
void setRelay(bool on) { digitalWrite(kRelay, on ? HIGH : LOW); }
void setCamera(bool on) { digitalWrite(kCamera, on ? HIGH : LOW); }
knob::SonyOutput output(writePot, setRelay, setCamera);
TWIST twist;
struct Sample { uint16_t count; bool down; uint8_t major, minor; };
// Explicit declarations prevent Arduino's generated prototypes preceding Sample.
bool readSample(Sample &sample);
void inputEvent(const Sample &sample);
Sample lastSample = {};
bool connected = false;
knob::LedFeedback feedback;
knob::Color lastLed = knob::kOff;
bool ledKnown = false;
uint32_t lastAttempt = 0, lastPoll = 0, lastHeartbeat = 0, sequence = 0;

void emitEvent(const char *type, const char *fields) {
  if (!Serial) return;
  char record[256];
  const int length = snprintf(record, sizeof(record),
      "{\"v\":1,\"type\":\"%s\",\"seq\":%lu,\"ms\":%lu%s}\n",
      type, static_cast<unsigned long>(sequence + 1),
      static_cast<unsigned long>(millis()), fields);
  if (length <= 0 || static_cast<size_t>(length) >= sizeof(record) ||
      Serial.availableForWrite() < length) return;
  ++sequence;
  Serial.write(reinterpret_cast<const uint8_t *>(record), length);
}

const char *actionName(knob::Action action) {
  switch (action) {
    case knob::Action::VolumeUp: return "volume_up";
    case knob::Action::VolumeDown: return "volume_down";
    case knob::Action::MuteAtt: return "mute_att";
    case knob::Action::NextTrack: return "next_track";
    case knob::Action::PreviousTrack: return "previous_track";
    case knob::Action::CameraOn: return "camera_on";
    case knob::Action::CameraOff: return "camera_off";
    case knob::Action::LongPressIgnored: return "long_press_ignored";
  }
  return "unknown";
}

void gestureEvent(const knob::Event &event, void *) {
  // Feedback must work with no serial monitor; fault-cancel events are not gestures.
  if (connected) feedback.onGesture(event, millis());
  const bool accepted = output.submit(event, millis());
  char fields[150];
  snprintf(fields, sizeof(fields),
      ",\"action\":\"%s\",\"steps\":%lu,\"simulated\":false,\"accepted\":%s",
      actionName(event.action), static_cast<unsigned long>(event.steps), accepted ? "true" : "false");
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
  char fields[180];
  snprintf(fields, sizeof(fields),
      ",\"connected\":%s,\"camera_preview\":%s,\"simulated\":false,\"outputs_enabled\":%s,\"relay_requested\":%s",
      connected ? "true" : "false", gestures.cameraRequested() ? "true" : "false",
      output.enabled() ? "true" : "false", output.relayOn() ? "true" : "false");
  emitEvent("heartbeat", fields);
}

void help() {
  status("Gesture controller v3; USB maintenance: Picotool + boot command; Sony/camera hardware backend");
  status("Turn=volume; click=mute/ATT; held turn=track; double click=camera preview; long hold=ignored");
  status("Debounce=30ms; double click=300ms from first release to second press; long hold=700ms");
  status("Serial (Enter): help/h, status/s, release, enable, boot; outputs are real requests, receiver acceptance unverified");
  status("LED: full blue idle; purple held; white volume pulse; green click; amber camera-gesture pulse only");
  heartbeat();
}

bool readSample(Sample &sample) {
  Wire.beginTransmission(QWIIC_TWIST_ADDR);
  Wire.write(TWIST_ID);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(QWIIC_TWIST_ADDR, static_cast<uint8_t>(7)) != 7) {
    while (Wire.available()) Wire.read();
    return false;
  }
  uint8_t bytes[7];
  for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = Wire.read();
  if (bytes[0] != 0x5C) return false;
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
  bool ok = twist.begin(Wire);
  Wire.setClock(100000);
  if (ok) ok = readSample(lastSample);
  if (ok) ok = twist.connectColor(0, 0, 0) && twist.setLimit(0);
  if (!ok) { status("Twist unavailable; retrying in 2s"); return; }
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

void serialCommand(const char *text) {
  if (!strcmp(text, "boot")) {
    output.release(); watchdog_disable(); reset_usb_boot(0, 0);
  } else if (!strcmp(text, "help") || !strcmp(text, "h") || !strcmp(text, "?")) help();
  else if (!strcmp(text, "status") || !strcmp(text, "s")) { maintenanceStatus(); heartbeat(); }
  else if (!strcmp(text, "release")) {
    suspended = true; output.release(); gestures.cancel();
    status("Outputs released; enter enable to resume");
  } else if (!strcmp(text, "enable")) {
    suspended = false;
    if (connected && interfaceReady) {
      gestures.begin(millis(), lastSample.count, lastSample.down);
      output.enable();
    }
    heartbeat();
  } else {
    suspended = true; output.release(); gestures.cancel();
    status("Unknown command; outputs released. Enter enable to resume");
  }
}

void serviceSerial() {
  if (!Serial) {
    // Normal standalone operation does not depend on a serial monitor.
    serialLength = 0; serialOverflow = false; hadSerial = false; return;
  }
  if (!hadSerial) { hadSerial = true; status("Controller ready. Enter help or status."); }
  for (unsigned n = 0; n < 48 && Serial.available(); ++n) {
    const char ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (!serialOverflow && serialLength) {
        serialLine[serialLength] = '\0'; serialCommand(serialLine);
      }
      serialLength = 0; serialOverflow = false;
    } else if (!serialOverflow) {
      if (ch >= 32 && ch <= 126 && serialLength + 1 < sizeof(serialLine))
        serialLine[serialLength++] = ch;
      else {
        suspended = true; output.release(); gestures.cancel();
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
  serviceSerial();
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
  if (now - lastHeartbeat >= 2000) { lastHeartbeat = now; heartbeat(); }
  delay(1);
}
