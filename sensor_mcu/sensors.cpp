#include "sensors.h"

static SensirionI2cScd4x scd4x;
static SensirionI2cSht4x sht4x;

bool sensorsInit() {
  Wire.begin();

  // SHT4x 초기화
  sht4x.begin(Wire, SHT40_I2C_ADDR_44);

  // SCD41 초기화
  scd4x.begin(Wire, SCD41_I2C_ADDR_62);

  // 혹시 이전에 측정 중이었으면 정지
  uint16_t error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.println("SCD41 stop failed");
    return false;
  }

  // SHT4x 통신 확인
  float tempC, humRH;
  uint16_t shtError = sht4x.measureHighPrecision(tempC, humRH);
  if (shtError) {
    Serial.println("SHT4x init failed");
    return false;
  }

  // SCD41 온습도 보정 설정
  error = scd4x.setTemperatureOffset(0.0f);
  if (error) {
    Serial.println("SCD41 temp offset failed");
    return false;
  }

  // SCD41 주기적 측정 시작
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.println("SCD41 start failed");
    return false;
  }

  Serial.println("Sensors initialized");
  return true;
}

bool sensorsMeasure(SensorData &data) {
  // SHT4x 측정
  float tempC, humRH;
  uint16_t shtError = sht4x.measureHighPrecision(tempC, humRH);
  if (shtError) {
    Serial.println("SHT4x measure failed");
    return false;
  }
  data.temperature = tempC;
  data.humidity    = humRH;

  // SCD41 온습도 보정 업데이트
  scd4x.setTemperatureOffset(tempC - 25.0f);

  // SCD41 데이터 준비 확인
  bool isReady = false;
  uint16_t error = scd4x.getDataReadyStatus(isReady);
  if (error || !isReady) {
    Serial.println("SCD41 data not ready");
    return false;
  }

  // SCD41 측정값 읽기
  uint16_t co2;
  error = scd4x.readMeasurement(co2, tempC, humRH);
  if (error) {
    Serial.println("SCD41 read failed");
    return false;
  }
  data.co2_ppm = (float)co2;

  return true;
}

AirQualityState evaluateAirQuality(const SensorData &data, AirQualityState currentState) {
  // CO₂ 기준 판단
  AirQualityState newState = COMFORT;

  if (data.co2_ppm > CO2_ALERT) {
    newState = ALERT;
  } else if (data.co2_ppm > CO2_WARNING) {
    newState = WARNING;
  } else {
    // CO₂ COMFORT 기준 충족 시 온습도로 보정
    if (data.temperature < TEMP_LOW || data.temperature > TEMP_HIGH ||
        data.humidity    < HUMID_LOW || data.humidity    > HUMID_HIGH) {
      newState = WARNING;
    } else {
      newState = COMFORT;
    }
  }

  // Hysteresis 적용 (하향 전환 시 더 낮은 기준 사용)
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