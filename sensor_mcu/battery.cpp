#include "battery.h"

#ifdef TEST_MODE
// ── Test Mode Variables ───────────────────────
uint8_t testBatteryPct = TEST_BAT_DEFAULT;

bool batteryInit() {
  Serial.println("TEST MODE: battery init skipped");
  return true;
}

uint8_t batteryReadPct() {
  Serial.printf("TEST MODE: battery=%d%%\n", testBatteryPct);
  return testBatteryPct;
}

#else
// ── Real Mode ─────────────────────────────────
#define BQ25798_ADDR  0x6B
#define REG_VBAT_ADC  0x52
#define VBAT_MAX      8400
#define VBAT_MIN      6000

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
  Wire.beginTransmission(BQ25798_ADDR);
  Wire.write(REG_VBAT_ADC);
  Wire.endTransmission(false);
  Wire.requestFrom(BQ25798_ADDR, 2);

  if (Wire.available() < 2) {
    Serial.println("BQ25798 read failed");
    return 0;
  }

  uint16_t raw    = (Wire.read() << 8) | Wire.read();
  uint16_t vbat_mv = raw;

  if (vbat_mv >= VBAT_MAX) return 100;
  if (vbat_mv <= VBAT_MIN) return 0;

  return (uint8_t)((vbat_mv - VBAT_MIN) * 100 / (VBAT_MAX - VBAT_MIN));
}
#endif

BatteryState evaluateBatteryState(uint8_t pct) {
  if (pct >= BATTERY_NORMAL_PCT) return BATTERY_NORMAL;
  if (pct >= BATTERY_PSAVE_PCT)  return BATTERY_POWER_SAVE;
  return BATTERY_CRITICAL;
}