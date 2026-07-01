#include "comms.h"
#include "config.h"

static mode_packet_t last_mode[FAN_MCU_COUNT];
static volatile bool mode_available[FAN_MCU_COUNT];

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