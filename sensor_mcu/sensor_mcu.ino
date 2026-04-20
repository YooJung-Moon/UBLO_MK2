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
SensorMcuState currentState         = STATE_INIT;
AirQualityState airQualityState     = COMFORT;
AirQualityState prevAirQualityState = COMFORT;
uint8_t consecCount      = 0;
uint8_t retryCount       = 0;
uint8_t batteryPct       = 100;
uint8_t sdWriteFailCount = 0;
SensorData sensorData    = {0, 0, 0};
uint32_t lastMeasureTime = 0;
uint32_t lastSdWriteTime = 0;

// ── Helper: Air Quality → Fan/Gate Command ────
FanCommand toFanCommand(AirQualityState state) {
  return (state == COMFORT) ? CMD_OFF : CMD_ON;
}

GateCommand toGateCommand(AirQualityState state) {
  return (state == COMFORT) ? GATE_CLOSE : GATE_OPEN;
}

// ── Serial Command Parser (TEST_MODE only) ────
#ifdef TEST_MODE
void parseSerialCommand() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.startsWith("co2:")) {
    testSensorData.co2_ppm = input.substring(4).toFloat();
    Serial.printf("TEST: co2 set to %.1f ppm\n", testSensorData.co2_ppm);

  } else if (input.startsWith("temp:")) {
    testSensorData.temperature = input.substring(5).toFloat();
    Serial.printf("TEST: temp set to %.1f C\n", testSensorData.temperature);

  } else if (input.startsWith("humid:")) {
    testSensorData.humidity = input.substring(6).toFloat();
    Serial.printf("TEST: humid set to %.1f %%\n", testSensorData.humidity);

  } else if (input.startsWith("bat:")) {
    testBatteryPct = (uint8_t)input.substring(4).toInt();
    Serial.printf("TEST: battery set to %d%%\n", testBatteryPct);

  } else if (input == "status") {
    Serial.printf("── Status ──────────────────\n");
    Serial.printf("co2      : %.1f ppm\n", testSensorData.co2_ppm);
    Serial.printf("temp     : %.1f C\n",   testSensorData.temperature);
    Serial.printf("humid    : %.1f %%\n",  testSensorData.humidity);
    Serial.printf("battery  : %d%%\n",     testBatteryPct);
    Serial.printf("state    : %s\n",
      airQualityState == COMFORT ? "COMFORT" :
      airQualityState == WARNING ? "WARNING" : "ALERT");
    Serial.printf("consec   : %d/%d\n",    consecCount, CONSEC_THRESHOLD);

  } else if (input == "help") {
    Serial.println("── Commands ────────────────");
    Serial.println("co2:<value>    set CO2 ppm");
    Serial.println("temp:<value>   set temperature C");
    Serial.println("humid:<value>  set humidity %");
    Serial.println("bat:<value>    set battery %");
    Serial.println("status         show current values");

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

  Serial.println("Sensor MCU booting...");
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
      bool ok = true;

      if (!sensorsInit()) { ok = false; }
      if (!batteryInit()) { ok = false; }
      if (!espnowInit())  { ok = false; }

#ifndef TEST_MODE
      if (!storageInit()) {
        Serial.println("Storage init failed (continuing)");
      }
#endif

      if (!ok) {
        currentState = STATE_ERROR_SENSOR;
        break;
      }

      espnowSend(CMD_OFF, GATE_CLOSE, batteryPct);
      lastMeasureTime = millis();
      lastSdWriteTime = millis();
      currentState    = STATE_IDLE;
      Serial.println("Sensor MCU initialized");
      break;
    }

    // ── IDLE ──────────────────────────────────
    case STATE_IDLE: {
      if (millis() - lastMeasureTime >= T_MEASURE) {
        lastMeasureTime = millis();
        consecCount     = 0;
        currentState    = STATE_MEASURING;
      }
      break;
    }

    // ── MEASURING ─────────────────────────────
    case STATE_MEASURING: {
      if (!sensorsMeasure(sensorData)) {
        Serial.println("Sensor measure failed");
        currentState = STATE_IDLE;
        break;
      }

      batteryPct = batteryReadPct();

      AirQualityState evaluated = evaluateAirQuality(sensorData, airQualityState);

      if (evaluated == airQualityState) {
        consecCount++;
      } else {
        consecCount = 1;
      }

      // 디버그 추가
      Serial.printf("DEBUG: evaluated=%s current=%s consec=%d\n",
        evaluated == COMFORT ? "COMFORT" : evaluated == WARNING ? "WARNING" : "ALERT",
        airQualityState == COMFORT ? "COMFORT" : airQualityState == WARNING ? "WARNING" : "ALERT",
        consecCount);

      if (consecCount >= CONSEC_THRESHOLD) {
        prevAirQualityState = airQualityState;
        airQualityState     = evaluated;
        consecCount         = 0;
        Serial.printf("State: %s → %s\n",
          prevAirQualityState == COMFORT ? "COMFORT" :
          prevAirQualityState == WARNING ? "WARNING" : "ALERT",
          airQualityState == COMFORT ? "COMFORT" :
          airQualityState == WARNING ? "WARNING" : "ALERT");
      }

      currentState = STATE_TRANSMIT;
      break;
    }

    // ── TRANSMIT ──────────────────────────────
    case STATE_TRANSMIT: {
      FanCommand  fanCmd  = toFanCommand(airQualityState);
      GateCommand gateCmd = toGateCommand(airQualityState);

    #ifdef TEST_MODE
      // 시나리오 3: step 3 이후 패킷 전송 중단
      #if TEST_SCENARIO == 3
      if (getScenarioStep() >= 3) {
        Serial.println("TEST: scenario 3 — stopping packet transmission");
        currentState = STATE_SD_WRITE;
        break;
      }
      #endif
    #endif

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

    // ── SD_WRITE ──────────────────────────────
    case STATE_SD_WRITE: {
#ifndef TEST_MODE
      if (millis() - lastSdWriteTime >= T_SD_WRITE) {
        lastSdWriteTime = millis();

        uint32_t timestamp = millis() / 1000;

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
#endif
      currentState = STATE_IDLE;
      break;
    }

    // ── ERROR_SENSOR ──────────────────────────
    case STATE_ERROR_SENSOR: {
      static bool errorLogged = false;
      if (!errorLogged) {
        Serial.println("FATAL: Sensor/ESP-NOW init failed. Halting.");
        errorLogged = true;
      }
      delay(1000);
      break;
    }
  }
}