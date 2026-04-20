#pragma once

// ── Build Mode ───────────────────────────────
#define TEST_MODE   // 주석 처리하면 실제 모드로 전환

// ── MAC Address ──────────────────────────────
#define RECEIVER_MAC {0xE0, 0x72, 0xA1, 0xF8, 0xF6, 0x0C}  // Fan MCU MAC

// ── Timers (ms) ──────────────────────────────
#define T_MEASURE       5000   // 측정 주기 30s
#define T_SD_WRITE      5000   // SD 카드 저장 주기 30s (T_MEASURE와 동기화)

// ── Air Quality Thresholds ───────────────────
#define CO2_WARNING          1000.0f  // CO₂ WARNING 상향 기준 (ppm)
#define CO2_ALERT            1500.0f  // CO₂ ALERT 상향 기준 (ppm)
#define CO2_WARNING_HYST      950.0f  // CO₂ WARNING 하향 기준 (ppm)
#define CO2_ALERT_HYST       1450.0f  // CO₂ ALERT 하향 기준 (ppm)

#define TEMP_LOW              18.0f   // 온도 하한 (°C)
#define TEMP_HIGH             26.0f   // 온도 상한 (°C)
#define TEMP_LOW_HYST         19.0f   // 온도 하한 hysteresis (°C)
#define TEMP_HIGH_HYST        25.0f   // 온도 상한 hysteresis (°C)

#define HUMID_LOW             40.0f   // 습도 하한 (%)
#define HUMID_HIGH            70.0f   // 습도 상한 (%)
#define HUMID_LOW_HYST        45.0f   // 습도 하한 hysteresis (%)
#define HUMID_HIGH_HYST       65.0f   // 습도 상한 hysteresis (%)

#define CONSEC_THRESHOLD      3       // 상태 전이 연속 횟수

// ── Battery Thresholds ───────────────────────
#define BATTERY_NORMAL_PCT    60      // NORMAL 기준 (%)
#define BATTERY_PSAVE_PCT     20      // POWER_SAVE 기준 (%)

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_MAX_RETRY      3       // 최대 재전송 횟수
#define ESPNOW_CHANNEL        1       // ESP-NOW 채널

// ── SD Card SPI Pins ─────────────────────────
// 실제 모듈 연결 후 핀 번호 업데이트 필요
#define PIN_SD_CS    5   // Chip Select
#define PIN_SD_MOSI  9   // MOSI
#define PIN_SD_MISO  8   // MISO
#define PIN_SD_SCK   7   // SCK

// ── SD Card ──────────────────────────────────
#define SD_FILENAME  "/ublo_log.csv"

// ── Test Mode Default Values ─────────────────
#ifdef TEST_MODE
#define TEST_CO2_DEFAULT      400.0f  // 초기 CO₂ (ppm)
#define TEST_TEMP_DEFAULT     22.0f   // 초기 온도 (°C)
#define TEST_HUMID_DEFAULT    55.0f   // 초기 습도 (%)
#define TEST_BAT_DEFAULT      80      // 초기 배터리 (%)
#endif