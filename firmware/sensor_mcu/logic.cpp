#include "logic.h"

command_packet_t logic_decide(uint16_t avg_co2) {
    command_packet_t cmd;

    if (avg_co2 >= CO2_THRESHOLD) {
        cmd.fan_cmd   = 1; // ON
        cmd.cover_cmd = 1; // OPEN
    } else {
        cmd.fan_cmd   = 0; // OFF
        cmd.cover_cmd = 0; // CLOSE
    }

    return cmd;
}