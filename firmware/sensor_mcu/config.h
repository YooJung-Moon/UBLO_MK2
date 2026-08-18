#pragma once
#include <Arduino.h>

// ===================== 모드 선택 =====================
// 아래 두 줄 중 하나만 주석 해제 — TEST_MODE: 짧은 주기/타임아웃으로 빠른 검증
//                                  PRODUCTION_MODE: 실제 운영 주기/타임아웃
// fan_mcu/config.h의 모드 스위치와 항상 같은 쪽으로 맞춰야 두 MCU의 COMMS_LOST_TIMEOUT이 일치한다.
#define TEST_MODE
// #define PRODUCTION_MODE

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
#if defined(TEST_MODE)
    #define BUFFER_SIZE         3      // 테스트용: 3개 (10초 × 3 = 30초)
    #define DECISION_COUNT      3      // 테스트용: 3개 (30초)
    #define COMMS_LOST_TIMEOUT  30000  // 테스트용: 30초
#elif defined(PRODUCTION_MODE)
    #define BUFFER_SIZE         30     // 최근 5분치 데이터 저장 (10초 × 30 = 300초 = 5분)
    #define DECISION_COUNT      30     // 판단 주기: 5분 (10초 × 30 = 5분)
    #define COMMS_LOST_TIMEOUT  600000 // 10분
#else
    #error "config.h: TEST_MODE 또는 PRODUCTION_MODE 중 하나를 정의해야 합니다."
#endif
// 통신 두절 판단 기준 — Fan MCU로부터 마지막 mode_packet 수신 후 COMMS_LOST_TIMEOUT을 초과하면 두절로 판단.
// Fan MCU 측 COMMS_LOST_TIMEOUT과 값은 동일하게 맞췄지만, 각 MCU가 자신의 마지막 수신 시각을
// 독립적으로 기준 삼기 때문에 실제 두절 "감지 시점"은 서로 다를 수 있다.