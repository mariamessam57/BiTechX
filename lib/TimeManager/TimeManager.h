#pragma once

#include <Arduino.h>

class TimeManager {
public:
    TimeManager();
    void begin();
    void updateTime();
    bool isScheduledTime(uint8_t hour, uint8_t minute);
};
