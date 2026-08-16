#pragma once

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum class SoundTrack : uint8_t {
    SYSTEM_STARTUP     = 1,
    MEDICINE_REMINDER  = 2,
    DOOR_OPENING       = 3,
    TAKEN_SUCCESS      = 4,
    MISSED_WARNING     = 5,
    EMERGENCY_ALARM    = 6
};

class AudioManager {
private:
    HardwareSerial audioSerial;
    DFRobotDFPlayerMini dfPlayer;
    uint8_t rxPin;
    uint8_t txPin;
    uint8_t busyPin;
    SemaphoreHandle_t audioMutex;
    bool isInitialized;

public:
    explicit AudioManager(uint8_t rx, uint8_t tx, uint8_t busy = 255);

    bool begin(uint8_t initialVolume = 25);
    void playTrack(SoundTrack track);
    void playTrackNumber(uint16_t trackNumber);
    void setVolume(uint8_t volume);
    void stop();
    void pause();
    void resume();
    void triggerMedicineAlarm();
    bool isPlaying();
    bool isReady() const { return isInitialized; }
};