#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN     5
#define LED_COUNT   1

#define CO2_GREEN   800
#define CO2_YELLOW  1000

// 통신 두절 표시 — CO2 색(초/노/빨) 대신 흰색으로 표시, 밝기만 부드럽게 오르내림.
// 완전 고정 흰색으로 하면 센서 read 실패 표시(led_set_error_sensor, 역시 흰색 고정)와
// 육안으로 구별이 안 되므로, 애니메이션(숨쉬기)으로 "고정 vs 움직임"을 구분점으로 둔다.
#define COMMS_LOST_WHITE_R  80
#define COMMS_LOST_WHITE_G  80
#define COMMS_LOST_WHITE_B  80

#define COMMS_LOST_BREATHE_PERIOD_MS   1200  // 한 사이클(밝음→어두움→밝음) 시간
#define COMMS_LOST_MIN_BRIGHTNESS_PCT  20    // 가장 어두울 때 밝기 (%)
#define COMMS_LOST_UPDATE_INTERVAL_MS  30    // 애니메이션 갱신 주기 (30ms ≈ 33fps)

static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 마지막으로 계산된 CO2 기준 색상 (정상 상태 고정 표시용)
static uint8_t base_r = 0, base_g = 0, base_b = 0;
static bool has_co2_color = false;   // 아직 한 번도 유효한 CO2 값을 못 받았으면 false

// 상태 플래그
static bool comms_lost_state = false;
static bool sensor_error_state = false;  // 센서 자체 read 실패 — 흰색 고정, 최우선

// 숨쉬기 애니메이션 타이머
static unsigned long breathe_start_time = 0;  // 두절 시작 시각 — 이 시점부터 항상 "밝은 상태"로 시작
static unsigned long last_update_time = 0;

void led_init() {
    strip.begin();
    strip.clear();
    strip.show();
    Serial.println("LED initialized");
}

// CO2 값에 따른 기준 색상을 계산해 저장. 통신 정상이면 즉시 고정 표시.
void led_set_co2(uint16_t co2) {
    sensor_error_state = false;  // 정상 read 성공 → 에러 표시 해제

    if (co2 <= CO2_GREEN) {
        base_r = 0;   base_g = 100; base_b = 0;   // 초록
    } else if (co2 <= CO2_YELLOW) {
        base_r = 50;  base_g = 50;  base_b = 0;   // 노랑
    } else {
        base_r = 100; base_g = 0;   base_b = 0;   // 빨강
    }
    has_co2_color = true;

    if (!comms_lost_state) {
        strip.setPixelColor(0, strip.Color(base_r, base_g, base_b));
        strip.show();
    }
    // comms_lost_state가 true인 동안은 화면이 흰색 숨쉬기 표시 중이므로 여기서 덮어쓰지 않는다.
    // (기준 CO2 색 자체는 계속 갱신해둬서, 통신 복구 시 최신 색으로 즉시 전환되도록 함)
}

// 센서 read 실패 — 통신 상태와 무관하게 항상 흰색 "고정" (하드웨어 이상은 최우선으로 알려야 함)
void led_set_error_sensor() {
    sensor_error_state = true;
    strip.setPixelColor(0, strip.Color(50, 50, 50));
    strip.show();
}

// 통신 두절 여부를 외부(loop)에서 매 iteration 갱신해준다.
void led_set_comms_lost(bool lost) {
    if (lost == comms_lost_state) return;  // 상태 변화 없으면 아무 것도 하지 않음
    comms_lost_state = lost;

    if (lost) {
        breathe_start_time = millis();  // 항상 "가장 밝은 지점"부터 애니메이션 시작
        last_update_time = 0;           // 다음 loop에서 즉시 첫 프레임 반영
        Serial.println("[LED] Comms lost — white breathing started");
    } else {
        if (!sensor_error_state && has_co2_color) {
            strip.setPixelColor(0, strip.Color(base_r, base_g, base_b));  // CO2 색 고정으로 복귀
            strip.show();
        }
        Serial.println("[LED] Comms restored — solid CO2 color");
    }
}

// 0~period 구간을 삼각파(밝음→어두움→밝음)로 매핑해 밝기 비율(%)을 반환
static uint8_t compute_breathe_brightness_pct(unsigned long elapsed) {
    unsigned long t = elapsed % COMMS_LOST_BREATHE_PERIOD_MS;
    unsigned long half = COMMS_LOST_BREATHE_PERIOD_MS / 2;

    float phase; // 0.0(가장 어두움) ~ 1.0(가장 밝음)
    if (t < half) {
        phase = 1.0f - ((float)t / half);          // 밝음 → 어두움
    } else {
        phase = (float)(t - half) / half;          // 어두움 → 밝음
    }

    return (uint8_t)(COMMS_LOST_MIN_BRIGHTNESS_PCT +
                      phase * (100 - COMMS_LOST_MIN_BRIGHTNESS_PCT));
}

// 매 loop마다 호출 — non-blocking 애니메이션 처리.
// 우선순위: 센서 read 에러(흰색 고정) > 통신두절(흰색 숨쉬기) > 정상(CO2색 고정, led_set_co2에서 이미 처리됨)
void led_update() {
    if (sensor_error_state) return;   // 흰색 고정 유지, 애니메이션 없음
    if (!comms_lost_state) return;    // 정상 상태는 led_set_co2()에서 이미 고정 표시 완료

    if (millis() - last_update_time < COMMS_LOST_UPDATE_INTERVAL_MS) return;
    last_update_time = millis();

    uint8_t brightness_pct = compute_breathe_brightness_pct(millis() - breathe_start_time);

    uint8_t r = (uint16_t)COMMS_LOST_WHITE_R * brightness_pct / 100;
    uint8_t g = (uint16_t)COMMS_LOST_WHITE_G * brightness_pct / 100;
    uint8_t b = (uint16_t)COMMS_LOST_WHITE_B * brightness_pct / 100;

    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}