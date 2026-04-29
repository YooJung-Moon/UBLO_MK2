#include "sensors.h"

static SensirionI2CScd4x scd4x;

bool sensors_init() {
    Wire.begin();
    scd4x.begin(Wire);

    uint16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.println("SCD41 stop failed");
        return false;
    }

    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.println("SCD41 start failed");
        return false;
    }

    Serial.println("SCD41 initialized");
    return true;
}

bool sensors_read(float &co2, float &temperature, float &humidity) {
    bool isDataReady = false;
    uint16_t error = scd4x.getDataReadyFlag(isDataReady);
    if (error || !isDataReady) {
        return false;
    }

    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        Serial.println("SCD41 read failed");
        return false;
    }

    return true;
}