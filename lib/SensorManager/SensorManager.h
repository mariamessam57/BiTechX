#pragma once

class SensorManager {
public:
    SensorManager();

    void begin();
    float readUltrasonicDistance();
    bool isPersonPresent();
    bool isMedicineDetected();
    bool isIrActive();
};
