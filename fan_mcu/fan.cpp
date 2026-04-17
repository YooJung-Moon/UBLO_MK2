#include "fan.h"

void fanInit() {
  pinMode(PIN_FAN_PWM, OUTPUT);
  fanOff();
  Serial.println("Fan initialized");
}

void fanOn() {
  analogWrite(PIN_FAN_PWM, map(FAN_PWM_ON, 0, 100, 0, 255));
}

void fanOff() {
  analogWrite(PIN_FAN_PWM, 0);
}