#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "logic.h"

void comms_init();
void comms_send(mode_packet_t packet);
command_packet_t comms_get_last_command();
bool comms_command_available();
unsigned long comms_last_cmd_age();  // 마지막 command_packet 수신으로부터 경과 시간 (ms)