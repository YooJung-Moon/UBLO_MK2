#define IN1 D3
#define IN2 D2

void motorForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void motorReverse() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  motorForward();
  delay(10000);

  motorStop();
  delay(1000);   // 정지 시간 — 필요에 따라 조정

  motorReverse();
  delay(10000);

  motorStop();
  delay(1000);
}