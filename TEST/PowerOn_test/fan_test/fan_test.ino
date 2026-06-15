#define BLOWER_EN_PIN D4
#define FAN_PWM_PIN   D7

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(BLOWER_EN_PIN, OUTPUT);
    pinMode(FAN_PWM_PIN,   OUTPUT);

    Serial.println("Fan test start");
}

void loop() {
    Serial.println("Fan ON");
    digitalWrite(BLOWER_EN_PIN, HIGH);  // Blower EN 활성화
    digitalWrite(FAN_PWM_PIN,   LOW);   // active low: LOW = fan ON
    delay(3000);

    Serial.println("Fan OFF");
    digitalWrite(FAN_PWM_PIN,   HIGH);  // active low: HIGH = fan OFF
    digitalWrite(BLOWER_EN_PIN, LOW);   // Blower EN 비활성화
    delay(3000);
}