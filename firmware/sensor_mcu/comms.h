#pragma once
#include <esp_now.h>
#include <WiFi.h>

// 패킷 구조체
typedef struct {
    uint8_t requested_mode; // 0=AUTO, 1=CLOSED, 2=BREEZE, 3=TURBO
} user_input_packet_t;

typedef struct {
    uint8_t mode;
    uint8_t fan_cmd;    // 0=OFF, 1=ON
    uint8_t cover_cmd;  // 0=CLOSE, 1=OPEN
} command_packet_t;

void comms_init();
void comms_send(command_packet_t packet);
user_input_packet_t comms_get_last_input();
bool comms_input_available();