#pragma once

#include <Arduino.h>

class DisplayManager {
public:
    DisplayManager();
    void begin();
    void showMessage(const String &message);
    void showStatus(const String &status);
};
