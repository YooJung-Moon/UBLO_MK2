#pragma once

// ── Build Mode ───────────────────────────────
#define TEST_MODE   // 주석 처리하면 실제 모드로 전환

// ── Test Scenario ─────────────────────────────
#ifdef TEST_MODE
#define TEST_SCENARIO  1   // 1: COMFORT→WARNING→COMFORT
                           // 2: COMFORT→ALERT→COMFORT
                           // 3: Safe Mode
#endif

// ── MAC Address ──────────────────────────────
#define RECEIVER_MAC {0xE0, 0x72, 0xA1, 0xF8, 0xF6, 0x0C}

// ── Timers (ms) ──────────────────────────────
#define T_MEASURE       5000    // 테스트용 5s (실제: 30000)
#define T_SD_WRITE      5000    // 테스트용 5s (실제: 30000)

// ── Air Quality Thresholds ───────────────────
#define CO2_WARNING          1000.0f
#define CO2_ALERT            1500.0f
#define CO2_WARNING_HYST      950.0f
#define CO2_ALERT_HYST       1450.0f

#define TEMP_LOW              18.0f
#define TEMP_HIGH             26.0f
#define TEMP_LOW_HYST         19.0f
#define TEMP_HIGH_HYST        25.0f

#define HUMID_LOW             40.0f
#define HUMID_HIGH            70.0f
#define HUMID_LOW_HYST        45.0f
#define HUMID_HIGH_HYST       65.0f

#define CONSEC_THRESHOLD      3

// ── Battery Thresholds ───────────────────────
#define BATTERY_NORMAL_PCT    60
#define BATTERY_PSAVE_PCT     20

// ── ESP-NOW ──────────────────────────────────
#define ESPNOW_MAX_RETRY      3
#define ESPNOW_CHANNEL        1

// ── SD Card SPI Pins ─────────────────────────
#define PIN_SD_CS    5
#define PIN_SD_MOSI  9
#define PIN_SD_MISO  8
#define PIN_SD_SCK   7

// ── SD Card ──────────────────────────────────
#define SD_FILENAME  "/ublo_log.csv"

// ── Test Mode Default Values ─────────────────
#ifdef TEST_MODE
#define TEST_CO2_DEFAULT      400.0f
#define TEST_TEMP_DEFAULT     22.0f
#define TEST_HUMID_DEFAULT    55.0f
#define TEST_BAT_DEFAULT      80
#endif