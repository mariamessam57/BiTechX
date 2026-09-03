#include "SafetyManager.h"
#include "config.h"

SafetyManager::SafetyManager()
    : servoManager(nullptr),
      doorManager(nullptr),
      motorManager(nullptr),
      faultCode(SafetyFaultCode::SAFETY_NONE) {
    faultReason[0] = '\0';
}

void SafetyManager::begin() {
    clearFault();
}

void SafetyManager::bind(ServoManager* servo, DoorManager* door, MotorManager* motor) {
    servoManager = servo;
    doorManager = door;
    motorManager = motor;
}

bool SafetyManager::isFaultActive() const {
    return faultCode != SafetyFaultCode::SAFETY_NONE;
}

SafetyFaultCode SafetyManager::getFaultCode() const {
    return faultCode;
}

const char* SafetyManager::getFaultReason() const {
    return faultReason;
}

void SafetyManager::triggerFault(SafetyFaultCode code, const char* reason) {
    faultCode = code;
    if (reason != nullptr) {
        snprintf(faultReason, sizeof(faultReason), "%s", reason);
    } else {
        faultReason[0] = '\0';
    }

    emergencyStop();
}

void SafetyManager::clearFault() {
    faultCode = SafetyFaultCode::SAFETY_NONE;
    faultReason[0] = '\0';
}

void SafetyManager::emergencyStop() {
    stopActuators();
}

void SafetyManager::stopActuators() {
    if (motorManager != nullptr) {
        motorManager->stop();
    }
    if (servoManager != nullptr) {
        servoManager->stop();
    }
    if (doorManager != nullptr) {
        doorManager->close();
        doorManager->relax();
    }
}

bool SafetyManager::check(bool rtcValid,
                         uint8_t requestedSection,
                         bool rotationComplete,
                         bool doorOpen,
                         bool motorRunning,
                         uint32_t motorRunMs,
                         uint32_t dispenserElapsedMs,
                         uint32_t doorElapsedMs,
                         bool invalidSensorRead) {
    if (faultCode != SafetyFaultCode::SAFETY_NONE) {
        return false;
    }

    if (!rtcValid) {
        triggerFault(SafetyFaultCode::SAFETY_INVALID_RTC, "Invalid RTC");
        return false;
    }

    if (requestedSection > 7) {
        triggerFault(SafetyFaultCode::SAFETY_INVALID_SECTION, "Invalid section index");
        return false;
    }

    if (invalidSensorRead) {
        triggerFault(SafetyFaultCode::SAFETY_INVALID_SENSOR, "Invalid sensor value");
        return false;
    }

    if (motorRunning && motorRunMs > DC_MOTOR_MAX_RUNTIME_MS) {
        triggerFault(SafetyFaultCode::SAFETY_MOTOR_TIMEOUT, "Motor runtime exceeded");
        return false;
    }

    if (dispenserElapsedMs > DISPENSER_OPERATION_TIMEOUT_MS) {
        triggerFault(SafetyFaultCode::SAFETY_DISPENSER_TIMEOUT, "Dispenser rotation timeout");
        return false;
    }

    if (doorElapsedMs > DOOR_OPERATION_TIMEOUT_MS) {
        triggerFault(SafetyFaultCode::SAFETY_DOOR_TIMEOUT, "Door operation timeout");
        return false;
    }

    if (!rotationComplete && doorOpen) {
        triggerFault(SafetyFaultCode::SAFETY_INVALID_STATE, "Door opened before rotation complete");
        return false;
    }

    if (motorRunning && !rotationComplete && !doorOpen) {
        triggerFault(SafetyFaultCode::SAFETY_INVALID_STATE, "Motor active during invalid phase");
        return false;
    }

    return true;
}
