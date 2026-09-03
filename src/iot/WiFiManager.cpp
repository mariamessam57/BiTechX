#include "WiFiManager.h"
#include "config.h"

WiFiManager::WiFiManager()
    : lastReconnectAttempt(0) {
}

void WiFiManager::begin() {
    Serial.println("[WiFiManager] Connecting to Wi-Fi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    lastReconnectAttempt = millis();
}

void WiFiManager::update() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    if (millis() - lastReconnectAttempt >= 10000) {
        reconnect();
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::reconnect() {
    Serial.println("[WiFiManager] Reconnecting...");

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    lastReconnectAttempt = millis();
}