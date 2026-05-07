#include "config.h"
#include "actuators.h"
#include "encoder.h"
#include "led.h"
#include "logic.h"
#include "comms.h"

static uint8_t current_mode = MODE_AUTO;
static unsigned long mode_entry_time = 0;
static command_packet_t last_cmd = {0, 0};
static uint8_t prev_fan_cmd = 255;
static uint8_t prev_cover_cmd = 255;

void setup() {
    delay(3000);
    Serial.begin(115200);
    while (!Serial);

    actuators_init();
    encoder_init();
    led_init();
    comms_init();

    led_set(current_mode);
    Serial.println("Fan MCU ready");
}

void loop() {
    // 다이얼 입력 감지 (Serial 입력으로 대체)
    if (encoder_changed()) {
        current_mode = encoder_get_mode();
        mode_entry_time = millis();
        Serial.print("[ENCODER] Mode changed to: ");
        Serial.println(current_mode);
        led_set(current_mode);
    }

    // command_packet 수신 (AUTO 모드)
    if (comms_command_available()) {
        last_cmd = comms_get_last_command();
        Serial.print("[COMMS] command_packet received — fan: ");
        Serial.print(last_cmd.fan_cmd);
        Serial.print(" | cover: ");
        Serial.println(last_cmd.cover_cmd);
    }

    // 판단
    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);

    // AUTO 복귀 감지
    if (result.mode != current_mode) {
        current_mode = result.mode;
        mode_entry_time = millis();
        led_set(current_mode);
        Serial.print("[LOGIC] Auto revert to mode: ");
        Serial.println(current_mode);
    }

    // 커버 먼저 실행
    if (result.cover_cmd != prev_cover_cmd) {
        cover_set(result.cover_cmd);
        prev_cover_cmd = result.cover_cmd;
    }

    // 팬은 커버 상태 확인 후 실행
    if (result.fan_cmd != prev_fan_cmd) {
        if (result.fan_cmd == 1 && prev_cover_cmd != 1) {
            // 커버가 아직 안 열렸으면 팬 보류
            Serial.println("[FAN] Waiting for cover to open...");
        } else {
            fan_set(result.fan_cmd);
            prev_fan_cmd = result.fan_cmd;
        }
    }

    // mode_packet 전송
    comms_send(result);

    delay(100);
}