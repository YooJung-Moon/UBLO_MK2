// led_test.ino
#include <Adafruit_NeoPixel.h>

#define LED_PIN    D2
#define LED_COUNT  1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    delay(3000);

    strip.begin();
    strip.clear();
    strip.show();
    Serial.println("LED test start");
}

void loop() {
    // 빨강
    Serial.println("RED");
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    delay(1000);

    // 초록
    Serial.println("GREEN");
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
    delay(1000);

    // 파랑
    Serial.println("BLUE");
    strip.setPixelColor(0, strip.Color(0, 0, 255));
    strip.show();
    delay(1000);

    // 노랑
    Serial.println("YELLOW");
    strip.setPixelColor(0, strip.Color(255, 165, 0));
    strip.show();
    delay(1000);

    // 끔
    Serial.println("OFF");
    strip.clear();
    strip.show();
    delay(1000);
}