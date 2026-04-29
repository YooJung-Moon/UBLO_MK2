#include "comms.h"
#include "config.h"

void on_send(const uint8_t *mac, esp_now_send_status_t status) {
    Serial.print("Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void comms_init() {
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_send_cb(on_send);

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(peer));
    memcpy(peer.peer_addr, FAN_MCU_MAC, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
}

void comms_send(command_packet_t packet) {
    esp_now_send(FAN_MCU_MAC, (uint8_t *)&packet, sizeof(command_packet_t));
}