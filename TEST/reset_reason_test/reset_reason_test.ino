#include <Arduino.h>
#include <esp_system.h>

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    esp_reset_reason_t reason = esp_reset_reason();

    Serial.print("Reset reason: ");
    switch (reason) {
        case ESP_RST_UNKNOWN:   Serial.println("UNKNOWN");   break;
        case ESP_RST_POWERON:   Serial.println("POWERON");   break;
        case ESP_RST_EXT:       Serial.println("EXT");       break;
        case ESP_RST_SW:        Serial.println("SW");        break;
        case ESP_RST_PANIC:     Serial.println("PANIC");     break;
        case ESP_RST_INT_WDT:   Serial.println("INT_WDT");   break;
        case ESP_RST_TASK_WDT:  Serial.println("TASK_WDT");  break;
        case ESP_RST_WDT:       Serial.println("WDT");       break;
        case ESP_RST_DEEPSLEEP: Serial.println("DEEPSLEEP"); break;
        case ESP_RST_BROWNOUT:  Serial.println("BROWNOUT");  break;
        case ESP_RST_SDIO:      Serial.println("SDIO");      break;
        default:                Serial.println("OTHER");     break;
    }
}

void loop() {}