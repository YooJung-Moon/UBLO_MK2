#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x68, 0x7C}; // pair 1_UBLO
// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAE, 0x7A, 0xD4}; // pair 2
// const uint8_t SENSOR_MCU_MAC[] = {0xE4, 0xB0, 0x63, 0xAD, 0xE2, 0x14}; // pair 3

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

// 타임아웃 (ms)
#define TIMEOUT_CLOSE   0         // 타임아웃 없음
#define TIMEOUT_OPEN    14400000  // 4시간
#define TIMEOUT_TURBO   3600000   // 1시간

// 모드 정의
#define MODE_AUTO    0
#define MODE_CLOSE   1
#define MODE_OPEN    2
#define MODE_TURBO   3