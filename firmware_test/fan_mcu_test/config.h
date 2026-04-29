#pragma once
#include <Arduino.h>

// ESP-NOW
uint8_t SENSOR_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x25, 0x44};

// 핀 정의
#define MOTOR_IN1   D2
#define MOTOR_IN2   D3
#define LIM_OPEN    D5
#define LIM_CLOSE   D6
#define FAN_PWM     D9