#define IN1 2
#define IN2 3

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  // 한 방향
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  delay(2000);

  // 반대 방향
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  delay(2000);

  // 정지
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(2000);
}