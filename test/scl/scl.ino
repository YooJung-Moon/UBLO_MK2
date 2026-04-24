#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(A4, A5);   // 👉 SDA=A4, SCL=A5

  Serial.println("I2C Scanner Start (A4/A5)");
}

void loop() {
  int count = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found");
  }

  Serial.println("----------------");
  delay(2000);
}