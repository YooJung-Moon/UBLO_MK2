#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "logic.h"

void comms_init();
void comms_send(command_packet_t packet);
mode_packet_t comms_get_last_mode();
bool comms_mode_available();