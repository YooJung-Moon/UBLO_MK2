#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t SENSOR_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x25, 0x44};

// 핀 정의
#define MOTOR_IN1   D2
#define MOTOR_IN2   D3
#define LIM_OPEN    D5
#define LIM_CLOSE   D6
#define FAN_PWM     D9

// 타임아웃 (ms)
#define TIMEOUT_CLOSED  0         // 타임아웃 없음
#define TIMEOUT_BREEZE  14400000  // 4시간
#define TIMEOUT_TURBO   3600000   // 1시간

// 모드 정의
#define MODE_AUTO    0
#define MODE_CLOSED  1
#define MODE_BREEZE  2
#define MODE_TURBO   3