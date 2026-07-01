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
mode_packet_t logic_update(uint8_t current_mode, command_packet_t cmd,
                           unsigned long entry_time, unsigned long last_packet_time);
bool logic_is_comms_lost_close();   // 통신 두절로 CLOSE 전환된 상태인지 확인
void logic_reset_comms_lost();      // 사용자 직접 조작 시 comms_lost_close 플래그 리셋