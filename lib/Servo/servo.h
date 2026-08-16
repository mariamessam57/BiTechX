#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


class ServoControl {
private:
    Servo myServo;                 
    uint8_t servoPin;              
    int currentAngle;               
    SemaphoreHandle_t servoMutex;   

public:
   
    explicit ServoControl(uint8_t pin);


    void begin();

    void write(int angle);


     //targetAngle الزاوية النهائية
     //stepDelayMs الوقت بالملي ثانية بين كل درجة والتالية
    void writeSlowly(int targetAngle, uint32_t stepDelayMs = 15);

    int read() const;

    void detach();
    
    void attach();
};