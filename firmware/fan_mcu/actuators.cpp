#include "actuators.h"
#include "config.h"

void actuators_init() {
    pinMode(MOTOR_IN1,  OUTPUT);
    pinMode(MOTOR_IN2,  OUTPUT);
    pinMode(LIM_OPEN,   INPUT_PULLUP);
    pinMode(LIM_CLOSE,  INPUT_PULLUP);
    pinMode(FAN_PWM,    OUTPUT);
    pinMode(BLOWER_EN,  OUTPUT);  // Blower EN 핀 초기화

    // 초기 상태: fan OFF, 모터 정지
    analogWrite(FAN_PWM, 255);     // active low: 255 = fan OFF
    digitalWrite(BLOWER_EN, LOW);  // Blower EN 비활성화
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);

    Serial.println("Actuators initialized");
}

void fan_set(uint8_t cmd) {
    if (cmd == 1) {
        digitalWrite(BLOWER_EN, HIGH);  // Blower EN 활성화
        analogWrite(FAN_PWM, 0);        // active low: 0 = fan ON
        Serial.println("Fan ON");
    } else {
        analogWrite(FAN_PWM, 255);      // active low: 255 = fan OFF
        digitalWrite(BLOWER_EN, LOW);   // Blower EN 비활성화
        Serial.println("Fan OFF");
    }
}

void cover_set(uint8_t cmd) {
    if (cmd == 1) {
        // OPEN: 모터 정방향
        Serial.println("Cover opening...");
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
        while (digitalRead(LIM_OPEN) == HIGH) {
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
            delay(10);
        }
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        Serial.println("Cover closed");
    }
}