#include <Wire.h>

#define DS1881_ADDR 0x28

bool zcdEnabled = true;
byte currentPos = 0;

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
    Serial.print("  Pot0:   0x"); Serial.println(pot0, HEX);
    Serial.print("  Pot1:   0x"); Serial.println(pot1, HEX);
    Serial.print("  Config: 0x"); Serial.println(cfg, HEX);
    Serial.print("  ZCD: ");
    Serial.println((cfg >> 1) & 0x01 ? "ENABLED" : "DISABLED");
  }
}

void setConfig(bool zcd) {
  byte cfg = zcd ? 0x86 : 0x84;
  Wire.beginTransmission(DS1881_ADDR);
  Wire.write(cfg);
  Wire.endTransmission();
  delay(15);
}

void printHelp() {
  Serial.println("\n=== DS1881 Manual Step Tester ===");
  Serial.println("  '+' = step up 1dB (less attenuation)");
  Serial.println("  '-' = step down 1dB (more attenuation)");
  Serial.println("  'u' = jump to 0dB (position 0)");
  Serial.println("  'd' = jump to -62dB (position 62)");
  Serial.println("  'm' = mute (position 63)");
  Serial.println("  'z' = toggle ZCD on/off");
  Serial.println("  'r' = read registers");
  Serial.println("  '?' = print this help");
  Serial.println("================================");
}

void printStatus() {
  Serial.print("ZCD=");
  Serial.print(zcdEnabled ? "ON " : "OFF");
  Serial.print("  pos=");
  Serial.print(currentPos);
  if (currentPos == 63) {
    Serial.println("  [MUTE]");
  } else {
    Serial.print("  attenuation=-");
    Serial.print(currentPos);
    Serial.println("dB");
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  if (!Serial) {
    delay(100);
    return;
  }

  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    setConfig(zcdEnabled);
    writeP0(currentPos);
    writeP1(currentPos);
    printHelp();
    printStatus();
  }

  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case '+':
        if (currentPos > 0) currentPos--;
        writeP0(currentPos);
        writeP1(currentPos);
        printStatus();
        break;

      case '-':
        if (currentPos < 62) currentPos++;
        writeP0(currentPos);
        writeP1(currentPos);
        printStatus();
        break;

      case 'u':
      case 'U':
        currentPos = 0;
        writeP0(currentPos);
        writeP1(currentPos);
        printStatus();
        break;

      case 'd':
      case 'D':
        currentPos = 62;
        writeP0(currentPos);
        writeP1(currentPos);
        printStatus();
        break;

      case 'm':
      case 'M':
        currentPos = 63;
        writeP0(currentPos);
        writeP1(currentPos);
        printStatus();
        break;

      case 'z':
      case 'Z':
        zcdEnabled = !zcdEnabled;
        setConfig(zcdEnabled);
        delay(15);
        Serial.print("*** ZCD toggled: now ");
        Serial.println(zcdEnabled ? "ENABLED" : "DISABLED");
        readAndPrintRegisters();
        break;

      case 'r':
      case 'R':
        readAndPrintRegisters();
        break;

      case '?':
        printHelp();
        break;
    }
  }
}
