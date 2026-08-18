#pragma once
#include <Arduino.h>

void encoder_init();
void encoder_update();
bool encoder_changed();
uint8_t encoder_get_mode();
void encoder_set_mode(uint8_t mode);
void encoder_error_blink();          // 커버 타임아웃 에러 시 호출 (LED 4개 동시 깜빡임)
bool encoder_is_error();             // 현재 에러 상태 확인
void encoder_set_comms_lost(bool lost); // AUTO 모드 통신 두절 상태 표시 (LED_AUTO_PIN 1개만 깜빡임)