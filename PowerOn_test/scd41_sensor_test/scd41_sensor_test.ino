#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

SensirionI2cScd4x scd4x;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  uint16_t error;
  char errorMessage[256];

  error = scd4x.stopPeriodicMeasurement();
  delay(500);

  error = scd4x.startPeriodicMeasurement();

  if (error) {
    Serial.print("Start error: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.println("SCD41 measurement started");
  }
}

void loop() {
  uint16_t co2;
  float temperature;
  float humidity;

  uint16_t error = scd4x.readMeasurement(co2, temperature, humidity);

  if (error) {
    char errorMessage[256];
    Serial.print("Read error: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 != 0) {
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm | Temp: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidity: ");
    Serial.print(humidity);
    Serial.println(" %RH");
  } else {
    Serial.println("Waiting for data...");
  }

  delay(5000);
}