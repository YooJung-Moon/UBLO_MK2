#include <Wire.h>
#include <SensirionI2CScd4x.h>

SensirionI2CScd4x scd4x;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  Wire.begin();   // Nano ESP32 기본 I2C 핀 사용

  scd4x.begin(Wire);

  uint16_t error;
  char errorMessage[256];

  error = scd4x.stopPeriodicMeasurement();
  delay(500);

  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error starting measurement: ");
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

  if (!error && co2 != 0) {
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm, Temp: ");
    Serial.print(temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %RH");
  }

  delay(5000);
}