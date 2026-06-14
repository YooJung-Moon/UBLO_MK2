#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "logic.h"

void comms_init();
void comms_send(mode_packet_t packet);
command_packet_t comms_get_last_command();
bool comms_command_available();