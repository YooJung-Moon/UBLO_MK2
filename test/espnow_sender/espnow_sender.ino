#include <esp_now.h>
#include <WiFi.h>

// Receiver (보드 2) MAC 주소
uint8_t receiverMAC[] = {0xE0, 0x72, 0xA1, 0xF8, 0xF6, 0x0C};

typedef struct {
  uint8_t command;
  uint8_t gate;
  uint8_t battery_pct;
} sensor_to_fan_t;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sender ready");
}

void loop() {
  sensor_to_fan_t packet;
  packet.command     = 1;   // LOW
  packet.gate        = 1;   // OPEN
  packet.battery_pct = 75;

  esp_now_send(receiverMAC, (uint8_t *)&packet, sizeof(packet));
  delay(3000);
}