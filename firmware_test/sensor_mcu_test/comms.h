#pragma once
#include <esp_now.h>
#include <WiFi.h>

typedef struct {
    uint8_t fan_cmd;    // 0=OFF, 1=ON
    uint8_t cover_cmd;  // 0=CLOSE, 1=OPEN
} command_packet_t;

void comms_init();
void comms_send(command_packet_t packet);