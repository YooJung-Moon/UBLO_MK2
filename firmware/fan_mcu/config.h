#pragma once
#include <Arduino.h>

// ===================== 모드 선택 =====================
// 아래 두 줄 중 하나만 주석 해제
// TEST_MODE_FAST : 모든 타임아웃(OPEN/TURBO/COMMS_LOST)을 짧게 — 전체 흐름 빠르게 검증용
// PRODUCTION_MODE: 실제 운영값
// ※ Fan MCU에는 "송수신 주기만 테스트"하는 모드가 없음 — 그건 Sensor MCU의 BUFFER_SIZE/
//   DECISION_COUNT만 관련된 값이라, Sensor가 TEST_MODE_SYNC_ONLY일 때 Fan은 PRODUCTION_MODE로 둔다.
// #define TEST_MODE_FAST
#define PRODUCTION_MODE

// ESP-NOW
// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x7D, 0x54};  // pair 1_Sonolux 내부용
// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x7A, 0xD4};  // pair 2_Finland_1st
const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAD, 0xE2, 0x14};     // pair 3_UBLO 내부용
// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x68, 0x7C};  // pair 4_Finland_2nd

// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x7C, 0x68};  // pair 5_TEST (Sensor)

// 핀 정의 — 액추에이터
#define MOTOR_IN1   D2
#define MOTOR_IN2   D3
#define BLOWER_EN   D4
#define LIM_CLOSE   D5
#define LIM_OPEN    D6
#define FAN_PWM     D7

// 핀 정의 — Encoder
#define ENC_A_PIN   D8
#define ENC_B_PIN   D9
#define ENC_SW_PIN  D10

// 핀 정의 — LED
#define LED_AUTO_PIN    A0  // PCB LED D2
#define LED_CLOSE_PIN   A1  // PCB LED D3
#define LED_OPEN_PIN    A2  // PCB LED D4
#define LED_TURBO_PIN   A3  // PCB LED D5

// 모드 정의
#define MODE_AUTO    0
#define MODE_CLOSE   1
#define MODE_OPEN    2
#define MODE_TURBO   3

// 풍량 (0~100)
#define FAN_SPEED_PCT 60

// ===================== 타임아웃 (ms) =====================
#define TIMEOUT_CLOSE   0         // 타임아웃 없음 (공통)

#if defined(TEST_MODE_FAST)
    #define TIMEOUT_OPEN        30000   // 테스트용: 30초
    #define TIMEOUT_TURBO       30000   // 테스트용: 30초
    #define COMMS_LOST_TIMEOUT  30000   // 테스트용: 30초
#elif defined(PRODUCTION_MODE)
    #define TIMEOUT_OPEN        14400000  // 4시간
    #define TIMEOUT_TURBO       3600000   // 1시간
    #define COMMS_LOST_TIMEOUT  600000    // 10분
#else
    #error "config.h: TEST_MODE_FAST 또는 PRODUCTION_MODE 중 하나를 정의해야 합니다."
#endif