#pragma once

#include <Arduino.h>
#include "Servo.h"


class ServoManager {
private:
    ServoControl carouselServo;
    uint8_t currentSection;

    
    const int SERVO_STOP = 90;              
    const int SERVO_FORWARD_SPEED = 125;    
    const uint32_t STEP_ROTATION_MS = 620;   // زمن الدوران المطلوب للانتقال لخانة واحدة (يُعاير حسب الميكانيزم)

public:
    explicit ServoManager(uint8_t pin);

    void begin();
    void rotateToNextSection();
    void rotateToSection(uint8_t targetSection);
    void resetToFirstSection();
    uint8_t getCurrentSection() const;
    void stop();
};