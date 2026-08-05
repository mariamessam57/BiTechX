#pragma once

#include <Arduino.h>

class DoorManager {
public:
    DoorManager();
    void begin();
    void openDoor();
    void closeDoor();
};
