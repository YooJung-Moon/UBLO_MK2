#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CScd4x.h>

bool sensors_init();
bool sensors_read(float &co2, float &temperature, float &humidity);