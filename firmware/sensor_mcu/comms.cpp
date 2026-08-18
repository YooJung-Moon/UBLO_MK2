#include "comms.h"
#include "config.h"

static mode_packet_t last_mode[FAN_MCU_COUNT];
static volatile bool mode_available[FAN_MCU_COUNT];
static volatile unsigned long last_received_time[FAN_MCU_COUNT];  // 마지막 mode_packet 수신 시각 (comms lost 판단용)

// MAC으로 어느 Fan MCU(인덱스)인지 찾기. 못 찾으면 -1
static int find_fan_index(const uint8_t *mac) {
    for (int i = 0; i < FAN_MCU_COUNT; i++) {
        if (memcmp(mac, FAN_MCU_MACS[i], 6) == 0) {
            return i;
        }
    }
    return -1;
}

void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    int idx = find_fan_index(mac);
    if (idx < 0) return;  // 등록되지 않은 MAC → 무시

    if (len == sizeof(mode_packet_t)) {
        memcpy(&last_mode[idx], data, sizeof(mode_packet_t));
        mode_available[idx] = true;
        last_received_time[idx] = millis();  // 통신 두절 판단 기준 시각 갱신
    }
}

void on_send(const uint8_t *mac, esp_now_send_status_t status) {
    Serial.print("[COMMS] Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void comms_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_now_init();
    esp_now_register_recv_cb(on_data_recv);
    esp_now_register_send_cb(on_send);

    unsigned long now = millis();
    for (int i = 0; i < FAN_MCU_COUNT; i++) {
        esp_now_peer_info_t peer;
        memset(&peer, 0, sizeof(peer));
        memcpy(peer.peer_addr, FAN_MCU_MACS[i], 6);
        peer.channel = 1;
        peer.encrypt = false;

        esp_err_t result = esp_now_add_peer(&peer);
        Serial.print("[COMMS] Peer ");
        Serial.print(i);
        Serial.print(" add → ");
        Serial.println(result == ESP_OK ? "OK" : "FAIL");

        last_received_time[i] = now;  // 부팅 시각으로 초기화 (부팅 직후 바로 두절 판정되는 것 방지)
    }
}

void comms_send(uint8_t fan_index, command_packet_t packet) {
    if (fan_index >= FAN_MCU_COUNT) return;
    esp_now_send(FAN_MCU_MACS[fan_index], (uint8_t *)&packet, sizeof(command_packet_t));
}

mode_packet_t comms_get_last_mode(uint8_t fan_index) {
    mode_available[fan_index] = false;
    return last_mode[fan_index];
}

bool comms_mode_available(uint8_t fan_index) {
    if (fan_index >= FAN_MCU_COUNT) return false;
    return mode_available[fan_index];
}

// 마지막 수신 후 COMMS_LOST_TIMEOUT을 초과했는지 확인
bool comms_is_lost(uint8_t fan_index) {
    if (fan_index >= FAN_MCU_COUNT) return true;
    return (millis() - last_received_time[fan_index] > COMMS_LOST_TIMEOUT);
}

// 등록된 Fan MCU 중 하나라도 두절 상태면 true.
// LED가 1개뿐이라 N대 상태를 하나로 합산 — 현재 FAN_MCU_COUNT=1이라 실질적으로 단일 판단과 동일함.
bool comms_any_lost() {
    for (int i = 0; i < FAN_MCU_COUNT; i++) {
        if (comms_is_lost(i)) return true;
    }
    return false;
}