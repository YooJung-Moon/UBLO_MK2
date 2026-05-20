// Arduino Nano ESP32 + 다이얼 PCB J6 테스트 코드

const int ENC_A  = D2;  // J6-3
const int ENC_B  = D3;  // J6-4
const int ENC_SW = D4;  // J6-5

const int LED1 = D5;    // J6-6
const int LED2 = D6;    // J6-7
const int LED3 = D7;    // J6-8
const int LED4 = D8;    // J6-9

int lastA = HIGH;
long encoderCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // PCB에 10k 풀업 저항이 있으므로 INPUT 사용
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  pinMode(ENC_SW, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // 이 회로는 LED_K가 LOW일 때 LED 켜짐
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);

  lastA = digitalRead(ENC_A);

  Serial.println("Dial PCB test start");
  Serial.println("Turn encoder / press switch");
}

void loop() {
  int a = digitalRead(ENC_A);
  int b = digitalRead(ENC_B);
  int sw = digitalRead(ENC_SW);

  // 엔코더 회전 감지
  if (a != lastA) {
    if (a == LOW) {
      if (b == HIGH) {
        encoderCount++;
        Serial.print("CW  count = ");
      } else {
        encoderCount--;
        Serial.print("CCW count = ");
      }
      Serial.println(encoderCount);
    }
    lastA = a;
  }

  // 버튼 상태 출력
  static int lastSW = HIGH;
  if (sw != lastSW) {
    if (sw == LOW) {
      Serial.println("Button pressed");
    } else {
      Serial.println("Button released");
    }
    lastSW = sw;
  }

  // 버튼 누르면 LED1 켜짐
  if (sw == LOW) {
    digitalWrite(LED1, LOW);   // ON
  } else {
    digitalWrite(LED1, HIGH);  // OFF
  }

  // encoderCount 값에 따라 LED2~LED4 테스트
  if (encoderCount % 2 == 0) {
    digitalWrite(LED2, LOW);   // ON
  } else {
    digitalWrite(LED2, HIGH);  // OFF
  }

  if (encoderCount % 3 == 0) {
    digitalWrite(LED3, LOW);
  } else {
    digitalWrite(LED3, HIGH);
  }

  if (encoderCount % 4 == 0) {
    digitalWrite(LED4, LOW);
  } else {
    digitalWrite(LED4, HIGH);
  }

  delay(2);
}