#pragma once
#include <Arduino.h>

// ===================== 모드 선택 =====================
// 아래 세 줄 중 하나만 주석 해제
// TEST_MODE_SYNC_ONLY: Fan-Sensor 송수신 주기(BUFFER_SIZE/DECISION_COUNT)만 30초로 단축,
//                       COMMS_LOST_TIMEOUT은 운영값 그대로 — 이때 Fan MCU는 PRODUCTION_MODE로 둘 것
// TEST_MODE_FAST      : 모든 타임아웃(송수신 주기 + COMMS_LOST_TIMEOUT)을 짧게 — 전체 흐름 빠르게 검증용
// PRODUCTION_MODE     : 실제 운영값
// #define TEST_MODE_SYNC_ONLY
// #define TEST_MODE_FAST
#define PRODUCTION_MODE

// ESP-NOW — 1:N 페어링 (Sensor MCU 1대 ↔ Fan MCU N대)
// 통신 테스트: Sensor pair 3 ↔ Fan pair 3 (1:1)
#define FAN_MCU_COUNT 1
const uint8_t FAN_MCU_MACS[FAN_MCU_COUNT][6] = {
    // {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x84},  // pair 1_Sonolux 내부용
    // {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x34},  // pair 2_Finland
       {0x20, 0x6E, 0xF1, 0x31, 0x30, 0x24},  // pair 3_UBLO
    // {0x20, 0x6E, 0xF1, 0x31, 0x2E, 0xE0},  // pair 4_Finland_2nd
    // {0x20, 0x6E, 0xF1, 0x31, 0x2E, 0xB4},  // pair 5_TEST
};

// CO₂ 임계값 (ppm)
#define CO2_THRESHOLD   1000
#define CO2_DANGER      2000

// 센서 수집 주기 (ms) — 공통
#define SENSOR_INTERVAL 10000  // 10초

// ===================== 버퍼 / 판단 주기 / 통신 두절 기준 =====================
#if defined(TEST_MODE_SYNC_ONLY)
    // 송수신 주기만 30초로 단축, COMMS_LOST_TIMEOUT은 운영값 유지
    #define BUFFER_SIZE         3      // 10초 × 3 = 30초
    #define DECISION_COUNT      3      // 10초 × 3 = 30초
    #define COMMS_LOST_TIMEOUT  600000 // 10분 (운영과 동일)
#elif defined(TEST_MODE_FAST)
    #define BUFFER_SIZE         3      // 10초 × 3 = 30초
    #define DECISION_COUNT      3      // 10초 × 3 = 30초
    #define COMMS_LOST_TIMEOUT  30000  // 테스트용: 30초
#elif defined(PRODUCTION_MODE)
    #define BUFFER_SIZE         30     // 10초 × 30 = 5분
    #define DECISION_COUNT      30     // 10초 × 30 = 5분
    #define COMMS_LOST_TIMEOUT  600000 // 10분
#else
    #error "config.h: TEST_MODE_SYNC_ONLY / TEST_MODE_FAST / PRODUCTION_MODE 중 하나를 정의해야 합니다."
#endif