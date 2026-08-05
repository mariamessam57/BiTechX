#pragma once

#include <Arduino.h>

class SystemController {
public:
    SystemController();
    void begin();
    void resetSystem();
    void enterManualMode();
};
