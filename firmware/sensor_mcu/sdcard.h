#pragma once
#include <Arduino.h>

void sdcard_init();
void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity);
void sdcard_log_decision(String timestamp, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd);
void sdcard_log_error(String timestamp, uint8_t error);  // 에러 로깅