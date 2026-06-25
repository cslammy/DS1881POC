#include <Wire.h>

#define DS1881_ADDR 0x28
#define STEP_TIME_MS 100

bool zcdEnabled = true;

void writeP0(byte pos) {
  Wire.beginTransmission(DS1881_ADDR);
  Wire.write(pos & 0x3F);
  Wire.endTransmission();
}

void writeP1(byte pos) {
  Wire.beginTransmission(DS1881_ADDR);
  Wire.write(0x40 | (pos & 0x3F));
  Wire.endTransmission();
}

void readAndPrintRegisters() {
  Wire.requestFrom(DS1881_ADDR, 3);
  if (Wire.available() == 3) {
    byte pot0 = Wire.read();
    byte pot1 = Wire.read();
    byte cfg  = Wire.read();
    Serial.print("  Verified Pot0:   0x"); Serial.println(pot0, HEX);
    Serial.print("  Verified Pot1:   0x"); Serial.println(pot1, HEX);
    Serial.print("  Verified Config: 0x"); Serial.println(cfg, HEX);
    Serial.print("  ZCD bit (bit1):  ");
    Serial.println((cfg >> 1) & 0x01 ? "ENABLED" : "DISABLED");
    Serial.print("  Pot config (bit0): Option ");
    Serial.println((cfg & 0x01) ? "2" : "1");
  } else {
    Serial.println("  ERROR: No response from DS1881");
  }
}

void setConfig(bool zcd) {
  byte cfg = zcd ? 0x86 : 0x84;
  Serial.print("\n*** Setting ZCD ");
  Serial.print(zcd ? "ENABLED" : "DISABLED");
  Serial.print("  writing config byte: 0x");
  Serial.println(cfg, HEX);

  Wire.beginTransmission(DS1881_ADDR);
  Wire.write(cfg);
  Wire.endTransmission();
  delay(15);

  // Read back and verify it took
  Serial.println("*** Readback after config write:");
  readAndPrintRegisters();
  Serial.println(zcd ? "*** Send 'z' to disable ZCD" : "*** Send 'z' to enable ZCD");
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  if (!Serial) {
    delay(500);
    return;
  }

  // Check for toggle command
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'z' || c == 'Z') {
      zcdEnabled = !zcdEnabled;
      setConfig(zcdEnabled);
    }
  }

  // --- Fade out: 0dB to -62dB ---
  Serial.println("\nFading out: 0dB to -62dB...");
  for (byte pos = 0; pos <= 62; pos++) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'z' || c == 'Z') {
        zcdEnabled = !zcdEnabled;
        setConfig(zcdEnabled);
      }
    }
    writeP0(pos);
    writeP1(pos);
    Serial.print("  pos=");
    Serial.print(pos);
    Serial.print("  -");
    Serial.print(pos);
    Serial.println("dB");
    delay(STEP_TIME_MS);
  }

  // --- Mute ---
  Serial.println("Muting...");
  writeP0(63);
  writeP1(63);
  delay(1000);

  // --- Fade in: -62dB to 0dB ---
  Serial.println("Fading in: -62dB to 0dB...");
  for (byte pos = 62; pos != 255; pos--) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'z' || c == 'Z') {
        zcdEnabled = !zcdEnabled;
        setConfig(zcdEnabled);
      }
    }
    writeP0(pos);
    writeP1(pos);
    Serial.print("  pos=");
    Serial.print(pos);
    Serial.print("  -");
    Serial.print(pos);
    Serial.println("dB");
    delay(STEP_TIME_MS);
  }

  // --- Hold at 0dB ---
  Serial.println("Holding at 0dB...");
  delay(1000);
}