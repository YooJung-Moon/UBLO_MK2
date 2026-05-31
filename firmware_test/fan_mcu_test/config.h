#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t SENSOR_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x32, 0xDC};

// 핀 정의 — 액추에이터
#define MOTOR_IN1   D3
#define MOTOR_IN2   D2
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
#define LED_CLOSED_PIN  A1  // PCB LED D3
#define LED_BREEZE_PIN  A2  // PCB LED D4
#define LED_TURBO_PIN   A3  // PCB LED D5

// 타임아웃 (ms)
#define TIMEOUT_CLOSED  0         // 타임아웃 없음
#define TIMEOUT_BREEZE  14400000  // 4시간
#define TIMEOUT_TURBO   3600000   // 1시간

// 모드 정의
#define MODE_AUTO    0
#define MODE_CLOSED  1
#define MODE_BREEZE  2
#define MODE_TURBO   3