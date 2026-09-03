#pragma once

#include <Arduino.h>
#include "DoorManager.h"
#include "MotorManager.h"
#include "ServoManager.h"

enum class SafetyFaultCode : uint8_t {
    SAFETY_NONE = 0,
    SAFETY_MOTOR_TIMEOUT,
    SAFETY_DISPENSER_TIMEOUT,
    SAFETY_DOOR_TIMEOUT,
    SAFETY_INVALID_RTC,
    SAFETY_INVALID_SECTION,
    SAFETY_INVALID_SENSOR,
    SAFETY_INVALID_STATE
};

class SafetyManager {
public:
    SafetyManager();

    void begin();
    void bind(ServoManager* servoManager, DoorManager* doorManager, MotorManager* motorManager = nullptr);

    bool isFaultActive() const;
    SafetyFaultCode getFaultCode() const;
    const char* getFaultReason() const;

    void triggerFault(SafetyFaultCode code, const char* reason);
    void clearFault();
    void emergencyStop();
    void stopActuators();

    bool check(bool rtcValid,
               uint8_t requestedSection,
               bool rotationComplete,
               bool doorOpen,
               bool motorRunning,
               uint32_t motorRunMs,
               uint32_t dispenserElapsedMs,
               uint32_t doorElapsedMs,
               bool invalidSensorRead);

private:
    ServoManager* servoManager;
    DoorManager* doorManager;
    MotorManager* motorManager;

    SafetyFaultCode faultCode;
    char faultReason[64];
};
