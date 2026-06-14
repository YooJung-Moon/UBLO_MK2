#include "logic.h"

void logic_init() {
    // 별도 초기화 없음
}

mode_packet_t logic_update(uint8_t current_mode, command_packet_t cmd, unsigned long entry_time) {
    mode_packet_t result;
    result.mode = current_mode;

    switch (current_mode) {
        case MODE_AUTO:
            // Sensor MCU 판단 결과 그대로 실행
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
            // 4시간 타임아웃 → AUTO 복귀
            if (TIMEOUT_OPEN > 0 && millis() - entry_time >= TIMEOUT_OPEN) {
                result.mode = MODE_AUTO;
                Serial.println("[LOGIC] OPEN timeout → AUTO");
            }
            break;

        case MODE_TURBO:
            result.fan_cmd   = 1;  // ON
            result.cover_cmd = 1;  // OPEN
            // 1시간 타임아웃 → AUTO 복귀
            if (TIMEOUT_TURBO > 0 && millis() - entry_time >= TIMEOUT_TURBO) {
                result.mode = MODE_AUTO;
                Serial.println("[LOGIC] TURBO timeout → AUTO");
            }
            break;

        default:
            result.fan_cmd   = 0;
            result.cover_cmd = 0;
            break;
    }

    return result;
}