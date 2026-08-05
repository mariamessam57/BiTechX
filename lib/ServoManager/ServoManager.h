#pragma once

#include <Arduino.h>

class ServoManager {
public:
    ServoManager();
    void begin();
    void moveDoorServo(float angle);
    void moveMainServo(float angle);
    void moveMedicineServo(uint8_t slot, float angle);
};
