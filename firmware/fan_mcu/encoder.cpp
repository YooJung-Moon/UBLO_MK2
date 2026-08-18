#include "encoder.h"
#include "config.h"

#define MODE_COUNT          4
#define PREVIEW_TIMEOUT     7000  // 7초 후 current_mode로 복귀
#define PREVIEW_BLINK_MS    500    // preview 깜빡임 간격 (ms)
#define CONFIRM_BLINK_MS    100    // 확정 시 깜빡임 간격 (ms)
#define CONFIRM_BLINK_COUNT 3      // 확정 시 깜빡임 횟수
#define ERROR_BLINK_MS      300    // 커버 타임아웃 에러 깜빡임 간격 (ms) — LED 4개 동시
#define COMMS_LOST_BLINK_MS 400    // AUTO 통신두절 깜빡임 간격 (ms) — LED_AUTO_PIN 1개만
                                    // ERROR_BLINK_MS(300ms, 4개)와 속도/개수 둘 다 달라서 구별됨

static uint8_t current_mode = MODE_AUTO;  // 초기 모드: AUTO
static uint8_t preview_mode = MODE_AUTO;
static bool mode_confirmed = false;
static bool in_preview = false;
static unsigned long last_rotate_time = 0;

// preview 깜빡임 상태
static bool blink_state = false;
static unsigned long last_blink_time = 0;

// 에러 상태 (커버 타임아웃 발생 시 진입) — 모터/리밋스위치 하드웨어 오류
static bool error_state = false;
static bool error_blink_state = false;
static unsigned long last_error_blink_time = 0;

// 통신 두절 상태 (AUTO 모드에서 command_packet 미수신 지속 시) — 하드웨어 오류와는 별개
static bool comms_lost_state = false;
static bool comms_lost_blink_state = false;
static unsigned long last_comms_lost_blink_time = 0;

// Quadrature decoding
static int last_encoded = 0;
static int encoder_step = 0;  // 4 펄스 누적 시 1칸 이동으로 인식

// 버튼 디바운싱
static int last_button_reading = HIGH;
static int stable_button_state = HIGH;
static unsigned long last_debounce_time = 0;
const unsigned long DEBOUNCE_DELAY = 40;  // 40ms 이내 신호 변화는 노이즈로 처리

const char* mode_names[MODE_COUNT] = {
    "AUTO",
    "CLOSE",
    "OPEN",
    "TURBO"
};

// LED는 캐소드(-) 연결이라 LOW일 때 ON, HIGH일 때 OFF
static void all_leds_off() {
    digitalWrite(LED_AUTO_PIN,  HIGH);
    digitalWrite(LED_CLOSE_PIN, HIGH);
    digitalWrite(LED_OPEN_PIN,  HIGH);
    digitalWrite(LED_TURBO_PIN, HIGH);
}

static void led_on(uint8_t mode) {
    all_leds_off();
    if (mode == MODE_AUTO)  digitalWrite(LED_AUTO_PIN,  LOW);
    if (mode == MODE_CLOSE) digitalWrite(LED_CLOSE_PIN, LOW);
    if (mode == MODE_OPEN)  digitalWrite(LED_OPEN_PIN,  LOW);
    if (mode == MODE_TURBO) digitalWrite(LED_TURBO_PIN, LOW);
}

static void show_mode_led(uint8_t mode) {
    led_on(mode);
}

// 모드 확정 시 빠르게 3번 깜빡인 후 고정
static void confirm_blink(uint8_t mode) {
    for (int i = 0; i < CONFIRM_BLINK_COUNT; i++) {
        all_leds_off();
        delay(CONFIRM_BLINK_MS);
        led_on(mode);
        delay(CONFIRM_BLINK_MS);
    }
}

// CW 회전: preview_mode 증가
static void next_mode() {
    preview_mode++;
    if (preview_mode >= MODE_COUNT) preview_mode = 0;
    in_preview = (preview_mode != current_mode);
    last_rotate_time = millis();
    blink_state = true;
    last_blink_time = millis();
    last_error_blink_time = millis();    // 에러 깜빡임 타이머 리셋 → 300ms 동안 깜빡임 중단
    led_on(preview_mode);
    Serial.print("CW -> preview mode ");
    Serial.print(preview_mode);
    Serial.print(" ");
    Serial.println(mode_names[preview_mode]);
}

// CCW 회전: preview_mode 감소
static void prev_mode() {
    preview_mode = (preview_mode == 0) ? MODE_COUNT - 1 : preview_mode - 1;
    in_preview = (preview_mode != current_mode);
    last_rotate_time = millis();
    blink_state = true;
    last_blink_time = millis();
    last_error_blink_time = millis();    // 에러 깜빡임 타이머 리셋 → 300ms 동안 깜빡임 중단
    led_on(preview_mode);
    Serial.print("CCW -> preview mode ");
    Serial.print(preview_mode);
    Serial.print(" ");
    Serial.println(mode_names[preview_mode]);
}

static void check_encoder() {
    int msb = digitalRead(ENC_A_PIN);
    int lsb = digitalRead(ENC_B_PIN);
    int encoded = (msb << 1) | lsb;

    int sum = (last_encoded << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoder_step++;
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoder_step--;

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

    if (reading != last_button_reading) last_debounce_time = millis();

    if ((millis() - last_debounce_time) > DEBOUNCE_DELAY) {
        if (reading != stable_button_state) {
            stable_button_state = reading;

            if (stable_button_state == LOW) {
                error_state = false;
                current_mode = preview_mode;
                in_preview = false;
                confirm_blink(current_mode);
                mode_confirmed = true;
                Serial.print("[ENCODER] Mode changed to: ");
                Serial.println(mode_names[current_mode]);
            }
        }
    }
    last_button_reading = reading;
}

static void update_preview() {
    if (!in_preview) return;

    if (millis() - last_rotate_time >= PREVIEW_TIMEOUT) {
        preview_mode = current_mode;
        in_preview = false;
        // preview 종료 후엔 comms_lost 상태면 그쪽 LED로, 아니면 현재 모드 LED로 복귀
        if (comms_lost_state) {
            led_on(MODE_AUTO);
        } else {
            show_mode_led(current_mode);
        }
        Serial.println("[ENCODER] Preview timeout → current mode");
        return;
    }

    if (millis() - last_blink_time >= PREVIEW_BLINK_MS) {
        last_blink_time = millis();
        blink_state = !blink_state;
        if (blink_state) {
            led_on(preview_mode);
        } else {
            all_leds_off();
        }
    }
}

static void update_error_blink() {
    if (!error_state) return;

    if (millis() - last_error_blink_time >= ERROR_BLINK_MS) {
        last_error_blink_time = millis();
        error_blink_state = !error_blink_state;

        if (error_blink_state) {
            digitalWrite(LED_AUTO_PIN,  LOW);
            digitalWrite(LED_CLOSE_PIN, LOW);
            digitalWrite(LED_OPEN_PIN,  LOW);
            digitalWrite(LED_TURBO_PIN, LOW);
        } else {
            all_leds_off();
        }
    }
}

// AUTO 모드 통신 두절 표시 — LED_AUTO_PIN 하나만 깜빡임.
// 커버 타임아웃 에러(4개 동시 깜빡임)와는 개수/속도 모두 달라 육안으로 구별된다.
static void update_comms_lost_blink() {
    if (!comms_lost_state) return;

    if (millis() - last_comms_lost_blink_time >= COMMS_LOST_BLINK_MS) {
        last_comms_lost_blink_time = millis();
        comms_lost_blink_state = !comms_lost_blink_state;
        if (comms_lost_blink_state) {
            led_on(MODE_AUTO);
        } else {
            all_leds_off();
        }
    }
}

void encoder_init() {
    pinMode(ENC_A_PIN,  INPUT);
    pinMode(ENC_B_PIN,  INPUT);
    pinMode(ENC_SW_PIN, INPUT);

    pinMode(LED_AUTO_PIN,  OUTPUT);
    pinMode(LED_CLOSE_PIN, OUTPUT);
    pinMode(LED_OPEN_PIN,  OUTPUT);
    pinMode(LED_TURBO_PIN, OUTPUT);

    int msb = digitalRead(ENC_A_PIN);
    int lsb = digitalRead(ENC_B_PIN);
    last_encoded = (msb << 1) | lsb;

    show_mode_led(current_mode);
    Serial.println("Mode input: 0=AUTO, 1=CLOSE, 2=OPEN, 3=TURBO");
}

void encoder_update() {
    check_encoder();
    check_button();

    // 표시 우선순위: 커버 타임아웃 에러(4개) > 사용자 조작 중인 프리뷰 > 통신두절(AUTO 1개)
    // 프리뷰가 comms_lost보다 우선해야 사용자가 다이얼 돌리는 동안은 정상적으로 미리보기가 보인다.
    if (error_state) {
        update_error_blink();
    } else if (in_preview) {
        update_preview();
    } else if (comms_lost_state) {
        update_comms_lost_blink();
    } else {
        update_preview();  // in_preview가 false이므로 즉시 반환 — 상태 안전망 목적으로 유지
    }
}

bool encoder_changed() {
    if (mode_confirmed) {
        mode_confirmed = false;
        return true;
    }
    return false;
}

uint8_t encoder_get_mode() {
    return current_mode;
}

void encoder_set_mode(uint8_t mode) {
    current_mode = mode;
    preview_mode = mode;
    in_preview = false;
    // comms_lost 상태가 아닐 때만 즉시 모드 LED로 표시 (comms_lost면 다음 encoder_update()에서
    // update_comms_lost_blink()가 이어서 처리하므로 여기서 겹쳐 쓰지 않는다)
    if (!comms_lost_state) {
        show_mode_led(current_mode);
    }
}

void encoder_error_blink() {
    error_state = true;
    in_preview = false;
    error_blink_state = true;
    last_error_blink_time = millis();
    Serial.println("[ENCODER] Error state entered");
}

bool encoder_is_error() {
    return error_state;
}

// lost=true 진입 시 AUTO LED 깜빡임 시작, false 진입 시 정상 모드 LED로 복귀.
// 이미 같은 상태면 아무 것도 하지 않아 매 loop 호출에도 불필요한 리셋이 없도록 한다.
void encoder_set_comms_lost(bool lost) {
    if (lost == comms_lost_state) return;
    comms_lost_state = lost;

    if (lost) {
        comms_lost_blink_state = true;
        last_comms_lost_blink_time = millis();
        led_on(MODE_AUTO);
        Serial.println("[ENCODER] Comms lost blink started (AUTO)");
    } else {
        // 에러/프리뷰 중이 아니면 현재 모드 LED로 즉시 복귀
        if (!error_state && !in_preview) {
            show_mode_led(current_mode);
        }
        Serial.println("[ENCODER] Comms lost blink stopped");
    }
}