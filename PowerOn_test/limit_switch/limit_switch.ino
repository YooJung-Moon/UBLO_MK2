void setup() {
    delay(3000);
    Serial.begin(115200);
    while (!Serial);
    
    // 모든 디지털 핀을 INPUT_PULLUP으로 설정
    for (int i = 0; i <= 21; i++) {
        pinMode(i, INPUT_PULLUP);
    }
    Serial.println("Press limit switch and watch which pin changes");
}

void loop() {
    for (int i = 0; i <= 21; i++) {
        if (digitalRead(i) == LOW) {
            Serial.print("Pin ");
            Serial.print(i);
            Serial.println(" is LOW (pressed)");
        }
    }
    delay(200);
}