#include "sensors.h"

#ifdef TEST_MODE
// ── Test Mode Variables ───────────────────────
SensorData testSensorData = {
  TEST_CO2_DEFAULT,
  TEST_TEMP_DEFAULT,
  TEST_HUMID_DEFAULT
};

bool sensorsInit() {
  Serial.println("TEST MODE: sensors init skipped");
  return true;
}

bool sensorsMeasure(SensorData &data) {
  data = testSensorData;
  Serial.printf("TEST MODE: co2=%.1f temp=%.1f humid=%.1f\n",
    data.co2_ppm, data.temperature, data.humidity);
  return true;
}

#else
// ── Real Mode ─────────────────────────────────
static SensirionI2cScd4x scd4x;
static SensirionI2cSht4x sht4x;

bool sensorsInit() {
  Wire.begin();

  sht4x.begin(Wire, SHT40_I2C_ADDR_44);
  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  uint16_t error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.println("SCD41 stop failed");
    return false;
  }

  float tempC, humRH;
  uint16_t shtError = sht4x.measureHighPrecision(tempC, humRH);
  if (shtError) {
    Serial.println("SHT4x init failed");
    return false;
  }

  error = scd4x.setTemperatureOffset(0.0f);
  if (error) {
    Serial.println("SCD41 temp offset failed");
    return false;
  }

  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.println("SCD41 start failed");
    return false;
  }

  Serial.println("Sensors initialized");
  return true;
}

bool sensorsMeasure(SensorData &data) {
  float tempC, humRH;
  uint16_t shtError = sht4x.measureHighPrecision(tempC, humRH);
  if (shtError) {
    Serial.println("SHT4x measure failed");
    return false;
  }
  data.temperature = tempC;
  data.humidity    = humRH;

  scd4x.setTemperatureOffset(tempC - 25.0f);

  bool isReady = false;
  uint16_t error = scd4x.getDataReadyStatus(isReady);
  if (error || !isReady) {
    Serial.println("SCD41 data not ready");
    return false;
  }

  uint16_t co2;
  error = scd4x.readMeasurement(co2, tempC, humRH);
  if (error) {
    Serial.println("SCD41 read failed");
    return false;
  }
  data.co2_ppm = (float)co2;

  return true;
}
#endif

AirQualityState evaluateAirQuality(const SensorData &data, AirQualityState currentState) {
  AirQualityState newState = COMFORT;

  if (data.co2_ppm > CO2_ALERT) {
    newState = ALERT;
  } else if (data.co2_ppm > CO2_WARNING) {
    newState = WARNING;
  } else {
    if (data.temperature < TEMP_LOW || data.temperature > TEMP_HIGH ||
        data.humidity    < HUMID_LOW || data.humidity    > HUMID_HIGH) {
      newState = WARNING;
    } else {
      newState = COMFORT;
    }
  }

  if (newState < currentState) {
    if (currentState == ALERT) {
      if (data.co2_ppm > CO2_ALERT_HYST) return currentState;
    } else if (currentState == WARNING) {
      if (data.co2_ppm > CO2_WARNING_HYST     ||
          data.temperature < TEMP_LOW_HYST    ||
          data.temperature > TEMP_HIGH_HYST   ||
          data.humidity    < HUMID_LOW_HYST   ||
          data.humidity    > HUMID_HIGH_HYST) {
        return currentState;
      }
    }
  }

  return newState;
}