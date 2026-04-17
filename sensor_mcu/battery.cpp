#include "battery.h"

// BQ25798 I2C 주소
#define BQ25798_ADDR 0x6B

// BQ25798 레지스터
#define REG_VBAT_ADC 0x52  // 배터리 전압 ADC 레지스터

// 2S Li-ion 전압 범위
#define VBAT_MAX 8400  // 만충 8.4V (mV)
#define VBAT_MIN 6000  // 컷오프 6.0V (mV)

bool batteryInit() {
  Wire.beginTransmission(BQ25798_ADDR);
  uint8_t error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("BQ25798 init failed");
    return false;
  }
  Serial.println("BQ25798 initialized");
  return true;
}

uint8_t batteryReadPct() {
  // BQ25798 VBAT_ADC 레지스터 읽기 (2바이트)
  Wire.beginTransmission(BQ25798_ADDR);
  Wire.write(REG_VBAT_ADC);
  Wire.endTransmission(false);
  Wire.requestFrom(BQ25798_ADDR, 2);

  if (Wire.available() < 2) {
    Serial.println("BQ25798 read failed");
    return 0;
  }

  uint16_t raw = (Wire.read() << 8) | Wire.read();

  // mV 단위로 변환 (BQ25798 ADC 해상도: 1mV/LSB)
  uint16_t vbat_mv = raw;

  // 전압 → 퍼센트 변환
  if (vbat_mv >= VBAT_MAX) return 100;
  if (vbat_mv <= VBAT_MIN) return 0;

  return (uint8_t)((vbat_mv - VBAT_MIN) * 100 / (VBAT_MAX - VBAT_MIN));
}

BatteryState evaluateBatteryState(uint8_t pct) {
  if (pct >= BATTERY_NORMAL_PCT)    return BATTERY_NORMAL;
  if (pct >= BATTERY_PSAVE_PCT)     return BATTERY_POWER_SAVE;
  return BATTERY_CRITICAL;
}