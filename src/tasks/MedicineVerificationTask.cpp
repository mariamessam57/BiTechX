#include "MedicineVerificationTask.h"

MedicineVerificationTask::MedicineVerificationTask(SensorManager& sensorManager)
    : sensorManager(sensorManager),
      safetyManager(nullptr),
      verifyStartTime(0),
      lastAttemptMs(0),
      confirmCount(0) {
}

void MedicineVerificationTask::setSafetyManager(SafetyManager* manager) {
    safetyManager = manager;
}

void MedicineVerificationTask::start() {
    verifyStartTime = millis();
    lastAttemptMs = 0;
    confirmCount = 0;
}

MedicineVerificationTask::Result MedicineVerificationTask::update() {
    const uint32_t now = millis();

    if ((now - verifyStartTime) >= (MEDICINE_VERIFY_WAIT_MS + MEDICINE_VERIFY_TIMEOUT_MS)) {
        Serial.printf("[MedicineVerificationTask] IR verification timeout after %lu ms\n",
                      static_cast<unsigned long>(MEDICINE_VERIFY_WAIT_MS + MEDICINE_VERIFY_TIMEOUT_MS));
        if (safetyManager) {
            safetyManager->triggerFault(SafetyFaultCode::SAFETY_INVALID_SENSOR, "Medicine verification timeout");
        }
        return Result::TIMEOUT;
    }

    if ((now - verifyStartTime) < MEDICINE_VERIFY_WAIT_MS) {
        return Result::PENDING;
    }

    if ((now - lastAttemptMs) < MEDICINE_VERIFY_RETRY_INTERVAL_MS) {
        return Result::PENDING;
    }

    lastAttemptMs = now;

    if (sensorManager.isMedicineDetected()) {
        confirmCount++;
        Serial.printf("[MedicineVerificationTask] IR confirmation %u/%u\n",
                      confirmCount,
                      MEDICINE_DETECTION_REQUIRED_READINGS);

        if (confirmCount >= MEDICINE_DETECTION_REQUIRED_READINGS) {
            Serial.println("[MedicineVerificationTask] MEDICINE_DETECTED confirmed");
            return Result::CONFIRMED;
        }
    } else {
        confirmCount = 0;
        if (safetyManager && (now - verifyStartTime) >= MEDICINE_VERIFY_WAIT_MS) {
            // Intermittent blanks are treated as non-confirmation without forcing a false-positive success.
        }
    }

    return Result::PENDING;
}
