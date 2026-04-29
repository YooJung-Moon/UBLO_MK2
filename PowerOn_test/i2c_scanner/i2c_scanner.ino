#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  Wire.begin();

  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int count = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found");
  }

  Serial.println("----------------");
  delay(3000);
}