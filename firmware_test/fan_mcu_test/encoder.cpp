#include "encoder.h"
#include "config.h"

// 핀 정의
#define ENC_A_PIN   D4
#define ENC_B_PIN   D7
#define ENC_SW_PIN  D8

#define LED_AUTO_PIN    A0  // PCB LED D2
#define LED_CLOSED_PIN  A1  // PCB LED D3
#define LED_BREEZE_PIN  A2  // PCB LED D4
#define LED_TURBO_PIN   A3  // PCB LED D5

#define MODE_COUNT 4

static uint8_t current_mode = MODE_BREEZE;  // 초기 모드: BREEZE
static uint8_t preview_mode = MODE_BREEZE;  // 회전 시 미리보기 모드
static bool mode_confirmed = false;         // 버튼 눌림 여부

// Quadrature decoding을 위한 이전 encoder 상태 저장
static int last_encoded = 0;
// encoder 펄스 누적 카운터 (한 칸 이동 시 A, B 신호가 4번 변화)
static int encoder_step = 0;

// 버튼 디바운싱을 위한 변수
static int last_button_reading = HIGH;
static int stable_button_state = HIGH;
static unsigned long last_debounce_time = 0;
const unsigned long DEBOUNCE_DELAY = 40;  // 40ms 이내 신호 변화는 노이즈로 처리

const char* mode_names[MODE_COUNT] = {
    "AUTO",
    "CLOSED",
    "BREEZE",
    "TURBO"
};

// LED는 캐소드(-) 연결이라 LOW일 때 ON, HIGH일 때 OFF
static void all_leds_off() {
    digitalWrite(LED_AUTO_PIN, HIGH);    // PCB LED D2
    digitalWrite(LED_CLOSED_PIN, HIGH);  // PCB LED D3
    digitalWrite(LED_BREEZE_PIN, HIGH);  // PCB LED D4
    digitalWrite(LED_TURBO_PIN, HIGH);   // PCB LED D5
}

// 해당 모드 LED만 켜고 나머지는 끔
static void show_mode_led(uint8_t mode) {
    all_leds_off();
    if (mode == MODE_AUTO)   digitalWrite(LED_AUTO_PIN, LOW);    // PCB LED D2
    if (mode == MODE_CLOSED) digitalWrite(LED_CLOSED_PIN, LOW);  // PCB LED D3
    if (mode == MODE_BREEZE) digitalWrite(LED_BREEZE_PIN, LOW);  // PCB LED D4
    if (mode == MODE_TURBO)  digitalWrite(LED_TURBO_PIN, LOW);   // PCB LED D5
}

// CW 회전: preview_mode만 증가, LED 미리보기
static void next_mode() {
    preview_mode++;
    if (preview_mode >= MODE_COUNT) preview_mode = 0;
    show_mode_led(preview_mode);
    Serial.print("CW -> preview mode ");
    Serial.print(preview_mode);
    Serial.print(" ");
    Serial.println(mode_names[preview_mode]);
}

// CCW 회전: preview_mode만 감소, LED 미리보기
static void prev_mode() {
    preview_mode = (preview_mode == 0) ? MODE_COUNT - 1 : preview_mode - 1;
    show_mode_led(preview_mode);
    Serial.print("CCW -> preview mode ");
    Serial.print(preview_mode);
    Serial.print(" ");
    Serial.println(mode_names[preview_mode]);
}

static void check_encoder() {
    int msb = digitalRead(ENC_A_PIN);
    int lsb = digitalRead(ENC_B_PIN);
    int encoded = (msb << 1) | lsb;

    // 이전 상태와 현재 상태를 조합해서 회전 방향 판별 (Quadrature decoding)
    int sum = (last_encoded << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoder_step++;
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoder_step--;

    // A, B 신호가 4번 변화해야 1칸 이동으로 인식
    if (encoder_step >= 4) {
        encoder_step = 0;
        next_mode();
    }
    if (encoder_step <= -4) {
        encoder_step = 0;
        prev_mode();
    }

    last_encoded = encoded;
}

static void check_button() {
    int reading = digitalRead(ENC_SW_PIN);

    // 신호 변화 감지 시 디바운스 타이머 리셋
    if (reading != last_button_reading) last_debounce_time = millis();

    if ((millis() - last_debounce_time) > DEBOUNCE_DELAY) {
        if (reading != stable_button_state) {
            stable_button_state = reading;

            // 버튼 눌림(LOW) 확정 시 preview_mode를 current_mode로 반영
            if (stable_button_state == LOW) {
                current_mode = preview_mode;
                show_mode_led(current_mode);
                mode_confirmed = true;
                Serial.print("[ENCODER] Mode changed to: ");
                Serial.println(mode_names[current_mode]);
            }
        }
    }
    last_button_reading = reading;
}

void encoder_init() {
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
    last_encoded = (msb << 1) | lsb;

    show_mode_led(current_mode);
    Serial.println("Mode input: 0=AUTO, 1=CLOSED, 2=BREEZE, 3=TURBO");
}

bool encoder_changed() {
    check_encoder();
    check_button();

    if (mode_confirmed) {
        mode_confirmed = false;
        return true;
    }
    return false;
}

uint8_t encoder_get_mode() {
    return current_mode;
}

// timeout이나 AUTO 복귀 시 외부에서 모드 강제 설정
void encoder_set_mode(uint8_t mode) {
    current_mode = mode;
    preview_mode = mode;
    show_mode_led(current_mode);
}