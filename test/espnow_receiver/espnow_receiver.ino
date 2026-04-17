#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  uint8_t command;
  uint8_t gate;
  uint8_t battery_pct;
} sensor_to_fan_t;

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  sensor_to_fan_t packet;
  memcpy(&packet, data, sizeof(packet));

  Serial.println("--- Packet Received ---");
  Serial.printf("command    : %d\n", packet.command);
  Serial.printf("gate       : %d\n", packet.gate);
  Serial.printf("battery_pct: %d%%\n", packet.battery_pct);
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Receiver ready");
}

void loop() {}