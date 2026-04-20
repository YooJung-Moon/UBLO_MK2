#include "battery.h"

#ifdef TEST_MODE
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
bool batteryInit() {
  pinMode(PIN_BAT_ADC, INPUT);
  Serial.println("Battery ADC initialized");
  return true;
}

uint8_t batteryReadPct() {
  // 여러 번 샘플링해서 평균 내기 (ADC 노이즈 감소)
  uint32_t adcSum = 0;
  for (int i = 0; i < 10; i++) {
    adcSum += analogRead(PIN_BAT_ADC);
    delay(5);
  }
  float adcAvg = adcSum / 10.0f;

  // ADC 값 → 전압 변환
  float vPin = (adcAvg / BAT_ADC_RES) * BAT_ADC_VREF;

  // 전압 분배 회로 역산 → 실제 배터리 전압
  // Vbat = Vpin * (R_TOP + R_BOT) / R_BOT
  float vBat = vPin * (BAT_R_TOP + BAT_R_BOT) / BAT_R_BOT;

  // 전압 → 퍼센트 변환
  if (vBat >= BAT_V_MAX) return 100;
  if (vBat <= BAT_V_MIN) return 0;

  return (uint8_t)((vBat - BAT_V_MIN) * 100.0f / (BAT_V_MAX - BAT_V_MIN));
}
#endif

BatteryState evaluateBatteryState(uint8_t pct) {
  if (pct >= BATTERY_NORMAL_PCT) return BATTERY_NORMAL;
  if (pct >= BATTERY_PSAVE_PCT)  return BATTERY_POWER_SAVE;
  return BATTERY_CRITICAL;
}