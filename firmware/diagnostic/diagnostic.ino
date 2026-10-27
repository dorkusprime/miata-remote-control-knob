#include <Wire.h>
#include <SparkFun_Qwiic_Twist_Arduino_Library.h>

// Serial-only diagnostic. No Sony/camera pins are configured or driven.
constexpr uint8_t kSdaGpio = 6;  // XIAO D4
constexpr uint8_t kSclGpio = 7;  // XIAO D5
constexpr uint8_t kBrightness = 8;  // 8/255 for each test channel
TWIST twist;
bool connected = false;
bool pressed = false;
uint32_t lastPoll = 0;
uint32_t lastAttempt = 0;
uint32_t lastHeartbeat = 0;

void help() {
  Serial.println("Miata knob diagnostic; Sony/camera outputs DISABLED");
  Serial.println("I2C: D4/GPIO6 SDA, D5/GPIO7 SCL, 100 kHz; Twist default 0x3F");
  Serial.println("Commands: h=help s=scan r/g/b/w=dim LED o=LED off");
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
  twist.getDiff();  // Discard movements made before attachment.
  pressed = twist.isPressed();
  connected = true;
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
    if (!twist.isConnected()) {
      connected = false;
      lastAttempt = now;
      Serial.println("Twist disconnected; events suspended. Retrying in 5s.");
    } else {
      const int16_t delta = twist.getDiff();
      const bool down = twist.isPressed();
      // Guard against disappearance during the library's individual reads.
      if (!twist.isConnected()) {
        connected = false;
        lastAttempt = now;
        Serial.println("Twist communication lost; sample discarded.");
      } else {
        if (delta) {
          Serial.print("TURN delta=");
          Serial.print(delta);
          Serial.println(down ? " button=DOWN" : " button=UP");
        }
        if (down != pressed) {
          pressed = down;
          Serial.println(down ? "BUTTON DOWN" : "BUTTON UP");
        }
      }
    }
  }
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    Serial.print("ALIVE; Twist=");
    Serial.println(connected ? "connected" : "missing");
  }
  delay(1);
}
