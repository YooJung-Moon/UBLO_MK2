#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t FAN_MCU_MAC[6] = {0xE4, 0xB0, 0x63, 0xAD, 0xE3, 0x1C};

// CO₂ 임계값 (ppm)
#define CO2_GOOD    570
#define CO2_DANGER  2000

// 센서 수집 주기 (ms)
#define SENSOR_INTERVAL 10000