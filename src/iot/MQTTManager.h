#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

using MQTTMessageCallback = void (*)(const char* topic, const char* payload);

class MQTTManager
{
public:
    MQTTManager();

    void begin();
    void update();

    bool isConnected();

    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);

    void setMessageCallback(MQTTMessageCallback callback);

    bool sendRpcResponse(
        const char* requestId,
        const char* responseJson
    );

private:
    WiFiClient espClient;
    PubSubClient mqttClient;

    unsigned long lastReconnectAttempt;

    MQTTMessageCallback messageCallback;

    void reconnect();

    void mqttCallback(
        char* topic,
        byte* payload,
        unsigned int length
    );
};