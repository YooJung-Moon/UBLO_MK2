#include "gate.h"

static GateState gateState     = GATE_STATE_CLOSED;
static uint32_t  gateMoveStart = 0;

#ifdef TEST_MODE
bool testLimitOpen   = TEST_LIMIT_OPEN_DEFAULT;
bool testLimitClosed = TEST_LIMIT_CLOSE_DEFAULT;

static bool     testGateMoving    = false;
static bool     testGateOpening   = false;
static uint32_t testGateMoveStart = 0;

void autoUpdateLimitSwitch(bool opening) {
  testGateMoving    = true;
  testGateOpening   = opening;
  testGateMoveStart = millis();
  Serial.printf("TEST: gate move started (%s), will complete in %dms\n",
    opening ? "opening" : "closing", TEST_GATE_MOVE_TIME);
}

bool isGateMoveComplete() {
  if (!testGateMoving) return false;
  if (millis() - testGateMoveStart >= TEST_GATE_MOVE_TIME) {
    testGateMoving = false;
    if (testGateOpening) {
      testLimitOpen   = true;
      testLimitClosed = false;
      Serial.println("TEST: limit switch → OPEN");
    } else {
      testLimitOpen   = false;
      testLimitClosed = true;
      Serial.println("TEST: limit switch → CLOSE");
    }
    return true;
  }
  return false;
}
#endif

void gateInit() {
#ifndef TEST_MODE
  pinMode(PIN_MOTOR_IN1,   OUTPUT);
  pinMode(PIN_MOTOR_IN2,   OUTPUT);
  pinMode(PIN_LIMIT_OPEN,  INPUT);
  pinMode(PIN_LIMIT_CLOSE, INPUT);
  gateStop();
#else
  testLimitOpen   = false;
  testLimitClosed = true;
  testGateMoving  = false;
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
  autoUpdateLimitSwitch(true);
#endif
}

void gateClose() {
  gateState = GATE_STATE_MOVING;
#ifndef TEST_MODE
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, HIGH);
#else
  Serial.println("TEST: gate closing");
  autoUpdateLimitSwitch(false);
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