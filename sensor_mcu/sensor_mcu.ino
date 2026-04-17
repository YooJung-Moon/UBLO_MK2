#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "battery.h"
#include "espnow.h"
#include "storage.h"

// ── FSM States ────────────────────────────────
enum SensorMcuState {
  STATE_INIT,
  STATE_IDLE,
  STATE_MEASURING,
  STATE_TRANSMIT,
  STATE_SD_WRITE,
  STATE_ERROR_SENSOR
};

// ── Global Variables ──────────────────────────
SensorMcuState currentState = STATE_INIT;

AirQualityState airQualityState     = COMFORT;
AirQualityState prevAirQualityState = COMFORT;
uint8_t consecCount  = 0;
uint8_t retryCount   = 0;
uint8_t batteryPct   = 100;
uint8_t sdWriteFailCount = 0;

SensorData sensorData = {0, 0, 0};

uint32_t lastMeasureTime = 0;
uint32_t lastSdWriteTime = 0;

// ── Helper: Air Quality → Fan/Gate Command ────
FanCommand  toFanCommand(AirQualityState state) {
  return (state == COMFORT) ? CMD_OFF : CMD_ON;
}

GateCommand toGateCommand(AirQualityState state) {
  return (state == COMFORT) ? GATE_CLOSE : GATE_OPEN;
}

// ── Setup ─────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Sensor MCU booting...");
  currentState = STATE_INIT;
}

// ── Loop ──────────────────────────────────────
void loop() {
  switch (currentState) {

    // ── INIT ────────────────────────────────
    case STATE_INIT: {
      bool ok = true;

      if (!sensorsInit()) {
        Serial.println("Sensor init failed");
        ok = false;
      }

      if (!batteryInit()) {
        Serial.println("Battery init failed");
        ok = false;
      }

      if (!espnowInit()) {
        Serial.println("ESP-NOW init failed");
        ok = false;
      }

      if (!storageInit()) {
        Serial.println("Storage init failed");
        // SD 카드 실패는 치명적 에러가 아님
        // 센서/통신 실패와 달리 계속 진행
      }

      if (!ok) {
        currentState = STATE_ERROR_SENSOR;
        break;
      }

      // 초기 패킷 전송 (OFF, CLOSE)
      espnowSend(CMD_OFF, GATE_CLOSE, batteryPct);

      lastMeasureTime = millis();
      lastSdWriteTime = millis();
      currentState    = STATE_IDLE;
      Serial.println("Sensor MCU initialized");
      break;
    }

    // ── IDLE ────────────────────────────────
    case STATE_IDLE: {
      if (millis() - lastMeasureTime >= T_MEASURE) {
        lastMeasureTime = millis();
        consecCount     = 0;
        currentState    = STATE_MEASURING;
      }
      // light sleep은 추후 적용
      break;
    }

    // ── MEASURING ───────────────────────────
    case STATE_MEASURING: {
      // 센서 측정
      if (!sensorsMeasure(sensorData)) {
        Serial.println("Sensor measure failed");
        currentState = STATE_IDLE;
        break;
      }

      // 배터리 측정
      batteryPct = batteryReadPct();

      // 공기질 판단
      AirQualityState evaluated = evaluateAirQuality(sensorData, airQualityState);

      if (evaluated == airQualityState) {
        consecCount++;
      } else {
        consecCount = 1;
      }

      // hysteresis 조건 충족 시 상태 전이
      if (consecCount >= CONSEC_THRESHOLD) {
        prevAirQualityState = airQualityState;
        airQualityState     = evaluated;
        consecCount         = 0;
      }

      currentState = STATE_TRANSMIT;
      break;
    }

    // ── TRANSMIT ────────────────────────────
    case STATE_TRANSMIT: {
      FanCommand  fanCmd  = toFanCommand(airQualityState);
      GateCommand gateCmd = toGateCommand(airQualityState);

      bool sent = false;
      retryCount = 0;

      while (!sent && retryCount < ESPNOW_MAX_RETRY) {
        sent = espnowSend(fanCmd, gateCmd, batteryPct);
        if (!sent) {
          retryCount++;
          Serial.printf("ESP-NOW retry %d/%d\n", retryCount, ESPNOW_MAX_RETRY);
          delay(100);
        }
      }

      if (!sent) {
        Serial.println("ESP-NOW send failed after max retries");
      }

      currentState = STATE_SD_WRITE;
      break;
    }

    // ── SD_WRITE ────────────────────────────
    case STATE_SD_WRITE: {
      if (millis() - lastSdWriteTime >= T_SD_WRITE) {
        lastSdWriteTime = millis();

        uint32_t timestamp = millis() / 1000;  // 실제 RTC 있으면 교체

        bool written = storageWrite(
          sensorData.co2_ppm,
          sensorData.temperature,
          sensorData.humidity,
          batteryPct,
          timestamp
        );

        if (!written) {
          sdWriteFailCount++;
          Serial.printf("SD write failed (count: %d)\n", sdWriteFailCount);
        } else {
          Serial.println("SD write success");
        }
      }

      currentState = STATE_IDLE;
      break;
    }

    // ── ERROR_SENSOR ────────────────────────────
    case STATE_ERROR_SENSOR: {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("FATAL: Sensor/ESP-NOW init failed. Halting.");
        errorLogged = true;
      }
      // LED 에러 표시 추후 핀 확정 후 추가
      delay(1000);
      break;
    }

  }
}