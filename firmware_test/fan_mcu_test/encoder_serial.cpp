#include "encoder.h"

static uint8_t current_mode = 2;
static bool changed = false;

void encoder_init() {
    // Serial 입력으로 대체, 별도 초기화 없음
    Serial.println("Mode input: 0=AUTO, 1=CLOSED, 2=BREEZE, 3=TURBO");
}

bool encoder_changed() {
    if (Serial.available()) {
        char input = Serial.read();
        if (input >= '0' && input <= '3') {
            uint8_t new_mode = input - '0';
            if (new_mode != current_mode) {
                current_mode = new_mode;
                changed = true;
            }
        }
    }

    if (changed) {
        changed = false;
        return true;
    }
    return false;
}

uint8_t encoder_get_mode() {
    return current_mode;
}