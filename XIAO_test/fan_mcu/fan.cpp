#include "fan.h"

void fanInit() {
  pinMode(PIN_FAN, OUTPUT);
  fanOff();
  Serial.println("Fan initialized");
}

void fanOn() {
#ifndef TEST_MODE
  digitalWrite(PIN_FAN, FAN_ON);
#else
  Serial.println("TEST: fan ON");
#endif
}

void fanOff() {
#ifndef TEST_MODE
  digitalWrite(PIN_FAN, FAN_OFF);
#else
  Serial.println("TEST: fan OFF");
#endif
}