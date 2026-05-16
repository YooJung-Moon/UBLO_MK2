#include "gate.h"

static GateState gateState     = GATE_STATE_CLOSED;
static uint32_t  gateMoveStart = 0;

#ifdef TEST_MODE
bool testLimitOpen   = TEST_LIMIT_OPEN_DEFAULT;
bool testLimitClosed = TEST_LIMIT_CLOSE_DEFAULT;

void scenarioUpdateLimitSwitch(bool opening) {
#if TEST_SCENARIO == 1 || TEST_SCENARIO == 2
  if (opening) {
    testLimitOpen   = true;
    testLimitClosed = false;
    Serial.println("TEST: limit switch → OPEN");
  } else {
    testLimitOpen   = false;
    testLimitClosed = true;
    Serial.println("TEST: limit switch → CLOSE");
  }
#elif TEST_SCENARIO == 3
  if (opening) {
    testLimitOpen   = true;
    testLimitClosed = false;
    Serial.println("TEST: limit switch → OPEN");
  } else {
    testLimitOpen   = false;
    testLimitClosed = true;
    Serial.println("TEST: limit switch → CLOSE");
  }
#endif
}
#endif

void gateInit() {
#ifndef TEST_MODE
  pinMode(PIN_MOTOR_IN1,   OUTPUT);
  pinMode(PIN_MOTOR_IN2,   OUTPUT);
  pinMode(PIN_LIMIT_OPEN,  INPUT);   // 외부 풀업 R1 (10kΩ) 사용
  pinMode(PIN_LIMIT_CLOSE, INPUT);   // 외부 풀업 R2 (10kΩ) 사용
  gateStop();
#else
  testLimitOpen   = false;
  testLimitClosed = true;
  Serial.println("TEST: gate init → limit CLOSE");
#endif
  Serial.println("Gate initialized");
}

void gateOpen() {
  gateState = GATE_STATE_MOVING;
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
#else
  Serial.println("TEST: gate opening");
  scenarioUpdateLimitSwitch(true);
#endif
}

void gateClose() {
  gateState = GATE_STATE_MOVING;
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);
#else
  Serial.println("TEST: gate closing");
  scenarioUpdateLimitSwitch(false);
#endif
}

void gateStop() {
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
#else
  Serial.println("TEST: gate stopped");
#endif
}

bool isLimitSwitchOpen() {
#ifdef TEST_MODE
  return testLimitOpen;
#else
  // 외부 풀업 + NO 스위치: 눌리면 GND → LOW
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