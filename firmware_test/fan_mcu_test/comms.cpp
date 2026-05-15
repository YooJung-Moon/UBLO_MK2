#include "comms.h"
#include "config.h"

static command_packet_t last_command;
static volatile bool command_available = false;

void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len == sizeof(command_packet_t)) {
        memcpy(&last_command, data, sizeof(command_packet_t));
        command_available = true;
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