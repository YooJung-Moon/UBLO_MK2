#include "config.h"
#include "actuators.h"
#include "encoder.h"
#include "logic.h"
#include "comms.h"

static uint8_t current_mode = MODE_OPEN;
static unsigned long mode_entry_time = 0;
static command_packet_t last_cmd = {0, 1}; // 초기값: fan OFF, cover OPEN
static uint8_t prev_fan_cmd = 0;
static uint8_t prev_cover_cmd = 1;         // 초기값: 부팅 시 커버 OPEN 상태로 가정

// ── 테스트 모드 ──────────────────────────────────────────
#if TEST_MODE
static const uint8_t TEST_SEQUENCE[] = {
    MODE_OPEN,  MODE_TURBO, MODE_CLOSE,  // A
    MODE_TURBO, MODE_CLOSE, MODE_OPEN,   // B
    MODE_CLOSE, MODE_TURBO, MODE_OPEN,   // C
    MODE_TURBO, MODE_OPEN,  MODE_CLOSE,  // D
    MODE_OPEN,  MODE_CLOSE, MODE_TURBO,  // E
    MODE_CLOSE, MODE_OPEN,  MODE_TURBO,  // F
};
static const int TEST_SEQUENCE_LEN = sizeof(TEST_SEQUENCE);  // 18
static const int TEST_TOTAL_STEPS  = 60;  // 20세트 × 3모드 = 1시간

static int  test_step = 0;
static unsigned long test_step_start = 0;
static bool test_done = false;
#endif
// ─────────────────────────────────────────────────────────

String mode_name(uint8_t mode) {
    switch (mode) {
        case MODE_AUTO:  return "AUTO";
        case MODE_CLOSE: return "CLOSE";
        case MODE_OPEN:  return "OPEN";
        case MODE_TURBO: return "TURBO";
        default:         return "UNKNOWN";
    }
}

void actuate(mode_packet_t result) {
    bool ok = true;

    if (result.cover_cmd == 1) {
        if (result.cover_cmd != prev_cover_cmd) {
            Serial.println("[COVER] Opening...");
            ok = cover_set(result.cover_cmd);
            if (ok) {
                prev_cover_cmd = result.cover_cmd;
            } else {
                prev_cover_cmd = 255;
                result.error = 1;
                comms_send(result);
                encoder_error_blink();
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
                prev_cover_cmd = 255;
                result.error = 2;
                comms_send(result);
                encoder_error_blink();
                return;
            }
        }
    }
}

// ── 테스트 스텝 전환 함수 ─────────────────────────────────
#if TEST_MODE
void test_apply_step(int step) {
    uint8_t mode    = TEST_SEQUENCE[step % TEST_SEQUENCE_LEN];
    int set_num     = step / 3 + 1;
    int step_in_set = step % 3 + 1;
    char set_label  = 'A' + (step % TEST_SEQUENCE_LEN) / 3;

    current_mode = mode;
    mode_entry_time = millis();
    encoder_set_mode(current_mode);

    Serial.print("[TEST] Set ");
    Serial.print(set_num);
    Serial.print("/20 - Step ");
    Serial.print(step_in_set);
    Serial.print("/3 (");
    Serial.print(set_label);
    Serial.print(") → ");
    Serial.println(mode_name(current_mode));

    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
    comms_send(result);
    actuate(result);
}

void test_finish() {
    // OPEN 상태로 복귀
    current_mode = MODE_OPEN;
    mode_entry_time = millis();
    encoder_set_mode(current_mode);

    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
    comms_send(result);
    actuate(result);

    Serial.println("[TEST] Test complete → OPEN");
    Serial.println("[TEST] Resuming normal operation");
    test_done = true;
}
#endif
// ─────────────────────────────────────────────────────────

void setup() {
    delay(3000);
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000);

    Serial.print("Fan MCU MAC: ");
    Serial.println(WiFi.macAddress());

    actuators_init();
    encoder_init();
    comms_init();

    Serial.println("Fan MCU ready");

#if TEST_MODE
    Serial.println("[TEST] Test mode enabled");
    test_step = 0;
    test_done = false;
    test_step_start = millis();
    test_apply_step(test_step);
#else
    Serial.print("Initial mode: ");
    Serial.println(mode_name(current_mode));
#endif
}

void loop() {
#if TEST_MODE
    if (!test_done) {
        if (millis() - test_step_start >= TEST_STEP_MS) {
            test_step++;
            test_step_start = millis();
            if (test_step < TEST_TOTAL_STEPS) {
                test_apply_step(test_step);
            } else {
                test_finish();
            }
        }
        return;  // 테스트 진행 중엔 encoder/comms 무시
    }
    // test_done = true → 아래 일반 동작으로 fall-through
#endif

    encoder_update();

    if (encoder_changed()) {
        current_mode = encoder_get_mode();
        mode_entry_time = millis();
        Serial.print("[ENCODER] Mode changed to: ");
        Serial.println(mode_name(current_mode));

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
        actuate(result);
    }

    if (comms_command_available()) {
        last_cmd = comms_get_last_command();
        Serial.print("[COMMS] command_packet received — fan: ");
        Serial.print(last_cmd.fan_cmd);
        Serial.print(" | cover: ");
        Serial.println(last_cmd.cover_cmd);

        mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);

        if (result.mode != current_mode) {
            current_mode = result.mode;
            mode_entry_time = millis();
            encoder_set_mode(current_mode);
            Serial.print("[LOGIC] Auto revert to mode: ");
            Serial.println(mode_name(current_mode));
        }

        delay(50);
        comms_send(result);
        actuate(result);
    }

    mode_packet_t result = logic_update(current_mode, last_cmd, mode_entry_time);
    if (result.mode != current_mode) {
        current_mode = result.mode;
        mode_entry_time = millis();
        encoder_set_mode(current_mode);
        Serial.print("[LOGIC] Auto revert to mode: ");
        Serial.println(mode_name(current_mode));
    }
}