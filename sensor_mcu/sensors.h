#pragma once

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include "config.h"

#ifndef TEST_MODE
#include <DHT.h>
#endif

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
extern uint8_t testBatteryPct;
int getScenarioStep();
#endif

// ── Function Declarations ─────────────────────
bool sensorsInit();
bool sensorsMeasure(SensorData &data);
AirQualityState evaluateAirQuality(const SensorData &data, AirQualityState currentState);