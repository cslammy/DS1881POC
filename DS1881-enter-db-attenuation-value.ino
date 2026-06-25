#include <Wire.h>

#define DS1881_ADDR 0x28

/*DS1881 test
Put an audio signal (4v pp 2vdc offset) into channel 0
Into arduino serial monitor "message", enter the value to attenuate input (0 to 64)
*/

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
    Serial.print("  Pot0:   pos="); Serial.print(pot0 & 0x3F);
    Serial.print("  (-"); Serial.print(pot0 & 0x3F); Serial.println("dB)");
    Serial.print("  Pot1:   pos="); Serial.print(pot1 & 0x3F);
    Serial.print("  (-"); Serial.print(pot1 & 0x3F); Serial.println("dB)");
    Serial.print("  Config: 0x"); Serial.println(cfg, HEX);
    Serial.print("  ZCD bit:     "); Serial.println((cfg >> 1) & 0x01 ? "ENABLED" : "DISABLED");
    Serial.print("  Pot config:  Option "); Serial.println((cfg & 0x01) ? "2" : "1");
    Serial.print("  V/NV bit:    "); Serial.println((cfg >> 2) & 0x01 ? "VOLATILE" : "NONVOLATILE");
  }
}

void printHelp() {
  Serial.println("\n=== DS1881 Attenuator ===");
  Serial.println("  Enter 0-62  : set attenuation in dB (e.g. '30' = -30dB)");
  Serial.println("  'm'         : mute both channels");
  Serial.println("  'r'         : read current register values");
  Serial.println("  '?'         : print this help");
  Serial.println("========================\n");
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  static bool initialized = false;
  static String inputBuffer = "";

  if (!Serial) {
    delay(100);
    return;
  }

  if (!initialized) {
    initialized = true;

    // Config 0x86: NV, ZCD enabled, Option 1 (1dB steps, 63 positions)
    Wire.beginTransmission(DS1881_ADDR);
    Wire.write(0x86);
    Wire.endTransmission();
    delay(15);

    // Read back immediately to verify
    Serial.println("\n=== Config Register Verification ===");
    Wire.requestFrom(DS1881_ADDR, 3);
    if (Wire.available() == 3) {
      byte pot0 = Wire.read();
      byte pot1 = Wire.read();
      byte cfg  = Wire.read();
      Serial.print("  Raw config byte: 0x"); Serial.println(cfg, HEX);
      Serial.print("  ZCD bit:         "); Serial.println((cfg >> 1) & 0x01 ? "ENABLED" : "DISABLED");
      Serial.print("  Pot config:      Option "); Serial.println((cfg & 0x01) ? "2" : "1");
      Serial.print("  V/NV bit:        "); Serial.println((cfg >> 2) & 0x01 ? "VOLATILE" : "NONVOLATILE");
      
      if (cfg == 0x86) {
        Serial.println("  Config write VERIFIED OK");
      } else {
        Serial.print("  WARNING: Expected 0x86 but got 0x");
        Serial.println(cfg, HEX);
      }
    } else {
      Serial.println("  ERROR: No response from DS1881");
    }
    Serial.println("====================================\n");

    // Start at 0dB
    writeP0(0);
    writeP1(0);

    printHelp();
    Serial.println("Ready. Currently set to 0dB.");
    Serial.print("Enter attenuation (0-62): ");
  }

  // Collect serial input until newline
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() == 0) continue;

      String input = inputBuffer;
      inputBuffer = "";
      input.trim();

      // Handle single character commands
      if (input.length() == 1) {
        char cmd = input.charAt(0);

        if (cmd == 'm' || cmd == 'M') {
          writeP0(63);
          writeP1(63);
          Serial.println("\nBoth channels muted.");
          Serial.print("Enter attenuation (0-62): ");
          continue;
        }
        if (cmd == 'r' || cmd == 'R') {
          Serial.println();
          readAndPrintRegisters();
          Serial.print("Enter attenuation (0-62): ");
          continue;
        }
        if (cmd == '?') {
          printHelp();
          Serial.print("Enter attenuation (0-62): ");
          continue;
        }
      }

      // Handle numeric input
      bool isNumeric = true;
      for (int i = 0; i < input.length(); i++) {
        if (!isDigit(input.charAt(i))) {
          isNumeric = false;
          break;

        }
      }

      if (isNumeric) {
        int val = input.toInt();
        if (val >= 0 && val <= 62) {
          writeP0((byte)val);
          writeP1((byte)val);
          Serial.print("\nSet to -");
          Serial.print(val);
          Serial.println("dB on both channels.");
          Serial.print("Enter attenuation (0-62): ");
        } else {
          Serial.println("\nERROR: Value out of range. Enter 0-62.");
          Serial.print("Enter attenuation (0-62): ");
        }
      } else {
        Serial.println("\nERROR: Unrecognized input. Enter 0-62, 'm', 'r', or '?'");
        Serial.print("Enter attenuation (0-62): ");
      }

    } else {
      inputBuffer += c;
    }
  }
}