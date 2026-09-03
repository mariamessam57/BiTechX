#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "MQTTManager.h"

class TelemetryManager
{
public:
    TelemetryManager(MQTTManager& mqttManager);

    void begin();

    bool sendDoseTaken(
        const char* medicine,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

    bool sendDoseDelayed(
        const char* medicine,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

    bool sendDoseMissed(
        const char* medicine,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );

private:
    MQTTManager& mqtt;

    bool publishEvent(
        const char* event,
        const char* status,
        const char* medicine,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );
};