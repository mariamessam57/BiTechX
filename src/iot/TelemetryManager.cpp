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

bool TelemetryManager::sendStatus(const char* status)
{
    if (!mqtt.isConnected())
    {
        Serial.println("[TelemetryManager] MQTT not connected");
        return false;
    }

    JsonDocument doc;

    doc["device_id"] = MQTT_DEVICE_ID;
    doc["status"] = status;

    char payload[256];

    serializeJson(doc, payload, sizeof(payload));

    Serial.println("[TelemetryManager] Sending status:");
    Serial.println(payload);

    return mqtt.publish(MQTT_TOPIC_STATUS, payload);
}

bool TelemetryManager::sendDoseTaken(
    int section,
    const char* scheduledTime,
    const char* timestamp)
{
    return publishEvent(
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
    const char* timestamp)
{
    return publishEvent(
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
    const char* timestamp)
{
    return publishEvent(
        "dose_missed",
        "missed",
        section,
        scheduledTime,
        timestamp
    );
}

bool TelemetryManager::publishEvent(
    const char* event,
    const char* status,
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

    doc["device_id"] = MQTT_DEVICE_ID;
    doc["event"] = event;
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