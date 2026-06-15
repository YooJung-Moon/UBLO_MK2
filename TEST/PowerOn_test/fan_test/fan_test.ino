#define FAN_PIN D7 

#define PWM_CHANNEL 0
#define PWM_FREQ 25000
#define PWM_RESOLUTION 8

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Fan PWM test start");

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(FAN_PIN, PWM_CHANNEL);

  Serial.println("PWM attached to GPIO18");
}

void loop() {
  Serial.println("Fan duty 255");
  ledcWrite(PWM_CHANNEL, 255);
  delay(3000);

  Serial.println("Fan duty 0");
  ledcWrite(PWM_CHANNEL, 0);
  delay(3000);
}