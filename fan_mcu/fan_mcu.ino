#include <Arduino.h>
#include "config.h"
#include "espnow.h"
#include "gate.h"
#include "fan.h"

// ── FSM States ────────────────────────────────
enum FanMcuState {
  STATE_INIT,
  STATE_GATE_CLOSING,
  STATE_STANDBY,
  STATE_GATE_OPENING,
  STATE_FAN_RUNNING,
  STATE_SAFE_MODE,
  STATE_ERROR
};

// ── Global Variables ──────────────────────────
FanMcuState currentState   = STATE_INIT;
uint32_t    lastPacketTime = 0;

// ── Button Debounce ───────────────────────────
static uint32_t lastButtonTime  = 0;
static bool     lastButtonState = HIGH;
#define DEBOUNCE_MS  50

bool isButtonPressed() {
  bool current = digitalRead(PIN_BUTTON);
  if (current == LOW && lastButtonState == HIGH &&
      millis() - lastButtonTime > DEBOUNCE_MS) {
    lastButtonTime  = millis();
    lastButtonState = current;
    return true;
  }
  lastButtonState = current;
  return false;
}

// ── Serial Command Parser (TEST_MODE only) ────
#ifdef TEST_MODE
void parseSerialCommand() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "status") {
    Serial.println("── Status ──────────────────");
    Serial.printf("FSM state   : %s\n",
      currentState == STATE_INIT         ? "INIT" :
      currentState == STATE_GATE_CLOSING ? "GATE_CLOSING" :
      currentState == STATE_STANDBY      ? "STANDBY" :
      currentState == STATE_GATE_OPENING ? "GATE_OPENING" :
      currentState == STATE_FAN_RUNNING  ? "FAN_RUNNING" :
      currentState == STATE_SAFE_MODE    ? "SAFE_MODE" : "ERROR");
    Serial.printf("limit open  : %s\n", testLimitOpen   ? "true" : "false");
    Serial.printf("limit close : %s\n", testLimitClosed ? "true" : "false");

  } else if (input == "help") {
    Serial.println("── Commands ────────────────");
    Serial.println("status      show current state");

  } else {
    Serial.printf("Unknown command: %s\n", input.c_str());
    Serial.println("Type 'help' for commands");
  }
}
#endif

// ── Setup ─────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(3000);

#ifdef TEST_MODE
  Serial.println("=== TEST MODE ===");
#endif

  Serial.println("Fan MCU booting...");

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  gateInit();
  fanInit();

  if (!espnowInit()) {
    Serial.println("ESP-NOW init failed → ERROR");
    currentState = STATE_ERROR;
    return;
  }

  currentState = STATE_INIT;
}

// ── Loop ──────────────────────────────────────
void loop() {
#ifdef TEST_MODE
  parseSerialCommand();
  isGateMoveComplete();  // 딜레이 타이머 체크
#endif

  bool buttonPressed = isButtonPressed();

  switch (currentState) {

    // ── INIT ──────────────────────────────────
    case STATE_INIT: {
      Serial.println("INIT: closing gate");
      gateClose();  // 내부에서 타이머 시작
      currentState = STATE_GATE_CLOSING;
      break;
    }

    // ── GATE_CLOSING ──────────────────────────
    case STATE_GATE_CLOSING: {
      if (isLimitSwitchClosed()) {
        gateStop();
        gateSetState(GATE_STATE_CLOSED);
        Serial.println("GATE_CLOSING → STANDBY");
        lastPacketTime = millis();
        currentState   = STATE_STANDBY;
      } else if (isGateTimedOut()) {
        gateStop();
        Serial.println("GATE_CLOSING: timeout → ERROR");
        currentState = STATE_ERROR;
      }
      // 게이트 이동 중 버튼 무시
      break;
    }

    // ── STANDBY ───────────────────────────────
    case STATE_STANDBY: {
      if (newPacketReceived) {
        lastPacketTime = millis();
        sensor_to_fan_t packet;
        memcpy(&packet, (void *)&latestPacket, sizeof(sensor_to_fan_t));
        espnowResetPacket();

        if (packet.command == CMD_ON) {
          Serial.println("STANDBY: command=ON → GATE_OPENING");
          gateOpen();  // 내부에서 타이머 시작
          currentState = STATE_GATE_OPENING;
        }
      }

      if (buttonPressed) {
        Serial.println("STANDBY: button pressed → GATE_OPENING");
        gateOpen();  // 내부에서 타이머 시작
        currentState = STATE_GATE_OPENING;
      }

      if (millis() - lastPacketTime > T_NO_PACKET) {
        Serial.println("STANDBY: timeout → SAFE_MODE");
        currentState = STATE_SAFE_MODE;
      }
      break;
    }

    // ── GATE_OPENING ──────────────────────────
    case STATE_GATE_OPENING: {
      if (isLimitSwitchOpen()) {
        gateStop();
        gateSetState(GATE_STATE_OPEN);
        fanOn();
        Serial.println("GATE_OPENING → FAN_RUNNING");
        currentState = STATE_FAN_RUNNING;
      } else if (isGateTimedOut()) {
        gateStop();
        Serial.println("GATE_OPENING: timeout → ERROR");
        currentState = STATE_ERROR;
      }
      // 게이트 이동 중 버튼 무시
      break;
    }

    // ── FAN_RUNNING ───────────────────────────
    case STATE_FAN_RUNNING: {
      if (newPacketReceived) {
        lastPacketTime = millis();
        sensor_to_fan_t packet;
        memcpy(&packet, (void *)&latestPacket, sizeof(sensor_to_fan_t));
        espnowResetPacket();

        if (packet.command == CMD_OFF) {
          fanOff();
          Serial.println("FAN_RUNNING: command=OFF → GATE_CLOSING");
          gateClose();  // 내부에서 타이머 시작
          currentState = STATE_GATE_CLOSING;
        }
      }

      if (buttonPressed) {
        fanOff();
        Serial.println("FAN_RUNNING: button pressed → GATE_CLOSING");
        gateClose();  // 내부에서 타이머 시작
        currentState = STATE_GATE_CLOSING;
      }

      if (millis() - lastPacketTime > T_NO_PACKET) {
        fanOff();
        Serial.println("FAN_RUNNING: timeout → SAFE_MODE");
        currentState = STATE_SAFE_MODE;
      }
      break;
    }

    // ── SAFE_MODE ─────────────────────────────
    case STATE_SAFE_MODE: {
      static bool safeModeLogged = false;
      static bool gateStopLogged = false;

      if (!safeModeLogged) {
        fanOff();
        Serial.println("SAFE_MODE: fan OFF, closing gate");
        safeModeLogged = true;
        gateClose();  // 내부에서 타이머 시작
      }

      if (!isLimitSwitchClosed()) {
        if (isGateTimedOut()) {
          gateClose();  // 타임아웃 시 재시도
        }
      } else if (!gateStopLogged) {
        gateStop();
        gateStopLogged = true;
      }

      if (buttonPressed) {
        safeModeLogged = false;
        gateStopLogged = false;
        Serial.println("SAFE_MODE: button pressed → GATE_OPENING");
        gateOpen();  // 내부에서 타이머 시작
        currentState = STATE_GATE_OPENING;
      }

      if (newPacketReceived) {
        lastPacketTime = millis();
        espnowResetPacket();
        safeModeLogged = false;
        gateStopLogged = false;
        Serial.println("SAFE_MODE: packet received → STANDBY");
        currentState   = STATE_STANDBY;
      }
      break;
    }

    // ── ERROR ─────────────────────────────────
    case STATE_ERROR: {
      static bool errorLogged = false;
      if (!errorLogged) {
        fanOff();
        gateStop();
        Serial.println("ERROR: manual reset required");
        errorLogged = true;
      }

      if (buttonPressed) {
        Serial.println("ERROR: button pressed → INIT");
        errorLogged  = false;
        currentState = STATE_INIT;
      }
      delay(100);
      break;
    }
  }
}