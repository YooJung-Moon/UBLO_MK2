#pragma once
#include <Arduino.h>

// ESP-NOW — 1:N 페어링 (Sensor MCU 1대 ↔ Fan MCU N대)
// 통신 테스트: Sensor pair 4 ↔ Fan pair 4, 5 (1:2)
#define FAN_MCU_COUNT 2
const uint8_t FAN_MCU_MACS[FAN_MCU_COUNT][6] = {
    // {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x84},  // pair 1_UBLO
    // {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x34},  // pair 2_Finland
    // {0x20, 0x6E, 0xF1, 0x31, 0x30, 0x24},  // pair 3_UBLO
    {0x20, 0x6E, 0xF1, 0x31, 0x2E, 0xE0},  // pair 4_Spare
    {0x20, 0x6E, 0xF1, 0x31, 0x2E, 0xB4},  // pair 5_TEST
};

// CO₂ 임계값 (ppm)
#define CO2_THRESHOLD   1000
#define CO2_DANGER      2000

// 버퍼
#define BUFFER_SIZE     3   // 테스트용: 3개 (10초 x 3 = 30초)
// #define BUFFER_SIZE     30   // 최근 5분치 데이터 저장 (10초 × 30 = 300초 = 5분)

#define DECISION_COUNT  3    // 테스트용: 3개 (30초)
//#define DECISION_COUNT  30   // 판단 주기: 5분 (10초 × 30 = 5분)

// 센서 수집 주기 (ms)
#define SENSOR_INTERVAL 10000  // 10초