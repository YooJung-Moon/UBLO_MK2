#pragma once
#include <Arduino.h>
#include "config.h"

void buffer_init();
void buffer_add(uint16_t co2);
bool buffer_full();
uint16_t buffer_average();