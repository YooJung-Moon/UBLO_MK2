#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"

// ── Function Declarations ─────────────────────
bool storageInit();
bool storageWrite(float co2_ppm, float temperature, float humidity,
                  uint8_t battery_pct, uint32_t timestamp);
bool storageIsAvailable();