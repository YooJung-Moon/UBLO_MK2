#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "logic.h"

void comms_init();
void comms_send(uint8_t fan_index, command_packet_t packet);  // 특정 Fan MCU에 전송
mode_packet_t comms_get_last_mode(uint8_t fan_index);
bool comms_mode_available(uint8_t fan_index);
bool comms_is_lost(uint8_t fan_index);  // 특정 Fan MCU와의 통신이 두절 상태인지 확인
bool comms_any_lost();                  // 등록된 Fan MCU 중 하나라도 두절이면 true (LED 표시용)