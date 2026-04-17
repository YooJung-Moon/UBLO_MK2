#include "espnow.h"

static uint8_t receiverMAC[] = RECEIVER_MAC;
static volatile bool ackReceived = false;
static volatile bool ackSuccess  = false;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  ackReceived = true;
  ackSuccess  = (status == ESP_NOW_SEND_SUCCESS);
  if (ackSuccess) {
    Serial.println("ESP-NOW: ACK received");
  } else {
    Serial.println("ESP-NOW: ACK failed");
  }
}

bool espnowInit() {
  // AP 연결 없이 스테이션 모드만 설정
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return false;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW add peer failed");
    return false;
  }

  Serial.println("ESP-NOW initialized");
  return true;
}

bool espnowSend(FanCommand cmd, GateCommand gate, uint8_t battery_pct) {
  sensor_to_fan_t packet;
  packet.command     = (uint8_t)cmd;
  packet.gate        = (uint8_t)gate;
  packet.battery_pct = battery_pct;

  ackReceived = false;
  ackSuccess  = false;

  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&packet, sizeof(packet));
  if (result != ESP_OK) {
    Serial.println("ESP-NOW send failed");
    return false;
  }

  // ACK 대기 (최대 100ms)
  uint32_t start = millis();
  while (!ackReceived && millis() - start < 100) {
    delay(1);
  }

  return ackSuccess;
}

bool espnowWasAckReceived() {
  return ackReceived;
}

void espnowResetAck() {
  ackReceived = false;
  ackSuccess  = false;
}