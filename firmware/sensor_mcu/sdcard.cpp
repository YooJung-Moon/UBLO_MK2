#include "sdcard.h"
#include "rtc.h"
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

static String get_filename() {
    String ts = rtc_timestamp();
    String date = ts.substring(0, 10);
    date.replace("-", "");
    return "/" + date + ".csv";
}

static void check_and_create_file(String filename) {
    if (!SD.exists(filename)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.println("timestamp,co2,temp,humidity,mode,fan_cmd,cover_cmd,error");
            f.close();
            Serial.println("[SD] new file created: " + filename);
        } else {
            Serial.println("[SD] file creation failed: " + filename);
        }
    }
}

void sdcard_init() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[SD] init failed");
        return;
    }
    Serial.println("[SD] initialized");
}

void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity) {
    String filename = get_filename();
    check_and_create_file(filename);

    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(co2);       f.print(",");
        f.print(temp);      f.print(",");
        f.print(humidity);  f.print(",");
        f.println(",,,");   // mode, fan_cmd, cover_cmd, error 비움
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
    String filename = get_filename();
    check_and_create_file(filename);

    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(",,,");     // co2, temp, humidity 비움
        f.print(mode);      f.print(",");
        f.print(fan_cmd);   f.print(",");
        f.print(cover_cmd); f.print(",");
        f.println();        // error 비움
        f.close();
        Serial.print("[SD] decision: mode=");
        Serial.print(mode);    Serial.print(", fan=");
        Serial.print(fan_cmd); Serial.print(", cover=");
        Serial.println(cover_cmd);
    } else {
        Serial.println("[SD] decision log failed");
    }
}

void sdcard_log_error(String timestamp, uint8_t error) {
    String filename = get_filename();
    check_and_create_file(filename);

    String error_str = "";
    if (error == 1)      error_str = "cover_open_timeout";
    else if (error == 2) error_str = "cover_close_timeout";

    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(",,,,,,");  // co2, temp, humidity, mode, fan_cmd, cover_cmd 비움
        f.println(error_str);
        f.close();
        Serial.print("[SD] error: ");
        Serial.println(error_str);
    } else {
        Serial.println("[SD] error log failed");
    }
}