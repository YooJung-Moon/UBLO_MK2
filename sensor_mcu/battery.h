#pragma once

#include <Arduino.h>
#include "config.h"

// ── Battery State ─────────────────────────────
enum BatteryState {
  BATTERY_NORMAL,
  BATTERY_POWER_SAVE,
  BATTERY_CRITICAL
};

// ── Test Mode ─────────────────────────────────
#ifdef TEST_MODE
extern uint8_t testBatteryPct;
#endif

// ── Function Declarations ─────────────────────
bool batteryInit();
uint8_t batteryReadPct();
BatteryState evaluateBatteryState(uint8_t pct);