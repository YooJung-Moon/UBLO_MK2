#include "logic.h"

// 통신 두절로 인해 CLOSE로 강제 전환된 상태 플래그.
// AUTO(환기 중 두절), OPEN, TURBO 세 경우 모두 이 플래그를 공유해서 사용한다.
// 통신이 복구되면(top-of-function 체크) AUTO로 자동 복귀시키는 데 쓰인다.
static bool comms_lost_close = false;

void logic_init() {
    // 별도 초기화 없음
}

bool logic_is_comms_lost_close() {
    return comms_lost_close;
}

void logic_reset_comms_lost() {
    comms_lost_close = false;
}

mode_packet_t logic_update(uint8_t current_mode, command_packet_t cmd,
                           unsigned long entry_time, unsigned long last_packet_time) {
    mode_packet_t result;
    result.mode      = current_mode;
    result.error     = 0;

    // 통신 두절 CLOSE 상태에서 패킷이 다시 수신되면 AUTO로 복귀
    // last_packet_time이 COMMS_LOST_TIMEOUT 이내로 갱신됐다는 건
    // command_packet을 방금 받았다는 의미
    if (comms_lost_close && (millis() - last_packet_time < COMMS_LOST_TIMEOUT)) {
        comms_lost_close = false;
        result.mode      = MODE_AUTO;
        result.fan_cmd   = cmd.fan_cmd;
        result.cover_cmd = cmd.cover_cmd;
        Serial.println("[LOGIC] Comms restored → AUTO");
        return result;
    }

    switch (current_mode) {
        case MODE_AUTO:
            // AUTO 상태에서 마지막으로 받은 명령이 "환기(fan ON)"인 채로 통신이 두절되면
            // 새 판단이 갱신되지 않아 커버가 무한정 열린 채로 방치될 수 있다.
            // → 두절 시간이 COMMS_LOST_TIMEOUT을 넘으면 안전을 위해 CLOSE로 강제 전환한다.
            // 마지막 명령이 fan_cmd==0(정지 상태)이었다면 이미 안전한 상태이므로 그대로 둔다.
            if (cmd.fan_cmd == 1 && (millis() - last_packet_time > COMMS_LOST_TIMEOUT)) {
                comms_lost_close = true;
                result.mode      = MODE_CLOSE;
                result.fan_cmd   = 0;
                result.cover_cmd = 0;
                Serial.println("[LOGIC] AUTO comms lost (fan was ON) → CLOSE");
            } else {
                result.fan_cmd   = cmd.fan_cmd;
                result.cover_cmd = cmd.cover_cmd;
            }
            break;

        case MODE_CLOSE:
            result.fan_cmd   = 0;  // OFF
            result.cover_cmd = 0;  // CLOSE
            break;

        case MODE_OPEN:
            result.fan_cmd   = 0;  // OFF
            result.cover_cmd = 1;  // OPEN
            if (TIMEOUT_OPEN > 0 && millis() - entry_time >= TIMEOUT_OPEN) {
                if (millis() - last_packet_time > COMMS_LOST_TIMEOUT) {
                    // 통신 두절: CLOSE 전환
                    comms_lost_close = true;
                    result.mode      = MODE_CLOSE;
                    result.fan_cmd   = 0;
                    result.cover_cmd = 0;
                    Serial.println("[LOGIC] OPEN timeout + comms lost → CLOSE");
                } else {
                    // 통신 정상: AUTO 복귀
                    result.mode = MODE_AUTO;
                    Serial.println("[LOGIC] OPEN timeout → AUTO");
                }
            }
            break;

        case MODE_TURBO:
            result.fan_cmd   = 1;  // ON
            result.cover_cmd = 1;  // OPEN
            if (TIMEOUT_TURBO > 0 && millis() - entry_time >= TIMEOUT_TURBO) {
                if (millis() - last_packet_time > COMMS_LOST_TIMEOUT) {
                    // 통신 두절: CLOSE 전환
                    comms_lost_close = true;
                    result.mode      = MODE_CLOSE;
                    result.fan_cmd   = 0;
                    result.cover_cmd = 0;
                    Serial.println("[LOGIC] TURBO timeout + comms lost → CLOSE");
                } else {
                    // 통신 정상: AUTO 복귀
                    result.mode = MODE_AUTO;
                    Serial.println("[LOGIC] TURBO timeout → AUTO");
                }
            }
            break;

        default:
            result.fan_cmd   = 0;
            result.cover_cmd = 0;
            break;
    }

    return result;
}