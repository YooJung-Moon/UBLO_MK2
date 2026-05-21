// rotary_encoder_led_test.ino
// Arduino Nano ESP32 + Rotary Encoder + 4 Mode LEDs
//
// 연결 기준:
// Arduino D4  -> ENC_A
// Arduino D7  -> ENC_B
// Arduino D8  -> ENC_SW
// Arduino A0  -> LED1_K, PCB LED D2 (AUTO)
// Arduino A1  -> LED2_K, PCB LED D3 (CLOSED)
// Arduino A2  -> LED3_K, PCB LED D4 (BREEZE)
// Arduino A3  -> LED4_K, PCB LED D5 (TURBO)

#define ENC_A_PIN   D8
#define ENC_B_PIN   D7
#define ENC_SW_PIN  D10

#define LED_AUTO_PIN    A0  // PCB LED D2
#define LED_CLOSED_PIN  A1  // PCB LED D3
#define LED_BREEZE_PIN  A2  // PCB LED D4
#define LED_TURBO_PIN   A3  // PCB LED D5

#define MODE_COUNT 4

int currentMode = 0;
int previewMode = 0;  // 회전 시 미리보기 모드 (버튼 누르기 전까지 currentMode에 반영 안 됨)

const char* modeNames[MODE_COUNT] = {
  "AUTO",
  "CLOSED",
  "BREEZE",
  "TURBO"
};

// Quadrature decoding을 위한 이전 encoder 상태 저장
int lastEncoded = 0;
// encoder 펄스 누적 카운터 (한 칸 이동 시 A, B 신호가 4번 변화)
int encoderStep = 0;

// 버튼 디바운싱을 위한 변수
int lastButtonReading = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;  // 40ms 이내 신호 변화는 노이즈로 처리

// LED는 캐소드(-) 연결이라 LOW일 때 ON, HIGH일 때 OFF
void allLedsOff() {
  digitalWrite(LED_AUTO_PIN, HIGH);    // PCB LED D2
  digitalWrite(LED_CLOSED_PIN, HIGH);  // PCB LED D3
  digitalWrite(LED_BREEZE_PIN, HIGH);  // PCB LED D4
  digitalWrite(LED_TURBO_PIN, HIGH);   // PCB LED D5
}

// 해당 모드 LED만 켜고 나머지는 끔
void showModeLed(int mode) {
  allLedsOff();
  if (mode == 0) {
    digitalWrite(LED_AUTO_PIN, LOW);      // PCB LED D2
  } else if (mode == 1) {
    digitalWrite(LED_CLOSED_PIN, LOW);    // PCB LED D3
  } else if (mode == 2) {
    digitalWrite(LED_BREEZE_PIN, LOW);    // PCB LED D4
  } else if (mode == 3) {
    digitalWrite(LED_TURBO_PIN, LOW);     // PCB LED D5
  }
}

void printCurrentMode() {
  Serial.print("Current mode: ");
  Serial.print(currentMode);
  Serial.print(" ");
  Serial.println(modeNames[currentMode]);
}

void printStartupMessage() {
  Serial.println();
  Serial.println("================================");
  Serial.println("Rotary Mode Selector Test Start");
  Serial.println("Board: Arduino Nano ESP32");
  Serial.println("Baud rate: 115200");
  Serial.println("--------------------------------");
  Serial.println("Mode table:");
  Serial.println("0 = AUTO    -> PCB LED D2 (A0)");
  Serial.println("1 = CLOSED  -> PCB LED D3 (A1)");
  Serial.println("2 = BREEZE  -> PCB LED D4 (A2)");
  Serial.println("3 = TURBO   -> PCB LED D5 (A3)");
  Serial.println("--------------------------------");
  Serial.println("Rotate dial: preview mode");
  Serial.println("Press dial : confirm selected mode");
  Serial.println("Mode wraps around: 3 -> 0, 0 -> 3");
  Serial.println("================================");
  printCurrentMode();
}

void printModeChange(const char* direction) {
  Serial.print(direction);
  Serial.print(" -> preview mode ");
  Serial.print(previewMode);
  Serial.print(" ");
  Serial.println(modeNames[previewMode]);
}

// CW 회전: previewMode만 증가, LED 미리보기
void nextMode() {
  previewMode++;
  if (previewMode >= MODE_COUNT) previewMode = 0;
  showModeLed(previewMode);
  printModeChange("CW");
}

// CCW 회전: previewMode만 감소, LED 미리보기
void previousMode() {
  previewMode--;
  if (previewMode < 0) previewMode = MODE_COUNT - 1;
  showModeLed(previewMode);
  printModeChange("CCW");
}

void checkEncoder() {
  int msb = digitalRead(ENC_A_PIN);
  int lsb = digitalRead(ENC_B_PIN);
  int encoded = (msb << 1) | lsb;

  // 이전 상태와 현재 상태를 조합해서 회전 방향 판별 (Quadrature decoding)
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderStep++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderStep--;

  // A, B 신호가 4번 변화해야 1칸 이동으로 인식
  if (encoderStep >= 4) {
    encoderStep = 0;
    nextMode();
  }
  if (encoderStep <= -4) {
    encoderStep = 0;
    previousMode();
  }

  lastEncoded = encoded;
}

void checkButton() {
  int reading = digitalRead(ENC_SW_PIN);

  // 신호 변화 감지 시 디바운스 타이머 리셋
  if (reading != lastButtonReading) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // 버튼 눌림(LOW) 확정 시 previewMode를 currentMode로 반영
      if (stableButtonState == LOW) {
        currentMode = previewMode;
        showModeLed(currentMode);
        Serial.print("CONFIRMED mode ");
        Serial.print(currentMode);
        Serial.print(" ");
        Serial.println(modeNames[currentMode]);
      }
    }
  }
  lastButtonReading = reading;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  pinMode(ENC_A_PIN, INPUT);
  pinMode(ENC_B_PIN, INPUT);
  pinMode(ENC_SW_PIN, INPUT);

  pinMode(LED_AUTO_PIN, OUTPUT);
  pinMode(LED_CLOSED_PIN, OUTPUT);
  pinMode(LED_BREEZE_PIN, OUTPUT);
  pinMode(LED_TURBO_PIN, OUTPUT);

  // 초기 encoder 상태 저장
  int msb = digitalRead(ENC_A_PIN);
  int lsb = digitalRead(ENC_B_PIN);
  lastEncoded = (msb << 1) | lsb;

  showModeLed(currentMode);
  printStartupMessage();
}

void loop() {
  checkEncoder();
  checkButton();
}