#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN     D2
#define LED_COUNT   1

#define CO2_GREEN   600  // 600 이하 → 초록
#define CO2_YELLOW  800  // 600~800 → 노랑, 800 이상 → 빨강

static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void led_init() {
    strip.begin();
    strip.clear();
    strip.show();
    Serial.println("LED initialized");
}

void led_set_co2(uint16_t co2) {
    if (co2 <= CO2_GREEN) {
        strip.setPixelColor(0, strip.Color(0, 255, 0));    // 초록
    } else if (co2 <= CO2_YELLOW) {
        strip.setPixelColor(0, strip.Color(255, 165, 0));  // 노랑
    } else {
        strip.setPixelColor(0, strip.Color(255, 0, 0));    // 빨강
    }
    strip.show();
}