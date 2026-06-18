#include "comms.h"
#include "config.h"

static command_packet_t last_command;
static volatile bool command_available = false;
static unsigned long last_cmd_time = 0;  // 마지막 command_packet 수신 시각

void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    if (memcmp(mac, SENSOR_MCU_MAC, 6) != 0) return;
    if (len == sizeof(command_packet_t)) {
        memcpy(&last_command, data, sizeof(command_packet_t));
        command_available = true;
        last_cmd_time = millis();  // 수신 시각 갱신
        Serial.println("Packet received");
    }
}

void comms_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_now_init();
    esp_now_register_recv_cb(on_data_recv);

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(peer));
    memcpy(peer.peer_addr, SENSOR_MCU_MAC, 6);
    peer.channel = 1;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
}

void comms_send(mode_packet_t packet) {
    esp_now_send(SENSOR_MCU_MAC, (uint8_t *)&packet, sizeof(mode_packet_t));
}

command_packet_t comms_get_last_command() {
    command_available = false;
    return last_command;
}

bool comms_command_available() {
    return command_available;
}

// 마지막 수신으로부터 경과 시간 반환
// 한 번도 수신 못 한 경우 ULONG_MAX 반환 → 항상 통신 두절로 판단
unsigned long comms_last_cmd_age() {
    if (last_cmd_time == 0) return ULONG_MAX;
    return millis() - last_cmd_time;
}