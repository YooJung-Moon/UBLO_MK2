#include "config.h"
#include "actuators.h"
#include "encoder.h"
#include "logic.h"
#include "comms.h"

static uint8_t current_mode = MODE_AUTO;
static unsigned long mode_entry_time = 0;
static unsigned long last_packet_time = 0;  // 마지막 command_packet 수신 시각
static command_packet_t last_cmd = {0, 0}; // 초기값: fan OFF, cover CLOSE
static uint8_t prev_fan_cmd = 255;         // 무효값: 첫 명령 시 무조건 실행
static uint8_t prev_cover_cmd = 255;       // 무효값: 첫 명령 시 무조건 실행

String mode_name(uint8_t mode) {
    switch (mode) {
        case MODE_AUTO:  return "AUTO";
        case MODE_CLOSE: return "CLOSE";
        case MODE_OPEN:  return "OPEN";
        case MODE_TURBO: return "TURBO";
        default:         return "UNKNOWN";
    }
}

// 액추에이터 실행 함수
// OPEN 방향: 커버 먼저 열고 → fan ON
// CLOSE 방향: fan 먼저 끄고 → 커버 닫기
// prev_fan_cmd/prev_cover_cmd와 값이 같으면 실제 하드웨어 명령은 스킵되므로
// 이 함수를 매 loop 무조건 호출해도 안전하다 (모터 재구동/중복 로그 없음).
void actuate(mode_packet_t result) {
    bool ok = true;

    if (result.cover_cmd == 1) {
        if (result.cover_cmd != prev_cover_cmd) {
            Serial.println("[COVER] Opening...");
            ok = cover_set(result.cover_cmd);
            if (ok) {
                prev_cover_cmd = result.cover_cmd;
            } else {
                prev_cover_cmd = 255;          // 무효값: 다음 명령 시 무조건 cover_set() 호출
                result.error = 1;              // cover_open_timeout
                comms_send(result);            // 에러 정보 Sensor MCU로 전송
                encoder_error_blink();         // 타임아웃 에러 → LED 4개 깜빡임
                return;
            }
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
            ok = cover_set(result.cover_cmd);
            if (ok) {
                prev_cover_cmd = result.cover_cmd;
            } else {
                prev_cover_cmd = 255;          // 무효값: 다음 명령 시 무조건 cover_set() 호출
                result.error = 2;              // cover_close_timeout
                comms_send(result);            // 에러 정보 Sensor MCU로 전송
                encoder_error_blink();         // 타임아웃 에러 → LED 4개 깜빡임
                return;
            }
        }
    }
}

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    last_packet_time = millis();  // 부팅 시각으로 초기화

    // 실제 MAC 주소 출력 — config.h의 SENSOR_MCU_MAC과 비교용
    Serial.print("Fan MCU MAC: ");
    Serial.println(WiFi.macAddress());

    // 현재 config.h 모드 표시 — Sensor MCU와 어긋나지 않았는지 확인용
    #if defined(TEST_MODE_FAST)
        Serial.println("[CONFIG] Mode: TEST_MODE_FAST");
    #elif defined(PRODUCTION_MODE)
        Serial.println("[CONFIG] Mode: PRODUCTION_MODE");
    #endif

    actuators_init();
    encoder_init();
    comms_init();

    Serial.println("Fan MCU ready");
    Serial.print("Initial mode: ");
    Serial.println(mode_name(current_mode));
}

void loop() {
    // encoder 회전 및 버튼 상태 항상 감지
    encoder_update();

    // 버튼 눌림 시 mode_confirmed → current_mode 갱신
    if (encoder_changed()) {
        current_mode = encoder_get_mode();
        mode_entry_time = millis();
        logic_reset_comms_lost();  // 사용자 직접 조작 → comms_lost_close 플래그 리셋
        Serial.print("[ENCODER] Mode changed to: ");
        Serial.println(mode_name(current_mode));

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time, last_packet_time);
        actuate(result);
    }

    // Sensor MCU로부터 command_packet 수신 시 처리
    if (comms_command_available()) {
        last_cmd = comms_get_last_command();
        last_packet_time = millis();  // 수신 시각 갱신
        Serial.print("[COMMS] command_packet received — fan: ");
        Serial.print(last_cmd.fan_cmd);
        Serial.print(" | cover: ");
        Serial.println(last_cmd.cover_cmd);

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time, last_packet_time);

        // 모드 전환 감지 (AUTO 복귀, comms_lost CLOSE 전환 포함)
        if (result.mode != current_mode) {
            current_mode = result.mode;
            mode_entry_time = millis();
            encoder_set_mode(current_mode);
            Serial.print("[LOGIC] Mode changed to: ");
            Serial.println(mode_name(current_mode));
        }

        // Sensor MCU 수신 대기 진입 여유 시간
        delay(50);

        // cover_set() blocking 전에 먼저 mode_packet 전송
        // Sensor MCU의 3초 대기 구간 안에 응답하기 위함
        comms_send(result);

        actuate(result);
    }

    // 통신 상태/타임아웃 반영 체크 — command_packet 수신과 무관하게 매 loop 항상 실행.
    // (MANUAL 모드 타임아웃뿐 아니라, AUTO 모드에서 모드 전환 없이 액추에이터만
    //  강제로 닫히는 경우도 이 블록에서 반영된다 — result.mode가 안 바뀌어도 actuate()는 호출해야
    //  실제로 커버/팬이 닫힌다)
    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time, last_packet_time);
    if (result.mode != current_mode) {
        current_mode = result.mode;
        mode_entry_time = millis();
        encoder_set_mode(current_mode);
        Serial.print("[LOGIC] Mode changed to: ");
        Serial.println(mode_name(current_mode));
    }
    actuate(result);  // 모드 변경 여부와 무관하게 항상 호출 (내부에서 값 변경분만 실제 반영됨)

    // AUTO 모드 통신 두절 LED 표시 갱신 — 매 loop마다 항상 실행 (패킷 수신/모드 변경과 무관)
    // 조건: 현재 모드가 AUTO이고, 마지막 수신으로부터 COMMS_LOST_TIMEOUT을 초과했을 때만 깜빡임.
    // 이번 변경으로 AUTO의 comms-lost 강제 닫힘은 모드를 바꾸지 않으므로,
    // 닫힌 상태에서도 current_mode == MODE_AUTO가 유지되어 이 블록이 계속 깜빡임을 보여준다.
    // 통신이 복구되면(last_packet_time 갱신) 조건이 false가 되어 즉시 꺼지고 정상 AUTO로 복귀한다.
    bool comms_lost_now = (current_mode == MODE_AUTO) &&
                          (millis() - last_packet_time > COMMS_LOST_TIMEOUT);
    encoder_set_comms_lost(comms_lost_now);
}