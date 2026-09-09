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

private:
    MQTTManager& mqtt;

    bool publishEvent(
        const char* event,
        const char* status,
        int section,
        const char* scheduledTime,
        const char* timestamp
    );
};