#include "buffer.h"

static uint16_t buffer[BUFFER_SIZE];
static int count = 0;
static int index = 0;

void buffer_init() {
    count = 0;
    index = 0;
    memset(buffer, 0, sizeof(buffer));
}

void buffer_add(float co2) {
    buffer[index] = co2;
    index = (index + 1) % BUFFER_SIZE;
    if (count < BUFFER_SIZE) count++;
}

bool buffer_full() {
    return count >= BUFFER_SIZE;
}

uint16_t buffer_average() {
    uint32_t sum = 0;  // 오버플로우 방지
    for (int i = 0; i < count; i++) {
        sum += buffer[i];
    }
    return (uint16_t)(sum / count);
}