#include <SPI.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <hardware/gpio.h>
#include <stdlib.h>
#include <string.h>

// GPIO numbers, not XIAO silk-screen D numbers.
constexpr uint8_t kCs = 26;      // D0
constexpr uint8_t kRelay = 27;   // D1
constexpr uint8_t kCamera = 28;  // D2
constexpr uint32_t kTestMs = 10000;
constexpr uint32_t kArmMs = 60000;
bool armed = false;
bool relayOn = false;
bool cameraOn = false;
uint8_t potCode = 128;
uint32_t armedAt = 0;
uint32_t testAt = 0;
char line[48];
size_t length = 0;
bool overflow = false;
bool hadSerial = false;

void releaseOutputs() {
  digitalWrite(kRelay, LOW);
  digitalWrite(kCamera, LOW);
  relayOn = cameraOn = false;
}

void disarm() {
  releaseOutputs();
  digitalWrite(kCs, HIGH);
  armed = false;
}

void writePot(uint8_t code) {
  // Always disconnect the receiver path before changing resistance.
  releaseOutputs();
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(kCs, LOW);
  SPI.transfer(0x11);  // Write data, potentiometer 0 (Microchip DS11195C).
  SPI.transfer(code);
  digitalWrite(kCs, HIGH);
  SPI.endTransaction();
  potCode = code;
}

void status() {
  Serial.print("armed="); Serial.print(armed);
  Serial.print(" pot_requested="); Serial.print(potCode);
  Serial.print(" relay_requested="); Serial.print(relayOn);
  Serial.print(" camera_requested="); Serial.println(cameraOn);
}

void help() {
  Serial.println("Interface BENCH diagnostic. Disconnect Sony and all external signal sources.");
  Serial.println("Commands (Enter after each): help, status, arm bench, pot 0..255, relay, camera, release, boot");
  Serial.println("relay/camera assert for 10 seconds. Arm expires after 60 seconds.");
  Serial.println("release, invalid input, serial loss and reset release outputs and disarm.");
  Serial.println("Status reports requested settings; MCP41050 has no SPI readback.");
}

void command(const char *text) {
  if (!strcmp(text, "boot")) {
    disarm();
    watchdog_disable();
    reset_usb_boot(0, 0);
  } else if (!strcmp(text, "release")) {
    disarm();
  } else if (!strcmp(text, "help")) {
    help();
  } else if (!strcmp(text, "status")) {
    status();
    return;
  } else if (!strcmp(text, "arm bench")) {
    disarm();
    armed = true;
    armedAt = millis();
  } else if (armed && !strncmp(text, "pot ", 4)) {
    const char *digits = text + 4;
    bool valid = *digits != '\0';
    for (const char *p = digits; *p; ++p) valid &= *p >= '0' && *p <= '9';
    const unsigned long value = strtoul(digits, nullptr, 10);
    if (valid && value <= 255) writePot(static_cast<uint8_t>(value));
    else { disarm(); Serial.println("Invalid pot code; released."); }
  } else if (armed && (!strcmp(text, "relay") || !strcmp(text, "camera"))) {
    releaseOutputs();
    testAt = millis();
    relayOn = !strcmp(text, "relay");
    cameraOn = !strcmp(text, "camera");
    digitalWrite(kRelay, relayOn ? HIGH : LOW);
    digitalWrite(kCamera, cameraOn ? HIGH : LOW);
  } else {
    disarm();
    Serial.println("Unknown command or not armed; released.");
  }
  status();
}

void setup() {
  digitalWrite(kRelay, LOW); pinMode(kRelay, OUTPUT);
  digitalWrite(kCamera, LOW); pinMode(kCamera, OUTPUT);
  digitalWrite(kCs, HIGH); pinMode(kCs, OUTPUT);
  gpio_set_drive_strength(kRelay, GPIO_DRIVE_STRENGTH_4MA);
  SPI.setSCK(2);   // D8
  SPI.setTX(3);    // D10
  SPI.setRX(4);    // D9, unused MISO input
  SPI.begin();
  writePot(128);
  Serial.begin(115200);
  watchdog_enable(1000, true);
}

void loop() {
  watchdog_update();
  if (!Serial) {
    disarm();
    length = 0;
    overflow = false;
    hadSerial = false;
    return;
  }
  if (!hadSerial) { help(); hadSerial = true; }
  const uint32_t now = millis();
  if (armed && now - armedAt >= kArmMs) {
    disarm(); Serial.println("Arm expired; released.");
  }
  if ((relayOn || cameraOn) && now - testAt >= kTestMs) {
    releaseOutputs(); Serial.println("Test finished; outputs released.");
  }
  // Limit serial work so input floods cannot starve release timers.
  for (unsigned n = 0; n < 48 && Serial.available(); ++n) {
    const char ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (!overflow && length) { line[length] = '\0'; command(line); }
      length = 0;
      overflow = false;
    } else if (!overflow) {
      if (length + 1 < sizeof(line) && ch >= 32 && ch <= 126) line[length++] = ch;
      else {
        disarm(); length = 0; overflow = true;
        Serial.println("Invalid/oversized input; released.");
      }
    }
  }
}
