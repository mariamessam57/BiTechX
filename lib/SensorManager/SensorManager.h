#pragma once

#include <Arduino.h>

class SensorManager {
public:
    SensorManager();

    void begin();
    float readUltrasonicDistance();
    bool isIrActive();
};
