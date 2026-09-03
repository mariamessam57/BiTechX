#include "TelemetryManager.h"
#include "config.h"

TelemetryManager::TelemetryManager(MQTTManager& mqttManager)
    : mqtt(mqttManager)
{
}

void TelemetryManager::begin()
{
    Serial.println("[TelemetryManager] Telemetry initialized");
}

bool TelemetryManager::sendDoseTaken(
    const char* medicine,
    int section,
    const char* scheduledTime,
    const char* timestamp)
{
    return publishEvent(
        "dose_taken",
        "taken",
        medicine,
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::sendDoseDelayed(
    const char* medicine,
    int section,
    const char* scheduledTime,
    const char* timestamp)
{
    return publishEvent(
        "dose_delayed",
        "delayed",
        medicine,
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::sendDoseMissed(
    const char* medicine,
    int section,
    const char* scheduledTime,
    const char* timestamp)
{
    return publishEvent(
        "dose_missed",
        "missed",
        medicine,
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::publishEvent(
    const char* event,
    const char* status,
    const char* medicine,
    int section,
    const char* scheduledTime,
    const char* timestamp)
{
    if (!mqtt.isConnected())
    {
        Serial.println("[TelemetryManager] MQTT not connected");
        return false;
    }

    JsonDocument doc;

    doc["device_id"] = "BiTechX_001";
    doc["event"] = event;
    doc["medicine"] = medicine;
    doc["section"] = section;
    doc["scheduled_time"] = scheduledTime;
    doc["timestamp"] = timestamp;
    doc["status"] = status;

    char payload[512];

    serializeJson(doc, payload, sizeof(payload));

    Serial.println("[TelemetryManager] Sending telemetry:");
    Serial.println(payload);

    return mqtt.publish(MQTT_TOPIC_TELEMETRY, payload);
}