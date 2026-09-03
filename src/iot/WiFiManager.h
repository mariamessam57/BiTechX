#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();

    void begin();
    void update();

    bool isConnected() const;
    void reconnect();

private:
    unsigned long lastReconnectAttempt;
};