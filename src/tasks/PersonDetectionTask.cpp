#include "PersonDetectionTask.h"

PersonDetectionTask::PersonDetectionTask(SensorManager& sensorManager)
    : sensorManager(sensorManager),
      safetyManager(nullptr),
      detectionStartTime(0),
      lastAttemptMs(0),
      confirmCount(0) {
}

void PersonDetectionTask::setSafetyManager(SafetyManager* manager) {
    safetyManager = manager;
}

void PersonDetectionTask::start() {
    detectionStartTime = millis();
    lastAttemptMs = 0;
    confirmCount = 0;
}

PersonDetectionTask::Result PersonDetectionTask::update() {
    const uint32_t now = millis();

    if ((now - detectionStartTime) >= PERSON_DETECTION_TIMEOUT_MS) {
        Serial.println("[PersonDetectionTask] Detection timeout; returning to idle");
        return Result::TIMEOUT;
    }

    if ((now - lastAttemptMs) < PERSON_DETECTION_RETRY_INTERVAL_MS) {
        return Result::PENDING;
    }

    lastAttemptMs = now;
    const float distance = sensorManager.readUltrasonicDistance();

    if (distance <= 0.0f || distance == -1.0f) {
        Serial.println("[PersonDetectionTask] Invalid ultrasonic reading during person detection");
        if (safetyManager) {
            safetyManager->triggerFault(SafetyFaultCode::SAFETY_INVALID_SENSOR, "Invalid ultrasonic reading");
        }
        confirmCount = 0;
        return Result::PENDING;
    }

    if (distance <= PERSON_DETECTION_DISTANCE_CM) {
        confirmCount++;
        Serial.printf("[PersonDetectionTask] Person detection confirmation %u/%u at %.2f cm\n",
                      confirmCount,
                      PERSON_DETECTION_REQUIRED_READINGS,
                      distance);

        if (confirmCount >= PERSON_DETECTION_REQUIRED_READINGS) {
            return Result::CONFIRMED;
        }
    } else {
        Serial.printf("[PersonDetectionTask] Distance %.2f cm is outside threshold %.1f cm\n",
                      distance,
                      PERSON_DETECTION_DISTANCE_CM);
        confirmCount = 0;
    }

    return Result::PENDING;
}
