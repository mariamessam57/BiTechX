#pragma once

#include <Arduino.h>

class AudioManager {
public:
    AudioManager();
    void begin();
    void playNotification();
    void playErrorTone();
};
