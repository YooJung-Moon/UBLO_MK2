#pragma once

// ── Board Selection ───────────────────────────
#define BOARD_XIAO   // 주석 처리하면 Arduino Nano ESP32 기준

// ── Build Mode ───────────────────────────────
#define TEST_MODE   // 주석 처리하면 실제 모드로 전환

// ── Timers (ms) ──────────────────────────────
#define T_NO_PACKET     600000  // 테스트용 10분 (실제: 150000)
#define T_GATE          5000    // 게이트 모터 타임아웃 5s → ERROR

// ── Fan Control ──────────────────────────────
#define FAN_ON   HIGH
#define FAN_OFF  LOW

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_CHANNEL  1

// ── Pin Definitions ───────────────────────────
#ifdef BOARD_XIAO
  #define PIN_MOTOR_IN1   1
  #define PIN_MOTOR_IN2   2
  #define PIN_LIMIT_OPEN  3
  #define PIN_LIMIT_CLOSE 4
  #define PIN_FAN         5
  #define PIN_BUTTON      7
#else  // Arduino Nano ESP32
  #define PIN_MOTOR_IN1   D2
  #define PIN_MOTOR_IN2   D3
  #define PIN_LIMIT_OPEN  D5
  #define PIN_LIMIT_CLOSE D6
  #define PIN_FAN         D9
  #define PIN_BUTTON      D7
#endif

// ── Test Mode ────────────────────────────────
#ifdef TEST_MODE
#define TEST_LIMIT_OPEN_DEFAULT    false
#define TEST_LIMIT_CLOSE_DEFAULT   false
#define TEST_GATE_MOVE_TIME        2000  // 게이트 이동 시뮬레이션 2s
                                         // T_GATE(5s)보다 짧으면 정상 동작
                                         // T_GATE(5s)보다 길면 타임아웃 케이스
#endif