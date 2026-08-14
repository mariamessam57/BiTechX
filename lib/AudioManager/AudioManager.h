#pragma once

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include "config.h"

class AudioManager {
private:
    HardwareSerial dfSerial;
    DFRobotDFPlayerMini dfPlayer;
    bool isDfPlayerReady;

public:
    AudioManager();
    void begin();
    
    // الدوال القديمة بنفس الأسامي من الصورة بالضبط
    void playNotification();
    void playErrorTone();
    
    // دوال إضافية للتحكم المتقدم في الصوت
    void playBuzzerBeep(uint16_t durationMs = 100);
    void playTrack(uint8_t trackNumber);
    void setVolume(uint8_t volume);
    void stopAudio();
};