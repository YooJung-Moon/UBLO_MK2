#pragma once
#include <Arduino.h>

// ESP-NOW
const uint8_t FAN_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x84}; // pair 1_UBLO
// const uint8_t FAN_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x2E, 0x24};  // pair 2
// const uint8_t FAN_MCU_MAC[] = {0x20, 0x6E, 0xF1, 0x31, 0x2D, 0x34}; // pair 3 


// CO₂ 임계값 (ppm)
#define CO2_THRESHOLD   1000
#define CO2_DANGER      2000

// 버퍼
#define BUFFER_SIZE     3   // 테스트용: 3개 (30초)    
                            // 10초 × 60 = 10분

// 센서 수집 주기 (ms)
#define SENSOR_INTERVAL 10000