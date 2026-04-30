#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t SENSOR_MCU_MAC[6] = {0x20, 0x6E, 0xF1, 0x31, 0x25, 0x44};

// 핀 정의
#define MOTOR_IN1   2
#define MOTOR_IN2   3
#define LIM_OPEN    5
#define LIM_CLOSE   6
#define FAN_PWM     18