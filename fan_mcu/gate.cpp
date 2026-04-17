#include "gate.h"

static GateState gateState     = GATE_STATE_CLOSED;
static uint32_t  gateMoveStart = 0;

#ifdef TEST_MODE
bool testLimitOpen   = TEST_LIMIT_OPEN_DEFAULT;
bool testLimitClosed = TEST_LIMIT_CLOSE_DEFAULT;
#endif

void gateInit() {
#ifndef TEST_MODE
  pinMode(PIN_MOTOR_IN1,   OUTPUT);
  pinMode(PIN_MOTOR_IN2,   OUTPUT);
  pinMode(PIN_LIMIT_OPEN,  INPUT_PULLUP);
  pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);
  gateStop();
#endif
  Serial.println("Gate initialized");
}

void gateOpen() {
  gateState = GATE_STATE_MOVING;
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
#else
  Serial.println("TEST MODE: gate opening");
#endif
}

void gateClose() {
  gateState = GATE_STATE_MOVING;
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);
#else
  Serial.println("TEST MODE: gate closing");
#endif
}

void gateStop() {
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
#else
  Serial.println("TEST MODE: gate stopped");
#endif
}

bool isLimitSwitchOpen() {
#ifdef TEST_MODE
  return testLimitOpen;
#else
  return digitalRead(PIN_LIMIT_OPEN) == LOW;
#endif
}

bool isLimitSwitchClosed() {
#ifdef TEST_MODE
  return testLimitClosed;
#else
  return digitalRead(PIN_LIMIT_CLOSE) == LOW;
#endif
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

void gateSetState(GateState state) {
  gateState = state;
}