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
static bool comms_lost_logged[FAN_MCU_COUNT] = {false};  // Fan MCU별 comms_lost 최초 1회 로깅 추적

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    // 실제 MAC 주소 출력 — config.h의 FAN_MCU_MACS와 비교용
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
    Serial.print("Paired Fan MCU count: ");
    Serial.println(FAN_MCU_COUNT);
    Serial.println("timestamp, mac, co2, temp, humidity, mode, fan_cmd, cover_cmd, error");
}

void loop() {
    unsigned long now = millis();

    // 통신 두절 LED 표시 갱신 — 매 loop마다 항상 실행 (센서 read 성공/실패, 10초 주기와 무관).
    // 아래 sensor read 실패 시 return하는 경로가 있어서, 반드시 그보다 앞쪽(loop 최상단)에 둬야
    // 매 iteration마다 빠짐없이 깜빡임 타이머가 갱신된다.
    led_set_comms_lost(comms_any_lost());
    led_update();

    // 10초 주기: sensor 읽기 + raw logging + 버퍼 추가
    if (now - last_sensor_time >= SENSOR_INTERVAL) {
        last_sensor_time = now;

        uint16_t co2;
        float temp, humidity;
        if (!sensors_read(co2, temp, humidity)) {
            Serial.println("Sensor read failed, skipping");
            led_set_error_sensor();
            return;
        }

        String ts = rtc_timestamp();
        sdcard_log_raw(ts, co2, temp, humidity);  // 모든 Fan MCU 폴더에 동일하게 기록
        buffer_add(co2);
        led_set_co2(co2);
        sample_count++;

        // 판단 주기: DECISION_COUNT개 측정값 누적 시 평균으로 판단
        if (sample_count >= DECISION_COUNT) {
            sample_count = 0;

            uint16_t avg_co2 = buffer_average();
            Serial.print("[LOGIC] avg CO2: ");
            Serial.println(avg_co2);

            command_packet_t cmd = logic_decide(avg_co2);

            // 1. 모든 Fan MCU에 command_packet 연속 전송 (non-blocking, 사실상 동시 전송)
            for (uint8_t i = 0; i < FAN_MCU_COUNT; i++) {
                comms_send(i, cmd);
            }

            // 2. mode_packet 수신 대기 (최대 3초, N대 모두 동일 윈도우 안에서 수집)
            unsigned long wait_start = millis();
            while (millis() - wait_start < 3000) {
                bool all_received = true;
                for (uint8_t i = 0; i < FAN_MCU_COUNT; i++) {
                    if (!comms_mode_available(i)) {
                        all_received = false;
                        break;
                    }
                }
                if (all_received) break;  // 전부 응답 왔으면 3초 다 안 기다리고 진행
            }

            // 3. Fan MCU별로 응답 온 것만 로깅, 안 온 것은 최초 1회만 comms_lost 로깅
            for (uint8_t i = 0; i < FAN_MCU_COUNT; i++) {
                if (comms_mode_available(i)) {
                    mode_packet_t mp = comms_get_last_mode(i);
                    if (mp.error > 0) {
                        sdcard_log_error(ts, i, mp.error);
                    } else {
                        sdcard_log_decision(ts, i, mp.mode, mp.fan_cmd, mp.cover_cmd);
                    }
                    comms_lost_logged[i] = false;  // 정상 수신 시 리셋
                } else {
                    // mode_packet 미수신 → 최초 1회만 comms_lost 로깅
                    Serial.print("[COMMS] mode_packet timeout — fan index ");
                    Serial.println(i);
                    if (!comms_lost_logged[i]) {
                        sdcard_log_comms_lost(ts, i);
                        comms_lost_logged[i] = true;
                    }
                }
            }
        }
    }
}