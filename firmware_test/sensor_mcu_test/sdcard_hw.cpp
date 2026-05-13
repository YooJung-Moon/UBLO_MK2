#include "sdcard.h"
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

static String filename;

void sdcard_init() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[SD] init failed");
        return;
    }

    filename = "/data.csv";

    if (!SD.exists(filename)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.println("timestamp,co2,temp,humidity,mode,fan_cmd,cover_cmd");
            f.close();
            Serial.println("[SD] file created");
        } else {
            Serial.println("[SD] file creation failed");
        }
    } else {
        Serial.println("[SD] file exists, appending");
    }
    Serial.println("[SD] initialized");
}

void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity) {
    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(co2);       f.print(",");
        f.print(temp);      f.print(",");
        f.print(humidity);  f.print(",");
        f.println(",,");
        f.close();
        Serial.print("[SD] raw: ");
        Serial.print(timestamp); Serial.print(", ");
        Serial.print(co2);       Serial.print(", ");
        Serial.print(temp);      Serial.print(", ");
        Serial.println(humidity);
    } else {
        Serial.println("[SD] raw log failed");
    }
}

void sdcard_log_decision(String timestamp, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd) {
    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(",,,");
        f.print(mode);      f.print(",");
        f.print(fan_cmd);   f.print(",");
        f.println(cover_cmd);
        f.close();
        Serial.print("[SD] decision: mode=");
        Serial.print(mode);   Serial.print(", fan=");
        Serial.print(fan_cmd); Serial.print(", cover=");
        Serial.println(cover_cmd);
    } else {
        Serial.println("[SD] decision log failed");
    }
}