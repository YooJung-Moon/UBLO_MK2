#define FAN_PIN 18  

#define PWM_CHANNEL 0
#define PWM_FREQ 25000
#define PWM_RESOLUTION 8   // 0~255

void setup() {
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(FAN_PIN, PWM_CHANNEL);
}

void loop() {

  // 점점 속도 증가
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(PWM_CHANNEL, duty);
    delay(20);
  }

  // 점점 속도 감소
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(PWM_CHANNEL, duty);
    delay(20);
  }
}