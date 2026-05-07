#include "buffer.h"

static uint16_t buffer[BUFFER_SIZE];
static int count = 0;
static int buf_index = 0;  // index → buf_index

void buffer_init() {
    count = 0;
    buf_index = 0;
    memset(buffer, 0, sizeof(buffer));
}

void buffer_add(uint16_t co2) {
    buffer[buf_index] = co2;
    buf_index = (buf_index + 1) % BUFFER_SIZE;
    if (count < BUFFER_SIZE) count++;
}

bool buffer_full() {
    return count >= BUFFER_SIZE;
}

uint16_t buffer_average() {
    if (count == 0) return 0;
    uint32_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i];
    }
    return (uint16_t)(sum / count);
}