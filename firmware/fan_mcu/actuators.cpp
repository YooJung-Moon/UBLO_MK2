#include "actuators.h"
#include "config.h"

#define COVER_TIMEOUT_MS 10000  // 리밋스위치 미감지 시 타임아웃 (10초)

void actuators_init() {
    pinMode(MOTOR_IN1,  OUTPUT);
    pinMode(MOTOR_IN2,  OUTPUT);
    pinMode(LIM_OPEN,   INPUT_PULLUP);
    pinMode(LIM_CLOSE,  INPUT_PULLUP);
    pinMode(FAN_PWM,    OUTPUT);
    pinMode(BLOWER_EN,  OUTPUT);

    // 초기 상태: fan OFF, 모터 정지
    digitalWrite(FAN_PWM, HIGH);   // active low: HIGH = fan OFF
    digitalWrite(BLOWER_EN, LOW);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);

    Serial.println("Actuators initialized");
}

void fan_set(uint8_t cmd) {
    if (cmd == 1) {
        digitalWrite(BLOWER_EN, HIGH);
        digitalWrite(FAN_PWM, LOW);   // active low: LOW = fan ON
        Serial.println("Fan ON");
    } else {
        digitalWrite(FAN_PWM, HIGH);  // active low: HIGH = fan OFF
        digitalWrite(BLOWER_EN, LOW);
        Serial.println("Fan OFF");
    }
}

// 커버 모터 고장 — 커버 항상 OPEN 상태로 고정
bool cover_set(uint8_t cmd) {
    Serial.println("[COVER] Motor disabled — cover fixed OPEN");
    return true;
}