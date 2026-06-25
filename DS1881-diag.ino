#include <Wire.h>

#define DS1881_ADDR 0x28

bool ran = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  if (!Serial) {
    delay(100);
    return;  // wait here until Serial Monitor is open
  }

  if (!ran) {
    ran = true;

    Serial.println("=== DS1881 Debug ===");

    Serial.println("\n[TEST 1] Direct address test at 0x28...");
    Wire.beginTransmission(DS1881_ADDR);
    byte error = Wire.endTransmission();
    Serial.print("  endTransmission() returned: ");
    Serial.println(error);
    Serial.println("  (0=success, 2=NACK addr, 3=NACK data, 4=other error)");

    Serial.println("\n[TEST 2] Full I2C scan with error codes...");
    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      byte err = Wire.endTransmission();
      if (err == 0) {
        Serial.print("  FOUND device at 0x");
        Serial.println(addr, HEX);
      } else if (err == 4) {
        Serial.print("  Unknown error at 0x");
        Serial.println(addr, HEX);
      }
    }
    Serial.println("Scan complete.");

    Serial.println("\n[TEST 3] Attempt write to DS1881...");
    Wire.beginTransmission(DS1881_ADDR);
    Wire.write(0x87);
    error = Wire.endTransmission();
    Serial.print("  Write endTransmission() returned: ");
    Serial.println(error);

    Serial.println("\n[TEST 4] Attempt read from DS1881...");
    byte count = Wire.requestFrom(DS1881_ADDR, (byte)3);
    Serial.print("  Bytes returned: ");
    Serial.println(count);
    if (count > 0) {
      while (Wire.available()) {
        byte b = Wire.read();
        Serial.print("  Data: 0x");
        Serial.println(b, HEX);
      }
    }

    Serial.println("\n=== Done. Reprinting every 5s ===");
  }

  Serial.println("(still running)");
  delay(5000);
}