#include "comms.h"
#include "config.h"

static user_input_packet_t last_input;
static bool input_available = false;

void on_data_recv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len == sizeof(user_input_packet_t)) {
        memcpy(&last_input, data, sizeof(user_input_packet_t));
        input_available = true;
    }
}

void comms_init() {
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_recv_cb(on_data_recv);

    esp_now_peer_info_t peer;
    memcpy(peer.peer_addr, FAN_MCU_MAC, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
}

void comms_send(command_packet_t packet) {
    esp_now_send(FAN_MCU_MAC, (uint8_t *)&packet, sizeof(command_packet_t));
}

user_input_packet_t comms_get_last_input() {
    input_available = false;
    return last_input;
}

bool comms_input_available() {
    return input_available;
}