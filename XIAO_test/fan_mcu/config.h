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
#define T_NO_PACKET     60000   // 패킷 미수신 타임아웃 60s → SAFE_MODE
#define T_GATE          5000    // 게이트 모터 타임아웃 5s → ERROR

// ── Fan Control ──────────────────────────────
// AO3400A MOSFET으로 12V ON/OFF 제어. PWM 아님.
#define FAN_ON   HIGH
#define FAN_OFF  LOW

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_CHANNEL  1

// ── Pin Definitions (Arduino Nano ESP32 기준) ─
#define PIN_MOTOR_IN1   D2   // DRV8871 IN1 (게이트 정방향)
#define PIN_MOTOR_IN2   D3   // DRV8871 IN2 (게이트 역방향)
#define PIN_LIMIT_OPEN  D5   // 리밋 스위치 OPEN (외부 풀업 10kΩ)
#define PIN_LIMIT_CLOSE D6   // 리밋 스위치 CLOSE (외부 풀업 10kΩ)
#define PIN_FAN         D9   // AO3400A 게이트 (HIGH=팬 ON, LOW=팬 OFF)

// ── Test Mode Default Values ─────────────────
#ifdef TEST_MODE
#define TEST_LIMIT_OPEN_DEFAULT    false
#define TEST_LIMIT_CLOSE_DEFAULT   false
#endif