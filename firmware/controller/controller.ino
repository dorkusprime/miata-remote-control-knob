#include <Wire.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>
#include "gesture_engine.h"

#ifndef ENABLE_PICOTOOL_USB
#error "Build controller with uploadmethod=picotool to retain USB maintenance."
#endif

// Serial simulation only. No Sony or camera GPIO is configured or driven.
TWIST twist;
struct Sample { uint16_t count; bool down; uint8_t major, minor; };
Sample lastSample = {};
bool connected = false;
uint32_t lastAttempt = 0, lastPoll = 0, lastHeartbeat = 0, sequence = 0;

void startEvent(const char *type) {
  if (!Serial) return;
  Serial.print("{\"v\":1,\"type\":\""); Serial.print(type);
  Serial.print("\",\"seq\":"); Serial.print(++sequence);
  Serial.print(",\"ms\":"); Serial.print(millis());
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
  if (!Serial) return;
  startEvent("gesture");
  Serial.print(",\"action\":\""); Serial.print(actionName(event.action));
  Serial.print("\",\"steps\":"); Serial.print(event.steps);
  Serial.println(",\"simulated\":true}");
}
knob::Gestures gestures(gestureEvent);

void status(const char *message) {
  if (!Serial) return;
  startEvent("status");
  // Messages are fixed firmware literals, not user input.
  Serial.print(",\"message\":\""); Serial.print(message); Serial.println("\"}");
}

void heartbeat() {
  if (!Serial) return;
  startEvent("heartbeat");
  Serial.print(",\"connected\":"); Serial.print(connected ? "true" : "false");
  Serial.print(",\"camera_preview\":"); Serial.print(gestures.cameraRequested() ? "true" : "false");
  Serial.println(",\"simulated\":true}");
}

void help() {
  status("Gesture controller v1; USB maintenance: Picotool enabled; Sony/camera outputs DISABLED");
  status("Turn=volume; click=mute/ATT; held turn=track; double click=camera preview; long hold=ignored");
  status("Debounce=30ms; double click=300ms from first release to second press; long hold=700ms");
  status("Serial: h=help, s=status; all actions are simulations");
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
  if (!Serial) return;
  startEvent("input");
  Serial.print(",\"count\":");
  Serial.print(sample.count <= 32767 ? static_cast<int32_t>(sample.count)
                                    : static_cast<int32_t>(sample.count) - 65536);
  Serial.print(",\"pressed\":"); Serial.print(sample.down ? "true" : "false");
  Serial.println('}');
}

void attachTwist() {
  lastAttempt = millis();
  bool ok = twist.begin(Wire);
  Wire.setClock(100000);
  if (ok) ok = readSample(lastSample);
  if (ok) ok = twist.connectColor(0, 0, 0) && twist.setColor(0, 0, 0) && twist.setLimit(0);
  if (!ok) { status("Twist unavailable; retrying in 2s"); return; }
  gestures.begin(millis(), lastSample.count, lastSample.down);
  connected = true;
  status("Twist connected; gesture baseline established; LED off");
  inputEvent(lastSample);
  heartbeat();
}

void setup() {
  Serial.begin(115200);
  const uint32_t started = millis();
  while (!Serial && millis() - started < 2000) delay(10);
  Wire.setSDA(6);  // XIAO D4
  Wire.setSCL(7);  // XIAO D5
  Wire.begin(); Wire.setClock(100000);
  help(); attachTwist();
}

void loop() {
  const uint32_t now = millis();
  if (Serial.available()) {
    const char command = Serial.read();
    if (command == 'h' || command == '?') help();
    else if (command == 's') heartbeat();
  }
  if (!connected && now - lastAttempt >= 2000) attachTwist();
  if (connected && now - lastPoll >= 10) {
    lastPoll = now;
    Sample sample;
    if (!readSample(sample)) {
      connected = false;
      gestures.cancel();
      lastAttempt = now;
      status("Twist read failed; pending gestures cancelled and camera preview released");
      heartbeat();
    } else {
      if (sample.count != lastSample.count || sample.down != lastSample.down) inputEvent(sample);
      gestures.update(now, sample.count, sample.down);
      lastSample = sample;
    }
  }
  if (now - lastHeartbeat >= 2000) { lastHeartbeat = now; heartbeat(); }
  delay(1);
}
