#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ── Battery State ─────────────────────────────
enum BatteryState {
  BATTERY_NORMAL,
  BATTERY_POWER_SAVE,
  BATTERY_CRITICAL
};

// ── Function Declarations ─────────────────────
bool batteryInit();
uint8_t batteryReadPct();
BatteryState evaluateBatteryState(uint8_t pct);