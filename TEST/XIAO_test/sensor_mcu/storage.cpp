#include "storage.h"

static bool sdAvailable = false;

bool storageInit() {
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("SD card init failed");
    sdAvailable = false;
    return false;
  }

  sdAvailable = true;
  Serial.println("SD card initialized");

  // 헤더가 없으면 CSV 헤더 작성
  if (!SD.exists(SD_FILENAME)) {
    File file = SD.open(SD_FILENAME, FILE_WRITE);
    if (file) {
      file.println("timestamp,co2_ppm,temperature,humidity,battery_pct");
      file.close();
      Serial.println("CSV header written");
    } else {
      Serial.println("Failed to create CSV file");
      return false;
    }
  }

  return true;
}

bool storageWrite(float co2_ppm, float temperature, float humidity,
                  uint8_t battery_pct, uint32_t timestamp) {
  if (!sdAvailable) {
    Serial.println("SD not available");
    return false;
  }

  File file = SD.open(SD_FILENAME, FILE_APPEND);
  if (!file) {
    Serial.println("SD open failed");
    sdAvailable = false;
    return false;
  }

  // CSV 한 줄 작성
  // 예: 1713200000,1023.45,22.30,55.20,78
  file.print(timestamp);
  file.print(",");
  file.print(co2_ppm, 2);
  file.print(",");
  file.print(temperature, 2);
  file.print(",");
  file.print(humidity, 2);
  file.print(",");
  file.println(battery_pct);

  file.close();
  return true;
}

bool storageIsAvailable() {
  return sdAvailable;
}