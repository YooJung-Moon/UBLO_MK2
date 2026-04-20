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

// ── LED Helper ────────────────────────────────
void setLed(uint8_t battery_pct) {
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_Y, LOW);

  if (battery_pct >= 60) {
    digitalWrite(PIN_LED_G, HIGH);
  } else if (battery_pct >= 20) {
    digitalWrite(PIN_LED_Y, millis() % 1000 < 500 ? HIGH : LOW);
  } else {
    digitalWrite(PIN_LED_R, millis() % 500 < 250 ? HIGH : LOW);
  }
}

// ── Serial Command Parser (TEST_MODE only) ────
#ifdef TEST_MODE
void parseSerialCommand() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "lim:open") {
    testLimitOpen   = true;
    testLimitClosed = false;
    Serial.println("TEST: limit switch OPEN");

  } else if (input == "lim:close") {
    testLimitOpen   = false;
    testLimitClosed = true;
    Serial.println("TEST: limit switch CLOSE");

  } else if (input == "lim:none") {
    testLimitOpen   = false;
    testLimitClosed = false;
    Serial.println("TEST: limit switch NONE (moving)");

  } else if (input == "status") {
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
    Serial.println("lim:open    limit switch OPEN");
    Serial.println("lim:close   limit switch CLOSE");
    Serial.println("lim:none    no limit switch");
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

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_Y, OUTPUT);

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
#endif

  switch (currentState) {

    // ── INIT ──────────────────────────────────
    case STATE_INIT: {
      Serial.println("INIT: closing gate");
      gateClose();
      delay(100);
      gateStartTimer();
      currentState = STATE_GATE_CLOSING;
      break;
    }

    // ── GATE_CLOSING ──────────────────────────
    case STATE_GATE_CLOSING: {
      if (isLimitSwitchClosed()) {
        gateStop();
        gateSetState(GATE_STATE_CLOSED);
        Serial.println("GATE_CLOSING → STANDBY");
        lastPacketTime = millis() + 60000;  // 초기 60초 유예 추가
        currentState   = STATE_STANDBY;
      } else if (isGateTimedOut()) {
        gateStop();
        Serial.println("GATE_CLOSING: timeout → ERROR");
        currentState = STATE_ERROR;
      }
      break;
    }

    // ── STANDBY ───────────────────────────────
    case STATE_STANDBY: {
      if (newPacketReceived) {
        setLed(latestPacket.battery_pct);
        lastPacketTime = millis();
        sensor_to_fan_t packet;
        memcpy(&packet, (void *)&latestPacket, sizeof(sensor_to_fan_t));
        espnowResetPacket();

        if (packet.command == CMD_ON) {
          Serial.println("STANDBY: command=ON → GATE_OPENING");
          gateOpen();
          gateStartTimer();
          currentState = STATE_GATE_OPENING;
        }
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
      break;
    }

    // ── FAN_RUNNING ───────────────────────────
    case STATE_FAN_RUNNING: {
      if (newPacketReceived) {
        setLed(latestPacket.battery_pct);
        lastPacketTime = millis();
        sensor_to_fan_t packet;
        memcpy(&packet, (void *)&latestPacket, sizeof(sensor_to_fan_t));
        espnowResetPacket();

        if (packet.command == CMD_OFF) {
          fanOff();
          Serial.println("FAN_RUNNING: command=OFF → GATE_CLOSING");
          gateClose();
          gateStartTimer();
          currentState = STATE_GATE_CLOSING;
        }
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
      }

      if (!isLimitSwitchClosed()) {
        gateClose();
        gateStartTimer();
      } else if (!gateStopLogged) {
        gateStop();
        gateStopLogged = true;
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
      digitalWrite(PIN_LED_R, millis() % 500 < 250 ? HIGH : LOW);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_LED_Y, LOW);
      delay(100);
      break;
    }
  }
}