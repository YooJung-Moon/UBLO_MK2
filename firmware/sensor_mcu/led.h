#pragma once
#include <Arduino.h>

void led_init();
void led_set_co2(uint16_t co2);      // 최신 CO2 값 기반 색상 갱신 (10초 주기 호출)
void led_set_error_sensor();         // 센서 자체 read 실패 시 즉시 흰색 고정 (최우선 표시)
void led_set_comms_lost(bool lost);  // 통신 두절 여부 갱신 — true면 CO2색 느린 깜빡임으로 전환
void led_update();                   // 매 loop마다 호출 — 깜빡임 타이머 처리 (non-blocking)