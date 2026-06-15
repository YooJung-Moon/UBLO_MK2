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
    uint8_t error;      // 0=정상, 1=cover_open_timeout, 2=cover_close_timeout
} mode_packet_t;

void logic_init();
command_packet_t logic_decide(uint16_t avg_co2);  // CO₂ 평균값으로 fan/cover 판단