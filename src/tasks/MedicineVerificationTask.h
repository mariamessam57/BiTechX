#pragma once

#include <Arduino.h>
#include "config.h"
#include "SensorManager.h"
#include "SafetyManager.h"

class MedicineVerificationTask {
public:
    enum class Result {
        PENDING,
        CONFIRMED,
        TIMEOUT
    };

    explicit MedicineVerificationTask(SensorManager& sensorManager);
    void setSafetyManager(SafetyManager* safetyManager);

    void start();
    Result update();

private:
    SensorManager& sensorManager;
    SafetyManager* safetyManager;
    uint32_t verifyStartTime;
    uint32_t lastAttemptMs;
    uint8_t confirmCount;
};
