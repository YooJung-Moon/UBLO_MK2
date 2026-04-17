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
FanMcuState currentState    = STATE_INIT;
uint32_t    lastPacketTime  = 0;

// ── LED Helper ────────────────────────────────
void setLed(uint8_t battery_pct) {
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_Y, LOW);

  if (battery_pct >= 60) {
    digitalWrite(PIN_LED_G, HIGH);       // NORMAL: 초록 상시 점등
  } else if (battery_pct >= 20) {
    digitalWrite(PIN_LED_Y, millis() % 1000 < 500 ? HIGH : LOW);  // POWER_SAVE: 노랑 점멸
  } else {
    digitalWrite(PIN_LED_R, millis() % 500 < 250 ? HIGH : LOW);   // CRITICAL: 빨강 점멸
  }
}

// ── Setup ─────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(3000);
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
  switch (currentState) {

    // ── INIT ──────────────────────────────────
    case STATE_INIT: {
      Serial.println("INIT: closing gate");
      gateClose();
      gateStartTimer();
      currentState = STATE_GATE_CLOSING;
      break;
    }

    // ── GATE_CLOSING ──────────────────────────
    case STATE_GATE_CLOSING: {
      if (isLimitSwitchClosed()) {
        gateStop();
        Serial.println("GATE_CLOSING: limit switch CLOSE detected → STANDBY");
        lastPacketTime = millis();
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
      // LED 업데이트
      if (newPacketReceived) {
        setLed(latestPacket.battery_pct);
      }

      // 패킷 수신 처리
      if (newPacketReceived) {
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
        // command=OFF면 STANDBY 유지
      }

      // 패킷 타임아웃 체크
      if (millis() - lastPacketTime > T_NO_PACKET) {
        Serial.println("STANDBY: packet timeout → SAFE_MODE");
        currentState = STATE_SAFE_MODE;
      }
      break;
    }

    // ── GATE_OPENING ──────────────────────────
    case STATE_GATE_OPENING: {
      if (isLimitSwitchOpen()) {
        gateStop();
        fanOn();
        Serial.println("GATE_OPENING: limit switch OPEN detected → FAN_RUNNING");
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
      // LED 업데이트
      if (newPacketReceived) {
        setLed(latestPacket.battery_pct);
      }

      // 패킷 수신 처리
      if (newPacketReceived) {
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
        // command=ON이면 FAN_RUNNING 유지
      }

      // 패킷 타임아웃 체크
      if (millis() - lastPacketTime > T_NO_PACKET) {
        fanOff();
        Serial.println("FAN_RUNNING: packet timeout → SAFE_MODE");
        currentState = STATE_SAFE_MODE;
      }
      break;
    }

    // ── SAFE_MODE ─────────────────────────────
    case STATE_SAFE_MODE: {
      fanOff();
      gateClose();
      gateStartTimer();

      // 게이트 닫힘 확인
      if (isLimitSwitchClosed()) {
        gateStop();
      }

      // 유효한 패킷 수신 시 복귀
      if (newPacketReceived) {
        lastPacketTime = millis();
        espnowResetPacket();
        Serial.println("SAFE_MODE: packet received → STANDBY");
        currentState = STATE_STANDBY;
      }
      break;
    }

    // ── ERROR ─────────────────────────────────────
    case STATE_ERROR: {
        static bool errorLogged = false;  // 한 번만 출력하기 위한 플래그
        if (!errorLogged) {
            fanOff();
            gateStop();
            Serial.println("ERROR: manual reset required");
            errorLogged = true;
        }
        // LED 빨강 점멸
        digitalWrite(PIN_LED_R, millis() % 500 < 250 ? HIGH : LOW);
        digitalWrite(PIN_LED_G, LOW);
        digitalWrite(PIN_LED_Y, LOW);
        delay(100);
        break;
    }
  }
}