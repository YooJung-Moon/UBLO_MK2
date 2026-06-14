#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "logic.h"

typedef struct {
    uint8_t mode;
    uint8_t fan_cmd;
    uint8_t cover_cmd;
} mode_packet_t;

void comms_init();
void comms_send(command_packet_t packet);
mode_packet_t comms_get_last_mode();
bool comms_mode_available();