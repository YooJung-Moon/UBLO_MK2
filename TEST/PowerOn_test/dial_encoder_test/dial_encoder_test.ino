// rotary_encoder_led_test.ino
// Arduino Nano ESP32 + Rotary Encoder + 4 Mode LEDs

#define ENC_A_PIN   D2   // J6-3
#define ENC_B_PIN   D3   // J6-4
#define ENC_SW_PIN  D4   // J6-5

// PCB LED 이름 기준
// LED D2 = mode 0 AUTO
// LED D3 = mode 1 CLOSED
// LED D4 = mode 2 BREEZE
// LED D5 = mode 3 TURBO
#define LED_AUTO_PIN    D5   // J6-6, PCB LED D2
#define LED_CLOSED_PIN  D6   // J6-7, PCB LED D3
#define LED_BREEZE_PIN  D7   // J6-8, PCB LED D4
#define LED_TURBO_PIN   D8   // J6-9, PCB LED D5

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

void allLedsOff() {
  // 이 회로는 LOW일 때 LED ON, HIGH일 때 LED OFF
  digitalWrite(LED_AUTO_PIN, HIGH);
  digitalWrite(LED_CLOSED_PIN, HIGH);
  digitalWrite(LED_BREEZE_PIN, HIGH);
  digitalWrite(LED_TURBO_PIN, HIGH);
}

void showModeLed(int mode) {
  allLedsOff();

  if (mode == 0) digitalWrite(LED_AUTO_PIN, LOW);
  if (mode == 1) digitalWrite(LED_CLOSED_PIN, LOW);
  if (mode == 2) digitalWrite(LED_BREEZE_PIN, LOW);
  if (mode == 3) digitalWrite(LED_TURBO_PIN, LOW);
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

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderStep++;
  }

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderStep--;
  }

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
  delay(1000);

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

  Serial.println("Rotary mode selector test start");
  Serial.println("Initial mode 0 AUTO");
}

void loop() {
  checkEncoder();
  checkButton();
}