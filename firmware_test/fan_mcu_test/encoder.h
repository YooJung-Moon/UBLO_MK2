#pragma once
#include <Arduino.h>

void encoder_init();
void encoder_update();   // loop()에서 항상 호출
bool encoder_changed();
uint8_t encoder_get_mode();
void encoder_set_mode(uint8_t mode);