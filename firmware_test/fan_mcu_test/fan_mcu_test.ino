#include "config.h"
#include "comms.h"
#include "actuators.h"

void setup() {
    Serial.begin(115200);
    actuators_init();
    comms_init();
    Serial.println("Fan MCU test ready");
}

void loop() {
    if (comms_command_available()) {
        command_packet_t cmd = comms_get_last_command();

        Serial.print("Command received — fan: ");
        Serial.print(cmd.fan_cmd);
        Serial.print(" | cover: ");
        Serial.println(cmd.cover_cmd);

        fan_set(cmd.fan_cmd);
        cover_set(cmd.cover_cmd);
    }
}