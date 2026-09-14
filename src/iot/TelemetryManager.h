#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "MQTTManager.h"

class TelemetryManager
{
public:
    TelemetryManager(MQTTManager& mqttManager);

    void begin();

    bool sendStatus(
        const char* status
    );

    bool sendDeviceState(
        const char* state,
        int activeSection = -1
    );

    bool sendAlert(
        const char* message
    );

    bool sendSensorData(
        bool personDetected,
        float distanceCm,
        bool medicineDetected
    );

    bool sendSystemTelemetry(
        const char* deviceStatus,
        const char* connectionStatus,
        const char* currentState,
        const char* medicationName,
        const char* nextDoseTime,
        const char* doseStatus,
        const char* doorStatus,
        const char* conveyorStatus,
        bool personDetected,
        float distanceCm,
        bool medicineDetected
    );

    bool sendDoseTaken(
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

    bool sendDoseDelayed(
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

    bool sendDoseMissed(
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

    bool sendEvent(
        const char* event,
        const char* status,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

private:
    MQTTManager& mqtt;

    bool publishJson(
        JsonDocument& doc
    );
};