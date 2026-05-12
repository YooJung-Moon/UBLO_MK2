# UBLO S MKII Firmware

## 1. 프로젝트 개요

UBLO S MKII는 헬싱키 임대 아파트 공동공간을 위한 스마트 환기 시스템이다.

실내 CO₂ 농도를 모니터링하여 환기 커버와 blower fan을 자동으로 제어한다.

두 개의 Arduino Nano ESP32가 ESP-NOW로 통신하는 듀얼 MCU 구조로 동작한다.

- **Sensor MCU** — SCD41에서 10초마다 CO₂, 온도, 습도를 읽고 10분 평균값을 기반으로 AUTO 모드 판단을 수행한다. RTC timestamp와 함께 센서 데이터 및 판단 결과를 SD card에 로깅한다.
- **Fan MCU** — AUTO 모드에서는 Sensor MCU의 명령에 따라, MANUAL 모드에서는 사용자 다이얼 입력에 따라 커버 모터와 fan을 제어한다.

**모드 정의:**

| 모드 | 커버 | Fan |
|------|------|-----|
| AUTO (0) | CO₂ 기반 자동 제어 | CO₂ 기반 자동 제어 |
| CLOSED (1) | CLOSE | OFF |
| BREEZE (2) | OPEN | OFF |
| TURBO (3) | OPEN | ON |

---

## 2. 하드웨어 구성

| 부품 | 설명 |
|------|------|
| Arduino Nano ESP32 × 2 | Sensor MCU, Fan MCU |
| SCD41 | CO₂, 온도, 습도 센서 |
| SZH-EK024 | 온도, 습도 센서 |
| DS3231 | RTC 모듈 |
| DRV8871 | 모터 드라이버 |
| Micro SD Socket | SD card 로깅 |
| Limit switch × 2 | 커버 개폐 감지 |

**Fan MCU 핀 매핑:**

| 핀 | 기능 |
|----|------|
| D2 | MOTOR_IN2 |
| D3 | MOTOR_IN1 |
| D5 | LIM_CLOSE |
| D6 | LIM_OPEN |
| D9 | FAN_PWM |

**Sensor MCU 핀 매핑:**

| 핀 | 기능 |
|----|------|
| A4 | SDA (SCD41, DS3231) |
| A5 | SCL (SCD41, DS3231) |
| D10 | SD card SS |
| D11 | SD card MOSI |
| D12 | SD card MISO |
| D13 | SD card SCK |

---

## 3. 디렉토리 구조

```
ublo-mkii/
└── firmware/
    ├── sensor_mcu/
    │   ├── sensor_mcu.ino      // 진입점
    │   ├── config.h            // 설정값
    │   ├── sensors.h/cpp       // SCD41 읽기
    │   ├── buffer.h/cpp        // 측정값 버퍼 및 평균 연산
    │   ├── logic.h/cpp         // AUTO 판단 로직
    │   ├── rtc.h               // RTC 인터페이스
    │   ├── rtc_hw.cpp          // DS3231 구현체
    │   ├── sdcard.h            // SD card 인터페이스
    │   ├── sdcard_hw.cpp       // SD card 구현체
    │   └── comms.h/cpp         // ESP-NOW 송수신
    │
    └── fan_mcu/
        ├── fan_mcu.ino         // 진입점
        ├── config.h            // 설정값
        ├── actuators.h/cpp     // Fan PWM, 커버 모터, limit switch
        ├── logic.h/cpp         // MANUAL 판단 로직, 타임아웃
        ├── encoder.h           // 다이얼 인터페이스
        ├── encoder_hw.cpp      // Rotary encoder 구현체
        ├── led.h               // LED 인터페이스
        ├── led_hw.cpp          // 외부 LED 구현체
        └── comms.h/cpp         // ESP-NOW 송수신
```

---

## 4. 의존성

**보드 패키지:**
- Tools → Boards Manager → **Arduino ESP32 Boards** by Arduino (최신 버전)

**라이브러리:**
- `Sensirion I2C SCD4x` — SCD41 CO₂ 센서
- `Adafruit RTClib` — DS3231 RTC

---

## 5. 실행 방법

**1. MAC 주소 설정**

각 보드의 MAC 주소를 확인하여 `config.h`에 등록해야 한다.

아래 코드를 각 보드에 업로드하여 MAC 주소를 확인한다:
```cpp
#include <WiFi.h>
void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.macAddress());
}
void loop() {}
```

확인 후 `config.h`에 업데이트:
```cpp
// sensor_mcu/config.h
const uint8_t FAN_MCU_MAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

// fan_mcu/config.h
const uint8_t SENSOR_MCU_MAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

**2. 업로드 순서**
1. `fan_mcu.ino`를 Fan MCU에 먼저 업로드
2. `sensor_mcu.ino`를 Sensor MCU에 업로드

---

## 6. 통신 프로토콜

**패킷 구조:**
```cpp
// Sensor MCU → Fan MCU (AUTO 모드, 10분 주기)
typedef struct {
    uint8_t fan_cmd;    // 0=OFF, 1=ON
    uint8_t cover_cmd;  // 0=CLOSE, 1=OPEN
} command_packet_t;

// Fan MCU → Sensor MCU (항상, 판단 후)
typedef struct {
    uint8_t mode;       // 0=AUTO, 1=CLOSED, 2=BREEZE, 3=TURBO
    uint8_t fan_cmd;
    uint8_t cover_cmd;
} mode_packet_t;
```

**동작 흐름:**

| 상황 | Sensor MCU | Fan MCU |
|------|------------|---------|
| 10초 주기 | sensor 읽기 → raw data logging → 버퍼에 측정값 추가 | — |
| 10분 주기 | 평균 CO₂ 계산 → AUTO 판단 → command_packet 전송 → mode_packet 수신 → decision logging | command_packet 수신 → AUTO/MANUAL 판단 → 액추에이터 실행 → mode_packet 전송 |
| 다이얼 조작 시 | — | 모드 전환 → MANUAL 진입 시각 기록 |
| MANUAL 타임아웃 | — | AUTO 복귀 → mode_packet 전송 |

---

## 7. 설정값

**sensor_mcu/config.h**

| 항목 | 기본값 | 설명 |
|------|--------|------|
| CO2_THRESHOLD | 800 | 환기 시작 CO₂ 기준값 (ppm) |
| CO2_DANGER | 2000 | CO₂ 위험 수준 (ppm) |
| BUFFER_SIZE | 60 | 평균 연산에 사용할 측정값 수 (10초 × 60 = 10분) |
| SENSOR_INTERVAL | 10000 | 센서 읽기 주기 (ms) |

**fan_mcu/config.h**

| 항목 | 기본값 | 설명 |
|------|--------|------|
| TIMEOUT_BREEZE | 14400000 | BREEZE 모드 타임아웃 (4시간) |
| TIMEOUT_TURBO | 3600000 | TURBO 모드 타임아웃 (1시간) |
