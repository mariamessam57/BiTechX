#include "TelemetryManager.h"
#include "config.h"

TelemetryManager::TelemetryManager(
    MQTTManager& mqttManager
)
    : mqtt(mqttManager)
{
}

void TelemetryManager::begin()
{
    Serial.println(
        "[TelemetryManager] ThingsBoard Telemetry initialized"
    );
}

bool TelemetryManager::publishJson(
    JsonDocument& doc
)
{
    if (!mqtt.isConnected())
    {
        Serial.println(
            "[TelemetryManager] MQTT not connected"
        );

        return false;
    }

    char payload[512];

    size_t written = serializeJson(
        doc,
        payload,
        sizeof(payload)
    );

    if (written == 0)
    {
        Serial.println(
            "[TelemetryManager] Failed to serialize telemetry"
        );

        return false;
    }

    Serial.print(
        "[TelemetryManager] Telemetry: "
    );

    Serial.println(payload);

    return mqtt.publish(
        MQTT_TOPIC_TELEMETRY,
        payload
    );
}

bool TelemetryManager::sendStatus(
    const char* status
)
{
    JsonDocument doc;

    doc["device_status"] = status;

    return publishJson(doc);
}

bool TelemetryManager::sendDeviceState(
    const char* state,
    int activeSection
)
{
    JsonDocument doc;

    doc["device_status"] = state;
    doc["current_state"] = state;

    if (activeSection >= 0)
    {
        doc["active_section"] = activeSection;
    }

    return publishJson(doc);
}

bool TelemetryManager::sendAlert(
    const char* message
)
{
    JsonDocument doc;

    doc["device_status"] = "Alerting";
    doc["alert_message"] = message;
    doc["has_alert"] = true;

    return publishJson(doc);
}

bool TelemetryManager::sendSensorData(
    bool personDetected,
    float distanceCm,
    bool medicineDetected
)
{
    JsonDocument doc;

    doc["person_detected"] = personDetected;
    doc["distance_cm"] = distanceCm;
    doc["medicine_detected"] = medicineDetected;

    return publishJson(doc);
}

bool TelemetryManager::sendSystemTelemetry(
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
)
{
    JsonDocument doc;

    doc["device_status"] = deviceStatus;
    doc["connection_status"] = connectionStatus;
    doc["current_state"] = currentState;

    doc["medication_name"] = medicationName;
    doc["next_dose_time"] = nextDoseTime;

    doc["dose_status"] = doseStatus;

    doc["door_status"] = doorStatus;
    doc["conveyor_status"] = conveyorStatus;

    doc["person_detected"] = personDetected;
    doc["distance_cm"] = distanceCm;
    doc["medicine_detected"] = medicineDetected;

    return publishJson(doc);
}

bool TelemetryManager::sendDoseTaken(
    int section,
    const char* scheduledTime,
    const char* timestamp
)
{
    return sendEvent(
        "dose_taken",
        "taken",
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::sendDoseDelayed(
    int section,
    const char* scheduledTime,
    const char* timestamp
)
{
    return sendEvent(
        "dose_delayed",
        "delayed",
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::sendDoseMissed(
    int section,
    const char* scheduledTime,
    const char* timestamp
)
{
    return sendEvent(
        "dose_missed",
        "missed",
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::sendEvent(
    const char* event,
    const char* status,
    int section,
    const char* scheduledTime,
    const char* timestamp
)
{
    JsonDocument doc;

    doc["last_event"] = event;
    doc["last_dose_status"] = status;
    doc["last_dose_section"] = section;
    doc["last_scheduled_time"] = scheduledTime;
    doc["last_dose_time"] = timestamp;

    return publishJson(doc);
}