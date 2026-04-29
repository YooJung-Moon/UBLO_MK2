#include "actuators.h"
#include "config.h"

void actuators_init() {
    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(LIM_OPEN,  INPUT_PULLUP);
    pinMode(LIM_CLOSE, INPUT_PULLUP);
    pinMode(FAN_PWM,   OUTPUT);

    // 초기 상태: fan OFF, 모터 정지
    analogWrite(FAN_PWM, 0);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);

    Serial.println("Actuators initialized");
}

void fan_set(uint8_t cmd) {
    if (cmd == 1) {
        analogWrite(FAN_PWM, 255);
        Serial.println("Fan ON");
    } else {
        analogWrite(FAN_PWM, 0);
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