#include "led.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN     5
#define LED_COUNT   1

#define CO2_GREEN   800
#define CO2_YELLOW  1000

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
        strip.setPixelColor(0, strip.Color(50, 50, 0));    // GRB: 노랑
    } else {
        strip.setPixelColor(0, strip.Color(100, 0, 0));    // GRB: 빨강
    }
    strip.show();
}

void led_set_error_sensor() {
    strip.setPixelColor(0, strip.Color(50, 50, 50));  // GRB: 흰색
    strip.show();
}