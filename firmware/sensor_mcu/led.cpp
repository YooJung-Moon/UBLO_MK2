#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN     5
#define LED_COUNT   1

#define CO2_GREEN   800  // 800 이하 → 초록
#define CO2_YELLOW  1000  // 800~1000 → 노랑, 800 이상 → 빨강

static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void led_init() {
    strip.begin();
    strip.clear();
    strip.show();
    Serial.println("LED initialized");
}

void led_set_co2(uint16_t co2) {
    if (co2 <= CO2_GREEN) {
        strip.setPixelColor(0, strip.Color(0, 100, 0));    // GRB: 초록
    } else if (co2 <= CO2_YELLOW) {
        strip.setPixelColor(0, strip.Color(60, 50, 0));  // GRB: 노랑
    } else {
        strip.setPixelColor(0, strip.Color(100, 0, 0));    // GRB: 빨강
    }
    strip.show();
}   