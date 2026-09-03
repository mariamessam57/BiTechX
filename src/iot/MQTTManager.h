#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

class MQTTManager {
public:
    MQTTManager();

    void begin();
    void update();

   bool isConnected();

    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);

private:
    WiFiClient espClient;
    PubSubClient mqttClient;

    unsigned long lastReconnectAttempt;

    void reconnect();
};