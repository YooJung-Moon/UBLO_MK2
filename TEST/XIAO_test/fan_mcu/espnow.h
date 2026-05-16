#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

// ── PDU ───────────────────────────────────────
typedef struct {
  uint8_t command;      // 0: OFF, 1: ON
  uint8_t gate;         // 0: CLOSE, 1: OPEN
  uint8_t battery_pct;  // 0–100 (%)
} sensor_to_fan_t;

// ── Fan Command ───────────────────────────────
enum FanCommand {
  CMD_OFF = 0,
  CMD_ON  = 1
};

// ── Gate Command ──────────────────────────────
enum GateCommand {
  GATE_CLOSE = 0,
  GATE_OPEN  = 1
};

// ── Received Packet ───────────────────────────
extern volatile bool     newPacketReceived;
extern volatile sensor_to_fan_t latestPacket;

// ── Function Declarations ─────────────────────
bool espnowInit();
void espnowResetPacket();