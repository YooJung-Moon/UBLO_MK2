#include "sdcard.h"
#include "rtc.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN D10

// MAC 6바이트 → "206EF1312D84" 형식 12자 문자열
static String mac_to_string(const uint8_t *mac) {
    char buf[13];
    snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

static String get_folder(uint8_t fan_index) {
    return "/" + mac_to_string(FAN_MCU_MACS[fan_index]);
}

static String get_filename(uint8_t fan_index) {
    String ts = rtc_timestamp();
    String date = ts.substring(0, 10);
    date.replace("-", "");
    return get_folder(fan_index) + "/" + date + ".csv";
}

static void check_and_create_file(uint8_t fan_index, String filename) {
    String folder = get_folder(fan_index);
    if (!SD.exists(folder)) {
        if (SD.mkdir(folder)) {
            Serial.println("[SD] folder created: " + folder);
        } else {
            Serial.println("[SD] folder creation failed: " + folder);
        }
    }

    if (!SD.exists(filename)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.println("timestamp,mac,co2,temp,humidity,mode,fan_cmd,cover_cmd,error");
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

// raw 센서 데이터는 Fan MCU와 무관하게 측정된 값이므로
// 모든 Fan MCU 폴더에 동일하게 기록한다 (어느 폴더를 열어도 센서 데이터를 볼 수 있도록)
void sdcard_log_raw(String timestamp, uint16_t co2, float temp, float humidity) {
    for (uint8_t i = 0; i < FAN_MCU_COUNT; i++) {
        String filename = get_filename(i);
        check_and_create_file(i, filename);
        String mac_str = mac_to_string(FAN_MCU_MACS[i]);

        File f = SD.open(filename, FILE_APPEND);
        if (f) {
            f.print(timestamp); f.print(",");
            f.print(mac_str);   f.print(",");
            f.print(co2);       f.print(",");
            f.print(temp);      f.print(",");
            f.print(humidity);  f.print(",");
            f.println(",,,");   // mode, fan_cmd, cover_cmd, error 비움
            f.close();
        } else {
            Serial.println("[SD] raw log failed: " + filename);
        }
    }
    Serial.print("[SD] raw: ");
    Serial.print(timestamp); Serial.print(", ");
    Serial.print(co2);       Serial.print(", ");
    Serial.print(temp);      Serial.print(", ");
    Serial.println(humidity);
}

void sdcard_log_decision(String timestamp, uint8_t fan_index, uint8_t mode, uint8_t fan_cmd, uint8_t cover_cmd) {
    String filename = get_filename(fan_index);
    check_and_create_file(fan_index, filename);
    String mac_str = mac_to_string(FAN_MCU_MACS[fan_index]);

    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(mac_str);   f.print(",");
        f.print(",,,");     // co2, temp, humidity 비움
        f.print(mode);      f.print(",");
        f.print(fan_cmd);   f.print(",");
        f.print(cover_cmd); f.print(",");
        f.println();        // error 비움
        f.close();
        Serial.print("[SD] decision: mac=");
        Serial.print(mac_str); Serial.print(", mode=");
        Serial.print(mode);    Serial.print(", fan=");
        Serial.print(fan_cmd); Serial.print(", cover=");
        Serial.println(cover_cmd);
    } else {
        Serial.println("[SD] decision log failed: " + filename);
    }
}

void sdcard_log_error(String timestamp, uint8_t fan_index, uint8_t error) {
    String filename = get_filename(fan_index);
    check_and_create_file(fan_index, filename);
    String mac_str = mac_to_string(FAN_MCU_MACS[fan_index]);

    String error_str = "";
    if (error == 1)      error_str = "cover_open_timeout";
    else if (error == 2) error_str = "cover_close_timeout";

    File f = SD.open(filename, FILE_APPEND);
    if (f) {
        f.print(timestamp); f.print(",");
        f.print(mac_str);   f.print(",");
        f.print(",,,,,,");  // co2, temp, humidity, mode, fan_cmd, cover_cmd 비움
        f.println(error_str);
        f.close();
        Serial.print("[SD] error: mac=");
        Serial.print(mac_str); Serial.print(", ");
        Serial.println(error_str);
    } else {
        Serial.println("[SD] error log failed: " + filename);
    }
}