#define MOTOR_IN1  2
#define MOTOR_IN2  3
#define LIM_OPEN   5
#define LIM_CLOSE  6
#define FAN_PWM    18

void motorCW() {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
}

void motorCCW() {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
}

void motorStop() {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
}

void setup() {
    delay(3000);
    Serial.begin(115200);
    while (!Serial);

    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(LIM_OPEN,  INPUT_PULLUP);
    pinMode(LIM_CLOSE, INPUT_PULLUP);
    pinMode(FAN_PWM,   OUTPUT);

    analogWrite(FAN_PWM, 0);
    motorCW();
    Serial.println("Ready — Motor CW, Fan OFF");
}

void loop() {
    // LIM_OPEN 눌림
    if (digitalRead(LIM_OPEN) == LOW) {
        Serial.println("LIM_OPEN → Motor stop, Fan ON");
        motorStop();
        analogWrite(FAN_PWM, 255);
        delay(3000);
        // 초기 상태 복귀
        analogWrite(FAN_PWM, 0);
        motorCW();
        Serial.println("Back to initial state");
    }

    // LIM_CLOSE 눌림
    if (digitalRead(LIM_CLOSE) == LOW) {
        Serial.println("LIM_CLOSE → Motor CCW, Fan OFF");
        analogWrite(FAN_PWM, 0);
        motorCCW();
        delay(3000);
        // 초기 상태 복귀
        motorCW();
        Serial.println("Back to initial state");
    }
}