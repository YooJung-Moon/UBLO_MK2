#pragma once

// ── Build Mode ───────────────────────────────
#define TEST_MODE   // 주석 처리하면 실제 모드로 전환

// ── Timers (ms) ──────────────────────────────
#define T_NO_PACKET     30000   // 패킷 미수신 타임아웃 30s → SAFE_MODE
#define T_GATE          5000    // 게이트 모터 타임아웃 5s → ERROR

// ── Fan PWM ──────────────────────────────────
#define FAN_PWM_ON      80      // ON PWM duty (%)
#define FAN_PWM_OFF     0       // OFF PWM duty (%)

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_CHANNEL  1       // ESP-NOW 채널

// ── Pin Definitions (XIAO ESP32-S3 Plus 기준) ─
// 실제 연결 후 핀 번호 업데이트 필요
#define PIN_MOTOR_IN1   1       // DRV8871 IN1 (게이트 정방향)
#define PIN_MOTOR_IN2   2       // DRV8871 IN2 (게이트 역방향)
#define PIN_LIMIT_OPEN  3       // 리밋 스위치 OPEN
#define PIN_LIMIT_CLOSE 4       // 리밋 스위치 CLOSE
#define PIN_FAN_PWM     5       // 팬 PWM 출력
#define PIN_LED_R       6       // LED 빨강
#define PIN_LED_G       7       // LED 초록
#define PIN_LED_Y       8       // LED 노랑

// ── Test Mode Default Values ─────────────────
#ifdef TEST_MODE
#define TEST_LIMIT_OPEN_DEFAULT    false
#define TEST_LIMIT_CLOSE_DEFAULT   false
#endif