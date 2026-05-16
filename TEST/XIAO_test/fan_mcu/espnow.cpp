#include "espnow.h"

volatile bool            newPacketReceived = false;
volatile sensor_to_fan_t latestPacket      = {0, 0, 0};

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  if (len == sizeof(sensor_to_fan_t)) {
    memcpy((void *)&latestPacket, data, sizeof(sensor_to_fan_t));
    newPacketReceived = true;
  }
}

bool espnowInit() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return false;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW initialized");
  return true;
}

void espnowResetPacket() {
  newPacketReceived = false;
}