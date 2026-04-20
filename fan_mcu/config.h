#pragma once

// ── Build Mode ───────────────────────────────
#define TEST_MODE   // 주석 처리하면 실제 모드로 전환

// ── Test Scenario ─────────────────────────────
#ifdef TEST_MODE
#define TEST_SCENARIO  1   // 1: COMFORT→WARNING→COMFORT
                           // 2: COMFORT→ALERT→COMFORT
                           // 3: Safe Mode
#endif

// ── Timers (ms) ──────────────────────────────
#define T_NO_PACKET     30000   // 패킷 미수신 타임아웃 30s → SAFE_MODE
#define T_GATE          5000    // 게이트 모터 타임아웃 5s → ERROR

// ── Fan PWM ──────────────────────────────────
#define FAN_PWM_ON      80
#define FAN_PWM_OFF     0

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_CHANNEL  1

// ── Pin Definitions (XIAO ESP32-S3 Plus 기준) ─
#define PIN_MOTOR_IN1   1
#define PIN_MOTOR_IN2   2
#define PIN_LIMIT_OPEN  3
#define PIN_LIMIT_CLOSE 4
#define PIN_FAN_PWM     5
#define PIN_LED_R       6
#define PIN_LED_G       7
#define PIN_LED_Y       8

// ── Test Mode Default Values ─────────────────
#ifdef TEST_MODE
#define TEST_LIMIT_OPEN_DEFAULT    false
#define TEST_LIMIT_CLOSE_DEFAULT   false
#endif