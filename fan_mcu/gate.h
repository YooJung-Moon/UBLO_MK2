#pragma once

#include <Arduino.h>
#include "config.h"

// ── Gate State ────────────────────────────────
enum GateState {
  GATE_STATE_OPEN,
  GATE_STATE_CLOSED,
  GATE_STATE_MOVING
};

// ── Test Mode ─────────────────────────────────
#ifdef TEST_MODE
extern bool testLimitOpen;
extern bool testLimitClosed;
void autoUpdateLimitSwitch(bool opening);
bool isGateMoveComplete();
#endif

// ── Function Declarations ─────────────────────
void gateInit();
void gateOpen();
void gateClose();
void gateStop();
bool isLimitSwitchOpen();
bool isLimitSwitchClosed();
bool isGateTimedOut();
void gateStartTimer();
GateState gateGetState();
void gateSetState(GateState state);