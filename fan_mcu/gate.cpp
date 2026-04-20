#include "gate.h"

static GateState gateState     = GATE_STATE_CLOSED;
static uint32_t  gateMoveStart = 0;

#ifdef TEST_MODE
bool testLimitOpen   = TEST_LIMIT_OPEN_DEFAULT;
bool testLimitClosed = TEST_LIMIT_CLOSE_DEFAULT;

// 시나리오에서 게이트 커맨드 수신 시 자동으로 리밋 스위치 상태 변경
void scenarioUpdateLimitSwitch(bool opening) {
#if TEST_SCENARIO == 1 || TEST_SCENARIO == 2
  if (opening) {
    // 게이트 열기 시작 → T_GATE 내에 OPEN 감지 시뮬레이션
    testLimitOpen   = true;
    testLimitClosed = false;
    Serial.println("TEST: limit switch → OPEN");
  } else {
    // 게이트 닫기 시작 → T_GATE 내에 CLOSE 감지 시뮬레이션
    testLimitOpen   = false;
    testLimitClosed = true;
    Serial.println("TEST: limit switch → CLOSE");
  }
#elif TEST_SCENARIO == 3
  // Safe Mode 시나리오: 리밋 스위치 자동 응답 동일
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
  pinMode(PIN_LIMIT_OPEN,  INPUT_PULLUP);
  pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);
  gateStop();
#else
  // 부팅 시 CLOSE 상태로 초기화
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