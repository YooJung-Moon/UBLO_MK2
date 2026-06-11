#include "comms.h"
#include "config.h"

static mode_packet_t last_mode;
static volatile bool mode_available = false;

void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    if (memcmp(mac, FAN_MCU_MAC, 6) != 0) return;  // 
    if (len == sizeof(mode_packet_t)) {
        memcpy(&last_mode, data, sizeof(mode_packet_t));
        mode_available = true;
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

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(peer));
    memcpy(peer.peer_addr, FAN_MCU_MAC, 6);
    peer.channel = 1;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

}

void comms_send(command_packet_t packet) {
    esp_now_send(FAN_MCU_MAC, (uint8_t *)&packet, sizeof(command_packet_t));
}

mode_packet_t comms_get_last_mode() {
    mode_available = false;
    return last_mode;
}

bool comms_mode_available() {
    return mode_available;
}