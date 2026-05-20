// rotary_encoder_led_test.ino
// Arduino Nano ESP32 + Rotary Encoder + 4 Mode LEDs
//
// 연결 기준:
// Arduino 3V3  -> J6-1  VCC_3V3
// Arduino GND  -> J6-2  GND
// Arduino D2   -> J6-3  ENC_A
// Arduino D3   -> J6-4  ENC_B
// Arduino D4   -> J6-5  ENC_SW
// Arduino D5   -> J6-6  LED1_K, PCB LED D2
// Arduino D6   -> J6-7  LED2_K, PCB LED D3
// Arduino D7   -> J6-8  LED3_K, PCB LED D4
// Arduino D8   -> J6-9  LED4_K, PCB LED D5

#define ENC_A_PIN   D2
#define ENC_B_PIN   D3
#define ENC_SW_PIN  D4

// PCB LED 기준:
// LED D2 = mode 0 AUTO
// LED D3 = mode 1 CLOSED
// LED D4 = mode 2 BREEZE
// LED D5 = mode 3 TURBO
#define LED_AUTO_PIN    D5
#define LED_CLOSED_PIN  D6
#define LED_BREEZE_PIN  D7
#define LED_TURBO_PIN   D8

#define MODE_COUNT 4

int currentMode = 0;

const char* modeNames[MODE_COUNT] = {
  "AUTO",
  "CLOSED",
  "BREEZE",
  "TURBO"
};

int lastEncoded = 0;
int encoderStep = 0;

int lastButtonReading = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;

// 이 회로는 LED_K가 LOW일 때 LED ON, HIGH일 때 LED OFF
void allLedsOff() {
  digitalWrite(LED_AUTO_PIN, HIGH);
  digitalWrite(LED_CLOSED_PIN, HIGH);
  digitalWrite(LED_BREEZE_PIN, HIGH);
  digitalWrite(LED_TURBO_PIN, HIGH);
}

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
  Serial.println("0 = AUTO    -> PCB LED D2");
  Serial.println("1 = CLOSED  -> PCB LED D3");
  Serial.println("2 = BREEZE  -> PCB LED D4");
  Serial.println("3 = TURBO   -> PCB LED D5");
  Serial.println("--------------------------------");
  Serial.println("Rotate dial: change mode");
  Serial.println("Press dial : confirm selected mode");
  Serial.println("Mode wraps around: 3 -> 0, 0 -> 3");
  Serial.println("================================");
  printCurrentMode();
}

void printModeChange(const char* direction) {
  Serial.print(direction);
  Serial.print(" -> mode ");
  Serial.print(currentMode);
  Serial.print(" ");
  Serial.println(modeNames[currentMode]);
}

void nextMode() {
  currentMode++;

  if (currentMode >= MODE_COUNT) {
    currentMode = 0;
  }

  showModeLed(currentMode);
  printModeChange("CW");
}

void previousMode() {
  currentMode--;

  if (currentMode < 0) {
    currentMode = MODE_COUNT - 1;
  }

  showModeLed(currentMode);
  printModeChange("CCW");
}

void checkEncoder() {
  int msb = digitalRead(ENC_A_PIN);
  int lsb = digitalRead(ENC_B_PIN);

  int encoded = (msb << 1) | lsb;
  int sum = (lastEncoded << 2) | encoded;

  // Quadrature decoding
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderStep++;
  }

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderStep--;
  }

  // 일반적인 로터리 엔코더는 한 칸 이동에 4 step 정도 발생
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

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // 버튼은 눌렀을 때 LOW
      if (stableButtonState == LOW) {
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

  // USB Serial 안정화 대기
  delay(3000);

  pinMode(ENC_A_PIN, INPUT);
  pinMode(ENC_B_PIN, INPUT);
  pinMode(ENC_SW_PIN, INPUT);

  pinMode(LED_AUTO_PIN, OUTPUT);
  pinMode(LED_CLOSED_PIN, OUTPUT);
  pinMode(LED_BREEZE_PIN, OUTPUT);
  pinMode(LED_TURBO_PIN, OUTPUT);

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