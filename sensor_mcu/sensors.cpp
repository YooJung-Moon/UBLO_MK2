#include "sensors.h"

#ifdef TEST_MODE
extern uint8_t testBatteryPct;
#endif

#ifdef TEST_MODE

// ── Scenario Data ─────────────────────────────
struct ScenarioStep {
  float co2_ppm;
  float temperature;
  float humidity;
  uint8_t battery_pct;
};

#if TEST_SCENARIO == 1
// COMFORT → WARNING → COMFORT
static ScenarioStep scenario[] = {
  {400.0f,  22.0f, 55.0f, 80},  // Step 0: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 1: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 2: COMFORT (3회 → COMFORT 확정)
  {1200.0f, 22.0f, 55.0f, 75},  // Step 3: WARNING
  {1200.0f, 22.0f, 55.0f, 75},  // Step 4: WARNING
  {1200.0f, 22.0f, 55.0f, 75},  // Step 5: WARNING (3회 → WARNING 확정)
  {900.0f,  22.0f, 55.0f, 70},  // Step 6: COMFORT (hysteresis)
  {900.0f,  22.0f, 55.0f, 70},  // Step 7: COMFORT
  {900.0f,  22.0f, 55.0f, 70},  // Step 8: COMFORT (3회 → COMFORT 확정)
};

#elif TEST_SCENARIO == 2
// COMFORT → ALERT → COMFORT
static ScenarioStep scenario[] = {
  {400.0f,  22.0f, 55.0f, 80},  // Step 0: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 1: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 2: COMFORT (3회 → COMFORT 확정)
  {1600.0f, 22.0f, 55.0f, 75},  // Step 3: ALERT
  {1600.0f, 22.0f, 55.0f, 75},  // Step 4: ALERT
  {1600.0f, 22.0f, 55.0f, 75},  // Step 5: ALERT (3회 → ALERT 확정)
  {900.0f,  22.0f, 55.0f, 70},  // Step 6: COMFORT (hysteresis)
  {900.0f,  22.0f, 55.0f, 70},  // Step 7: COMFORT
  {900.0f,  22.0f, 55.0f, 70},  // Step 8: COMFORT (3회 → COMFORT 확정)
};

#elif TEST_SCENARIO == 3
// Safe Mode 테스트 (정상 동작 후 패킷 중단)
static ScenarioStep scenario[] = {
  {400.0f,  22.0f, 55.0f, 80},  // Step 0: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 1: COMFORT
  {400.0f,  22.0f, 55.0f, 80},  // Step 2: COMFORT (3회 → COMFORT 확정)
  // Step 3 이후는 패킷 전송 중단 시뮬레이션
  // sensor_mcu.ino에서 STEP >= 3이면 espnowSend 건너뜀
};
#endif

static int scenarioStep = 0;
static int scenarioLen  = sizeof(scenario) / sizeof(scenario[0]);

SensorData testSensorData = {
  TEST_CO2_DEFAULT,
  TEST_TEMP_DEFAULT,
  TEST_HUMID_DEFAULT
};

bool sensorsInit() {
  Serial.printf("TEST MODE: scenario %d\n", TEST_SCENARIO);
  return true;
}

bool sensorsMeasure(SensorData &data) {
  if (scenarioStep < scenarioLen) {
    testSensorData.co2_ppm     = scenario[scenarioStep].co2_ppm;
    testSensorData.temperature = scenario[scenarioStep].temperature;
    testSensorData.humidity    = scenario[scenarioStep].humidity;
    testBatteryPct             = scenario[scenarioStep].battery_pct;

    Serial.printf("TEST: step=%d co2=%.1f temp=%.1f humid=%.1f bat=%d%%\n",
      scenarioStep,
      testSensorData.co2_ppm,
      testSensorData.temperature,
      testSensorData.humidity,
      testBatteryPct);

    scenarioStep++;
  } else {
    Serial.println("TEST: scenario complete");
  }

  data = testSensorData;
  return true;
}

// ── Scenario Step Getter (sensor_mcu.ino에서 사용) ──
int getScenarioStep() {
  return scenarioStep;
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