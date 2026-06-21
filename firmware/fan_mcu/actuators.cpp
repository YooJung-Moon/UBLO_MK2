#include "actuators.h"
#include "config.h"

#define COVER_TIMEOUT_MS 15000  // 리밋스위치 미감지 시 타임아웃 (15초)

void actuators_init() {
    pinMode(MOTOR_IN1,  OUTPUT);
    pinMode(MOTOR_IN2,  OUTPUT);
    pinMode(LIM_OPEN,   INPUT_PULLUP);
    pinMode(LIM_CLOSE,  INPUT_PULLUP);
    pinMode(FAN_PWM,    OUTPUT);
    pinMode(BLOWER_EN,  OUTPUT);

    // 초기 상태: fan OFF, 모터 정지
    analogWrite(FAN_PWM, 255);     // active low: 255 = fan OFF
    digitalWrite(BLOWER_EN, LOW);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);

    Serial.println("Actuators initialized");
}

void fan_set(uint8_t cmd) {
    if (cmd == 1) {
        digitalWrite(BLOWER_EN, HIGH);
        analogWrite(FAN_PWM, 255 - (255 * FAN_SPEED_PCT / 100));  // active low 변환
        Serial.println("Fan ON");
    } else {
        analogWrite(FAN_PWM, 255);     // active low: 255 = fan OFF
        digitalWrite(BLOWER_EN, LOW);
        Serial.println("Fan OFF");
    }
}

bool cover_set(uint8_t cmd) {
    unsigned long start = millis();

    if (cmd == 1) {
        // OPEN: 모터 정방향
        Serial.println("Cover opening...");
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
        while (digitalRead(LIM_OPEN) == HIGH) {
            if (millis() - start >= COVER_TIMEOUT_MS) {
                digitalWrite(MOTOR_IN1, LOW);
                digitalWrite(MOTOR_IN2, LOW);
                Serial.println("[ERROR] Cover open timeout!");
                return false;
            }
            delay(10);
        }
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        Serial.println("Cover opened");
    } else {
        // CLOSE: 모터 역방향
        Serial.println("Cover closing...");
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
        while (digitalRead(LIM_CLOSE) == HIGH) {
            if (millis() - start >= COVER_TIMEOUT_MS) {
                digitalWrite(MOTOR_IN1, LOW);
                digitalWrite(MOTOR_IN2, LOW);
                Serial.println("[ERROR] Cover close timeout!");
                return false;
            }
            delay(10);
        }
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        Serial.println("Cover closed");
    }

    return true;
}