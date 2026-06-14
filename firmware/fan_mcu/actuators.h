#pragma once
#include <Arduino.h>

void actuators_init();
void fan_set(uint8_t cmd);        // 0=OFF, 1=ON
void cover_set(uint8_t cmd);      // 0=CLOSE, 1=OPEN