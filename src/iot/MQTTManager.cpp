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
    Serial.println(
        "[MQTTManager] Initializing MQTT for ThingsBoard..."
    );

    mqttClient.setServer(
        MQTT_SERVER,
        MQTT_PORT
    );

    mqttClient.setBufferSize(512);

    mqttClient.setCallback(
        [this](
            char* topic,
            byte* payload,
            unsigned int length
        )
        {
            mqttCallback(
                topic,
                payload,
                length
            );
        }
    );

    lastReconnectAttempt = 0;
}

void MQTTManager::update()
{
    if (mqttClient.connected())
    {
        mqttClient.loop();
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if (
        millis() - lastReconnectAttempt
        >= MQTT_RECONNECT_INTERVAL
    )
    {
        reconnect();
    }
}

bool MQTTManager::isConnected()
{
    return mqttClient.connected();
}

bool MQTTManager::publish(
    const char* topic,
    const char* payload
)
{
    if (!mqttClient.connected())
    {
        Serial.println(
            "[MQTTManager] Publish failed: MQTT not connected"
        );

        return false;
    }

    bool success = mqttClient.publish(
        topic,
        payload
    );

    if (success)
    {
        Serial.print(
            "[MQTTManager] Published to: "
        );

        Serial.println(topic);
    }
    else
    {
        Serial.print(
            "[MQTTManager] Failed to publish to: "
        );

        Serial.println(topic);
    }

    return success;
}

bool MQTTManager::subscribe(
    const char* topic
)
{
    if (!mqttClient.connected())
    {
        Serial.println(
            "[MQTTManager] Subscribe failed: MQTT not connected"
        );

        return false;
    }

    bool success = mqttClient.subscribe(topic);

    if (success)
    {
        Serial.print(
            "[MQTTManager] Subscribed to: "
        );

        Serial.println(topic);
    }
    else
    {
        Serial.print(
            "[MQTTManager] Failed to subscribe to: "
        );

        Serial.println(topic);
    }

    return success;
}

void MQTTManager::setMessageCallback(
    MQTTMessageCallback callback
)
{
    messageCallback = callback;
}

void MQTTManager::mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    constexpr size_t MAX_MESSAGE_SIZE = 512;

    if (length >= MAX_MESSAGE_SIZE)
    {
        Serial.println(
            "[MQTTManager] Incoming MQTT message too large"
        );

        return;
    }

    char message[MAX_MESSAGE_SIZE];

    memcpy(
        message,
        payload,
        length
    );

    message[length] = '\0';

    Serial.println(
        "[MQTTManager] MQTT Message received:"
    );

    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Payload: ");
    Serial.println(message);

    if (messageCallback != nullptr)
    {
        messageCallback(
            topic,
            message
        );
    }
}

void MQTTManager::reconnect()
{
    Serial.println(
        "[MQTTManager] Connecting to ThingsBoard MQTT Broker..."
    );

    lastReconnectAttempt = millis();

    String clientId = "BiTechX_ESP32_";

    clientId += String(
        (uint32_t)ESP.getEfuseMac(),
        HEX
    );

    bool connected = mqttClient.connect(
        clientId.c_str(),
        MQTT_ACCESS_TOKEN,
        nullptr
    );

    if (connected)
    {
        Serial.println(
            "[MQTTManager] Connected to ThingsBoard successfully!"
        );

        if (
            subscribe(
                MQTT_TOPIC_RPC_REQUEST
            )
        )
        {
            Serial.println(
                "[MQTTManager] RPC subscription active"
            );
        }
    }
    else
    {
        Serial.print(
            "[MQTTManager] ThingsBoard connection failed, state: "
        );

        Serial.println(
            mqttClient.state()
        );
    }
}

bool MQTTManager::sendRpcResponse(
    const char* requestId,
    const char* responseJson
)
{
    String responseTopic =
        String(MQTT_TOPIC_RPC_RESPONSE_PREFIX)
        + String(requestId);

    return publish(
        responseTopic.c_str(),
        responseJson
    );
}