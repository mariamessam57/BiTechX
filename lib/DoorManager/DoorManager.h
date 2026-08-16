#pragma once

#include <Arduino.h>
#include "Servo.h"

enum class DoorState {
    CLOSED,
    OPEN,
    OPENING,
    CLOSING,
    UNKNOWN
};

class DoorManager {
private:
    ServoControl doorServo;
    DoorState currentState;

    const int ANGLE_CLOSED = 0;    
    const int ANGLE_OPEN = 90;    
    const uint32_t STEP_SPEED_MS = 15; // سرعة حركة البوابة بالمللي ثانية لكل درجة

public:
    explicit DoorManager(uint8_t pin);

    void begin();
    void open();
    void close();
    
    // إيقاف الإشارة عن السيرفو لتوفير الطاقة ومنع الزنة عند الثبات
    void relax(); 

    DoorState getState() const;
    bool isOpen() const;
    bool isClosed() const;
};