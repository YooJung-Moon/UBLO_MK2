#pragma once

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <SensirionI2cSht4x.h>
#include "config.h"

// ── Air Quality State ─────────────────────────
enum AirQualityState {
  COMFORT,
  WARNING,
  ALERT
};

// ── Sensor Data ───────────────────────────────
struct SensorData {
  float co2_ppm;
  float temperature;
  float humidity;
};

// ── Test Mode ─────────────────────────────────
#ifdef TEST_MODE
extern SensorData testSensorData;
#endif

// ── Function Declarations ─────────────────────
bool sensorsInit();
bool sensorsMeasure(SensorData &data);
AirQualityState evaluateAirQuality(const SensorData &data, AirQualityState currentState);