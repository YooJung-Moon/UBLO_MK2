#pragma once
#include <Arduino.h>

void sdcard_init();

// raw 센서 데이터: 모든 Fan MCU 폴더에 동일하게 기록
void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity);

// decision/error 로그: 특정 fan_index 폴더에만 기록 (MAC 컬럼 포함)
void sdcard_log_decision(String timestamp, uint8_t fan_index, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd);
void sdcard_log_error(String timestamp, uint8_t fan_index, uint8_t error);