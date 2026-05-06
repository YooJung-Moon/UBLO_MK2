#pragma once
#include <Arduino.h>

void encoder_init();
bool encoder_changed();
uint8_t encoder_get_mode();