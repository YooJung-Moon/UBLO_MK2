#pragma once
#include <Arduino.h>

void encoder_init();
void encoder_update();
bool encoder_changed();
uint8_t encoder_get_mode();
void encoder_set_mode(uint8_t mode);
void encoder_error_blink();   // 커버 타임아웃 에러 시 호출
bool encoder_is_error();      // 현재 에러 상태 확인