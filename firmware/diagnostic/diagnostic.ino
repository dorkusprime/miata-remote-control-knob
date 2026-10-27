#include <Wire.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

// Serial-only diagnostic. No Sony/camera pins are configured or driven.
constexpr uint8_t kSdaGpio = 6;  // XIAO D4
constexpr uint8_t kSclGpio = 7;  // XIAO D5
constexpr uint8_t kBrightness = 8;  // 8/255 for each test channel
TWIST twist;
bool connected = false;
bool pressed = false;
uint16_t lastCount = 0;
struct Snapshot {
  uint8_t status;
  uint8_t major;
  uint8_t minor;
  uint16_t count;
  int16_t difference;
};
Snapshot latest = {};

// A checked read of ID/status/version/count/difference. Do not clear registers.
bool readSnapshot(Snapshot &sample) {
  Wire.beginTransmission(QWIIC_TWIST_ADDR);
  Wire.write(TWIST_ID);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(QWIIC_TWIST_ADDR, static_cast<uint8_t>(9)) != 9) {
    while (Wire.available()) Wire.read();
    return false;
  }
  uint8_t bytes[9];
  for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = Wire.read();
  if (bytes[0] != 0x5C) return false;
  sample.status = bytes[1];
  sample.major = bytes[2];
  sample.minor = bytes[3];
  sample.count = static_cast<uint16_t>(bytes[5]) | (static_cast<uint16_t>(bytes[6]) << 8);
  const uint16_t rawDiff = static_cast<uint16_t>(bytes[7]) | (static_cast<uint16_t>(bytes[8]) << 8);
  sample.difference = rawDiff <= 32767 ? rawDiff : static_cast<int32_t>(rawDiff) - 65536;
  return true;
}

constexpr int32_t countDelta(uint16_t current, uint16_t previous) {
  const uint16_t wrapped = static_cast<uint16_t>(current - previous);
  return wrapped <= 32767 ? wrapped : static_cast<int32_t>(wrapped) - 65536;
}
static_assert(countDelta(0, 65535) == 1, "Forward counter wrap");
static_assert(countDelta(65535, 0) == -1, "Reverse counter wrap");
static_assert(countDelta(32768, 32767) == 1, "Signed boundary");
static_assert(countDelta(100, 100) == 0, "Stationary counter");

void printSnapshot() {
  Serial.print("RAW count=");
  const int32_t signedCount = latest.count <= 32767 ? latest.count : static_cast<int32_t>(latest.count) - 65536;
  Serial.print(signedCount);
  Serial.print(" diff=");
  Serial.print(latest.difference);
  Serial.print(" status=0x");
  Serial.print(latest.status, HEX);
  Serial.print(" firmware=");
  Serial.print(latest.major);
  Serial.print('.');
  Serial.println(latest.minor);
}
uint32_t lastPoll = 0;
uint32_t lastAttempt = 0;
uint32_t lastHeartbeat = 0;

void help() {
  Serial.println("Miata knob diagnostic v2 (absolute counter); Sony/camera outputs DISABLED");
  Serial.println("I2C: D4/GPIO6 SDA, D5/GPIO7 SCL, 100 kHz; Twist default 0x3F");
  Serial.println("Commands: h=help s=scan c=raw counters r/g/b/w=dim LED o=LED off");
}

void scan() {
  Serial.println("Scanning I2C...");
  unsigned found = 0;
  for (uint8_t address = 8; address < 120; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device: 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      ++found;
    }
  }
  Serial.print("Devices found: ");
  Serial.println(found);
}

void attachTwist() {
  lastAttempt = millis();
  const bool detected = twist.begin(Wire);
  Wire.setClock(100000);  // Library begin() also calls Wire.begin().
  if (!detected) {
    Serial.println("Twist unavailable at 0x3F; check wiring while USB unplugged. Retrying in 5s.");
    return;
  }
  // Undo any previous automatic LED/count settings, without changing I2C address.
  bool configured = twist.connectColor(0, 0, 0);
  configured = twist.setColor(0, 0, 0) && configured;
  configured = twist.setLimit(0) && configured;
  if (!configured) {
    Serial.println("Twist configuration failed; retrying in 5s.");
    return;
  }
  if (!readSnapshot(latest)) {
    Serial.println("Twist register read failed or unexpected device ID; retrying in 5s.");
    return;
  }
  lastCount = latest.count;
  pressed = latest.status & (1 << statusButtonPressedBit);
  connected = true;
  printSnapshot();
  Serial.println("Twist connected; LED off. Turn or press knob; h for commands.");
  Serial.print("Initial button: ");
  Serial.println(pressed ? "DOWN" : "UP");
}

void setup() {
  Serial.begin(115200);
  const uint32_t started = millis();
  while (!Serial && millis() - started < 2000) delay(10);
  Wire.setSDA(kSdaGpio);
  Wire.setSCL(kSclGpio);
  Wire.begin();
  Wire.setClock(100000);
  help();
  scan();
  attachTwist();
}

void loop() {
  const uint32_t now = millis();
  if (Serial.available()) {
    const char command = Serial.read();
    if (command == 'h' || command == '?') help();
    else if (command == 's') scan();
    else if (command == 'c') {
      if (connected && readSnapshot(latest)) printSnapshot();
      else Serial.println("Raw counter read unavailable.");
    }
    else if (command == 'r' || command == 'g' || command == 'b' ||
             command == 'w' || command == 'o') {
      if (!connected) Serial.println("LED unavailable: Twist not connected.");
      else {
        const uint8_t r = (command == 'r' || command == 'w') ? kBrightness : 0;
        const uint8_t g = (command == 'g' || command == 'w') ? kBrightness : 0;
        const uint8_t b = (command == 'b' || command == 'w') ? kBrightness : 0;
        Serial.println(twist.setColor(r, g, b) ? "LED updated." : "LED write failed.");
      }
    }
  }
  if (!connected && now - lastAttempt >= 5000) attachTwist();
  if (connected && now - lastPoll >= 20) {
    lastPoll = now;
    Snapshot sample;
    if (!readSnapshot(sample)) {
      connected = false;
      lastAttempt = now;
      Serial.println("Twist register read failed; events suspended. Retrying in 5s.");
    } else {
      latest = sample;
      const int32_t delta = countDelta(sample.count, lastCount);
      lastCount = sample.count;
      const bool down = sample.status & (1 << statusButtonPressedBit);
      if (delta) {
        Serial.print("TURN delta=");
        Serial.print(delta);
        Serial.println(down ? " button=DOWN" : " button=UP");
        printSnapshot();
      }
      if (down != pressed) {
        pressed = down;
        Serial.println(down ? "BUTTON DOWN" : "BUTTON UP");
      }
    }
  }
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    Serial.print("ALIVE; Twist=");
    Serial.println(connected ? "connected" : "missing");
    if (connected) printSnapshot();
  }
  delay(1);
}
