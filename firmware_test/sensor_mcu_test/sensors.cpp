#include "sensors.h"

static SensirionI2cScd4x scd4x;

bool sensors_init() {
    Wire.begin();
    scd4x.begin(Wire, SCD41_I2C_ADDR_62);

    uint16_t error;
    char errorMessage[256];

    error = scd4x.stopPeriodicMeasurement();
    delay(500);

    error = scd4x.startPeriodicMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        Serial.print("SCD41 start failed: ");
        Serial.println(errorMessage);
        return false;
    }

    Serial.println("SCD41 initialized");
    return true;
}

bool sensors_read(uint16_t &co2, float &temperature, float &humidity) {
    uint16_t error;
    char errorMessage[256];

    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        errorToString(error, errorMessage, 256);
        Serial.print("SCD41 read failed: ");
        Serial.println(errorMessage);
        return false;
    }

    if (co2 == 0) {
        return false;
    }

    return true;
}