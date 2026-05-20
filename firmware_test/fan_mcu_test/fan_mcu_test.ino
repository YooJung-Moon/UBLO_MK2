#include "config.h"
#include "actuators.h"
#include "encoder.h"
#include "logic.h"
#include "comms.h"

static uint8_t current_mode = MODE_BREEZE;  // 초기 모드: BREEZE
static unsigned long mode_entry_time = 0;   // 부팅 시점부터 타임아웃 카운트 시작
static command_packet_t last_cmd = {0, 1};  // 초기값: fan OFF, cover OPEN
static uint8_t prev_fan_cmd = 0;            // BREEZE: fan OFF
static uint8_t prev_cover_cmd = 1;          // BREEZE: cover OPEN 상태

String mode_name(uint8_t mode) {
    switch (mode) {
        case MODE_AUTO:   return "AUTO";
        case MODE_CLOSED: return "CLOSED";
        case MODE_BREEZE: return "BREEZE";
        case MODE_TURBO:  return "TURBO";
        default:          return "UNKNOWN";
    }
}

// 액추에이터 실행 함수 (커버 방향에 따라 순서 결정)
void actuate(mode_packet_t result) {
    // OPEN 방향: 커버 먼저 열고 → fan ON
    // CLOSE 방향: fan 먼저 끄고 → 커버 닫기
    if (result.cover_cmd == 1) {
        if (result.cover_cmd != prev_cover_cmd) {
            Serial.println("[COVER] Opening...");
            cover_set(result.cover_cmd);
            prev_cover_cmd = result.cover_cmd;
        }
        if (result.fan_cmd != prev_fan_cmd) {
            fan_set(result.fan_cmd);
            prev_fan_cmd = result.fan_cmd;
        }
    } else {
        if (result.fan_cmd != prev_fan_cmd) {
            fan_set(result.fan_cmd);
            prev_fan_cmd = result.fan_cmd;
        }
        if (result.cover_cmd != prev_cover_cmd) {
            Serial.println("[COVER] Closing...");
            cover_set(result.cover_cmd);
            prev_cover_cmd = result.cover_cmd;
        }
    }
}

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    actuators_init();
    encoder_init();
    comms_init();

    Serial.println("Fan MCU ready");
    Serial.print("Initial mode: ");
    Serial.println(mode_name(current_mode));
}

void loop() {
    // 다이얼 입력 감지
    // 버튼 눌림 시 mode_confirmed → current_mode 갱신
    if (encoder_changed()) {
        current_mode = encoder_get_mode();
        mode_entry_time = millis();
        Serial.print("[ENCODER] Mode changed to: ");
        Serial.println(mode_name(current_mode));

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
        actuate(result);
    }

    // Sensor MCU로부터 command_packet 수신 시 처리
    if (comms_command_available()) {
        last_cmd = comms_get_last_command();
        Serial.print("[COMMS] command_packet received — fan: ");
        Serial.print(last_cmd.fan_cmd);
        Serial.print(" | cover: ");
        Serial.println(last_cmd.cover_cmd);

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);

        // AUTO 복귀 감지 (MANUAL 타임아웃)
        if (result.mode != current_mode) {
            current_mode = result.mode;
            mode_entry_time = millis();
            encoder_set_mode(current_mode);  // LED도 AUTO로 업데이트
            Serial.print("[LOGIC] Auto revert to mode: ");
            Serial.println(mode_name(current_mode));
        }

        // cover_set() blocking 전에 먼저 mode_packet 전송
        // Sensor MCU의 3초 대기 구간 안에 응답하기 위함
        comms_send(result);

        actuate(result);
    }

    // MANUAL 모드 타임아웃 체크
    // command_packet 수신과 무관하게 항상 실행
    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
    if (result.mode != current_mode) {
        current_mode = result.mode;
        mode_entry_time = millis();
        encoder_set_mode(current_mode);  // LED도 AUTO로 업데이트
        Serial.print("[LOGIC] Auto revert to mode: ");
        Serial.println(mode_name(current_mode));
    }

    delay(100);
}