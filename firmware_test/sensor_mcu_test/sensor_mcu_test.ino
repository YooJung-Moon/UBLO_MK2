#include "config.h"
#include "comms.h"
#include "sensors.h"

static unsigned long last_sensor_time = 0;

void setup() {
    Serial.begin(115200);
    comms_init();
    if (!sensors_init()) {
        Serial.println("Sensor init failed, halting");
        while (true);
    }
    Serial.println("Sensor MCU test ready");
}

void loop() {
    unsigned long now = millis();
    if (now - last_sensor_time >= SENSOR_INTERVAL) {
        last_sensor_time = now;

        float co2, temperature, humidity;
        if (!sensors_read(co2, temperature, humidity)) {
            Serial.println("Sensor read failed, skipping");
            return;
        }

        Serial.print("CO2: "); Serial.print(co2);
        Serial.print(" | Temp: "); Serial.print(temperature);
        Serial.print(" | Humidity: "); Serial.println(humidity);

        command_packet_t packet;
        if (co2 >= CO2_GOOD) {
            packet.fan_cmd   = 1; // ON
            packet.cover_cmd = 1; // OPEN
            Serial.println("Decision: VENTILATE");
        } else {
            packet.fan_cmd   = 0; // OFF
            packet.cover_cmd = 0; // CLOSE
            Serial.println("Decision: IDLE");
        }

        comms_send(packet);
    }
}