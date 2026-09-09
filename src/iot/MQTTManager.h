#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

// MQTT message callback
using MQTTMessageCallback = void (*)(const char* topic, const char* payload);

class MQTTManager {
public:
    MQTTManager();

    void begin();
    void update();

    bool isConnected();

    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);

    // Set callback for incoming MQTT messages
    void setMessageCallback(MQTTMessageCallback callback);

private:
    WiFiClient espClient;
    PubSubClient mqttClient;

    unsigned long lastReconnectAttempt;

    MQTTMessageCallback messageCallback;

    void reconnect();
    void mqttCallback(char* topic, byte* payload, unsigned int length);
};