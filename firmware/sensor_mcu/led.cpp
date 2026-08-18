#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN     5
#define LED_COUNT   1

#define CO2_GREEN   800
#define CO2_YELLOW  1000

// 통신 두절 시 CO2색 느린 깜빡임 주기 (ms) — CO2 표시 기능 자체는 유지하면서 통신 이상만 별도 알림
#define COMMS_LOST_BLINK_MS 1500

static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 마지막으로 계산된 CO2 기반 색상 — comms_lost 깜빡임 시 이 색을 그대로 재사용
static uint32_t last_co2_color = 0;
static bool has_co2_color = false;   // 아직 한 번도 유효한 CO2 값을 못 받았으면 false

// 상태 플래그
static bool comms_lost_state = false;
static bool sensor_error_state = false;  // 센서 자체 read 실패 — 흰색 고정, 최우선

// 깜빡임 타이머
static bool blink_on = true;
static unsigned long last_blink_time = 0;

void led_init() {
    strip.begin();
    strip.clear();
    strip.show();
    Serial.println("LED initialized");
}

// CO2 값에 따른 색상을 계산해 저장. 통신 정상이면 즉시 고정 표시하고,
// 통신 두절 상태라면 저장만 해두고 led_update()의 깜빡임 사이클이 반영한다.
void led_set_co2(uint16_t co2) {
    sensor_error_state = false;  // 정상 read 성공 → 에러 표시 해제

    if (co2 <= CO2_GREEN) {
        last_co2_color = strip.Color(0, 100, 0);    // GRB: 초록
    } else if (co2 <= CO2_YELLOW) {
        last_co2_color = strip.Color(50, 50, 0);    // GRB: 노랑
    } else {
        last_co2_color = strip.Color(100, 0, 0);    // GRB: 빨강
    }
    has_co2_color = true;

    if (!comms_lost_state) {
        strip.setPixelColor(0, last_co2_color);
        strip.show();
    }
    // comms_lost_state가 true인 동안은 여기서 강제로 켜지 않는다 — led_update()가 다음
    // 타이머 주기에 알아서 반영하므로, 깜빡임 사이클과 어긋나는 즉시 표시를 막기 위함.
}

// 센서 read 실패 — 통신 상태와 무관하게 항상 흰색 고정 (하드웨어 이상은 최우선으로 알려야 함)
void led_set_error_sensor() {
    sensor_error_state = true;
    strip.setPixelColor(0, strip.Color(50, 50, 50));  // GRB: 흰색
    strip.show();
}

// 통신 두절 여부를 외부(loop)에서 매 iteration 갱신해준다.
void led_set_comms_lost(bool lost) {
    if (lost == comms_lost_state) return;  // 상태 변화 없으면 아무 것도 하지 않음
    comms_lost_state = lost;

    if (lost) {
        blink_on = true;
        last_blink_time = millis();
        if (!sensor_error_state && has_co2_color) {
            strip.setPixelColor(0, last_co2_color);  // 켜진 상태로 깜빡임 시작
            strip.show();
        }
        Serial.println("[LED] Comms lost — CO2 color slow blink started");
    } else {
        if (!sensor_error_state && has_co2_color) {
            strip.setPixelColor(0, last_co2_color);  // 두절 해제 → CO2색 고정 표시로 복귀
            strip.show();
        }
        Serial.println("[LED] Comms restored — solid display");
    }
}

// 매 loop마다 호출 — non-blocking 깜빡임 처리.
// 우선순위: 센서 read 에러(흰색 고정) > 통신두절(CO2색 느린 깜빡임) > 정상(CO2색 고정, led_set_co2에서 이미 처리됨)
void led_update() {
    if (sensor_error_state) return;   // 흰색 고정 유지, 깜빡이지 않음
    if (!comms_lost_state) return;    // 정상 상태는 led_set_co2()에서 이미 고정 표시 완료
    if (!has_co2_color) return;       // 아직 CO2 값을 한 번도 못 받았으면 표시할 색이 없음

    if (millis() - last_blink_time >= COMMS_LOST_BLINK_MS) {
        last_blink_time = millis();
        blink_on = !blink_on;
        if (blink_on) {
            strip.setPixelColor(0, last_co2_color);
        } else {
            strip.clear();
        }
        strip.show();
    }
}