#pragma once

#include <Arduino.h>
#include "config.h"
#include "SensorManager.h"
#include "SafetyManager.h"

class PersonDetectionTask {
public:
    enum class Result {
        PENDING,
        CONFIRMED,
        TIMEOUT
    };

    explicit PersonDetectionTask(SensorManager& sensorManager);
    void setSafetyManager(SafetyManager* safetyManager);

    void start();
    Result update();

private:
    SensorManager& sensorManager;
    SafetyManager* safetyManager;
    uint32_t detectionStartTime;
    uint32_t lastAttemptMs;
    uint8_t confirmCount;
};
