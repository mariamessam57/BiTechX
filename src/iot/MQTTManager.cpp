#include "MQTTManager.h"
#include "config.h"

MQTTManager::MQTTManager()
    : mqttClient(espClient),
      lastReconnectAttempt(0),
      messageCallback(nullptr)
{
}

void MQTTManager::begin()
{
    Serial.println("[MQTTManager] Initializing MQTT...");

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

    // Set callback for incoming MQTT messages
    mqttClient.setCallback(
        [this](char* topic, byte* payload, unsigned int length)
        {
            mqttCallback(topic, payload, length);
        }
    );

    lastReconnectAttempt = 0;
}

void MQTTManager::update()
{
    // MQTT client must be connected before processing messages
    if (mqttClient.connected())
    {
        mqttClient.loop();
        return;
    }

    // Wait until Wi-Fi is available
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    // Try to reconnect every 5 seconds
    if (millis() - lastReconnectAttempt >= 5000)
    {
        reconnect();
    }
}

bool MQTTManager::isConnected()
{
    return mqttClient.connected();
}

bool MQTTManager::publish(const char* topic, const char* payload)
{
    if (!mqttClient.connected())
    {
        Serial.println("[MQTTManager] Publish failed: MQTT not connected");
        return false;
    }

    bool success = mqttClient.publish(topic, payload);

    if (success)
    {
        Serial.print("[MQTTManager] Published to: ");
        Serial.println(topic);
    }
    else
    {
        Serial.print("[MQTTManager] Failed to publish to: ");
        Serial.println(topic);
    }

    return success;
}

bool MQTTManager::subscribe(const char* topic)
{
    if (!mqttClient.connected())
    {
        Serial.println("[MQTTManager] Subscribe failed: MQTT not connected");
        return false;
    }

    bool success = mqttClient.subscribe(topic);

    if (success)
    {
        Serial.print("[MQTTManager] Subscribed to: ");
        Serial.println(topic);
    }
    else
    {
        Serial.print("[MQTTManager] Failed to subscribe to: ");
        Serial.println(topic);
    }

    return success;
}

void MQTTManager::setMessageCallback(MQTTMessageCallback callback)
{
    messageCallback = callback;
}

void MQTTManager::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    // Convert payload to a null-terminated string
    char message[length + 1];

    for (unsigned int i = 0; i < length; i++)
    {
        message[i] = (char)payload[i];
    }

    message[length] = '\0';

    Serial.println("[MQTTManager] Message received:");
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Payload: ");
    Serial.println(message);

    // Send message to the registered callback
    if (messageCallback != nullptr)
    {
        messageCallback(topic, message);
    }
}

void MQTTManager::reconnect()
{
    Serial.println("[MQTTManager] Connecting to MQTT broker...");

    lastReconnectAttempt = millis();

    // Generate a unique MQTT client ID
    String clientId = "BiTechX_ESP32_";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
        Serial.println("[MQTTManager] MQTT connected!");

        // Subscribe to Dashboard → ESP32 commands
        if (subscribe(MQTT_TOPIC_COMMANDS))
        {
            Serial.println("[MQTTManager] Command subscription ready");
        }
    }
    else
    {
        Serial.print("[MQTTManager] MQTT connection failed, state: ");
        Serial.println(mqttClient.state());
    }
}