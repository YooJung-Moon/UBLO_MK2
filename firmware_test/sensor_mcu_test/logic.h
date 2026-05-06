#pragma once
#include <Arduino.h>
#include "config.h"

typedef struct {
    uint8_t fan_cmd;    // 0=OFF, 1=ON
    uint8_t cover_cmd;  // 0=CLOSE, 1=OPEN
} command_packet_t;

command_packet_t logic_decide(uint16_t avg_co2);