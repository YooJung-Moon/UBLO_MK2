#pragma once
#include <Arduino.h>
#include "config.h"

typedef struct {
    uint8_t fan_cmd;    // 0=OFF, 1=ON
    uint8_t cover_cmd;  // 0=CLOSE, 1=OPEN
} command_packet_t;

typedef struct {
    uint8_t mode;
    uint8_t fan_cmd;
    uint8_t cover_cmd;
} mode_packet_t;

void logic_init();
mode_packet_t logic_update(uint8_t current_mode, command_packet_t cmd, unsigned long entry_time);