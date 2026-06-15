#include "config.h"
#include "sensors.h"
#include "buffer.h"
#include "rtc.h"
#include "logic.h"
#include "sdcard.h"
#include "comms.h"
#include "led.h"

static unsigned long last_sensor_time = 0;
static int sample_count = 0;

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    // 실제 MAC 주소 출력 — config.h의 FAN_MCU_MAC과 비교용
    Serial.print("Sensor MCU MAC: ");
    Serial.println(WiFi.macAddress());

    rtc_init();
    sdcard_init();
    comms_init();
    led_init();

    if (!sensors_init()) {
        Serial.println("Sensor init failed, halting");
        while (true);
    }

    buffer_init();
    Serial.println("Sensor MCU ready");
    Serial.println("timestamp, co2, temp, humidity, mode, fan_cmd, cover_cmd");
}

void loop() {
    unsigned long now = millis();

    // 10초 주기: sensor 읽기 + raw logging + 버퍼 추가
    if (now - last_sensor_time >= SENSOR_INTERVAL) {
        last_sensor_time = now;

        uint16_t co2;
        float temp, humidity;
        if (!sensors_read(co2, temp, humidity)) {
            Serial.println("Sensor read failed, skipping");
            return;
        }

        String ts = rtc_timestamp();
        sdcard_log_raw(ts, co2, temp, humidity);
        buffer_add(co2);
        led_set_co2(co2);  // CO₂ 농도에 따라 LED 색상 업데이트
        sample_count++;

        // 버퍼 가득 찼을 때 판단
        if (sample_count >= BUFFER_SIZE) {
            sample_count = 0;

            uint16_t avg_co2 = buffer_average();
            Serial.print("[LOGIC] avg CO2: ");
            Serial.println(avg_co2);

            command_packet_t cmd = logic_decide(avg_co2);
            comms_send(cmd);

            // mode_packet 수신 대기 (최대 3초)
            unsigned long wait_start = millis();
            while (!comms_mode_available() && millis() - wait_start < 3000);

            if (comms_mode_available()) {
                mode_packet_t mp = comms_get_last_mode();
                sdcard_log_decision(ts, mp.mode, mp.fan_cmd, mp.cover_cmd);
            } else {
                Serial.println("[COMMS] mode_packet timeout");
            }
        }
    }
}