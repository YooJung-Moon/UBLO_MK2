#include "logic.h"

#define COMMS_LOST_TIMEOUT 600000  // 10분 (ms) — 마지막 수신 후 이 시간 초과 시 통신 두절로 판단

static bool comms_lost_close = false;  // 통신 두절로 CLOSE 전환된 상태 플래그

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
            result.fan_cmd   = cmd.fan_cmd;
            result.cover_cmd = cmd.cover_cmd;
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