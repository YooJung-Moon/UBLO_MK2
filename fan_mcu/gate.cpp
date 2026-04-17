#include "gate.h"

static GateState gateState     = GATE_STATE_CLOSED;
static uint32_t  gateMoveStart = 0;

void gateInit() {
  pinMode(PIN_MOTOR_IN1,   OUTPUT);
  pinMode(PIN_MOTOR_IN2,   OUTPUT);
  pinMode(PIN_LIMIT_OPEN,  INPUT_PULLUP);
  pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);

  gateStop();
  Serial.println("Gate initialized");
}

void gateOpen() {
  gateState = GATE_STATE_MOVING;
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
}

void gateClose() {
  gateState = GATE_STATE_MOVING;
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);
}

void gateStop() {
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
}

bool isLimitSwitchOpen() {
  return digitalRead(PIN_LIMIT_OPEN) == LOW;  // INPUT_PULLUP이라 LOW가 눌린 상태
}

bool isLimitSwitchClosed() {
  return digitalRead(PIN_LIMIT_CLOSE) == LOW;
}

bool isGateTimedOut() {
  return (millis() - gateMoveStart) > T_GATE;
}

void gateStartTimer() {
  gateMoveStart = millis();
}

GateState gateGetState() {
  return gateState;
}