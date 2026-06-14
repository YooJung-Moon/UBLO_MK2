#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>

bool sensors_init();
bool sensors_read(uint16_t &co2, float &temperature, float &humidity);