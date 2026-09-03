#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum class MotorDirection {
    FORWARD,
    BACKWARD,
    STOP
};

class MotorManager {
private:
    uint8_t in1Pin;
    uint8_t in2Pin;
    SemaphoreHandle_t motorMutex;
    
    bool motorRunning;
    uint32_t motorStartTime;
    MotorDirection currentDirection;

    void setMotorState(uint8_t in1, uint8_t in2, MotorDirection dir, uint8_t speed);

public:
    MotorManager(uint8_t in1, uint8_t in2);

    void begin();
    void run(MotorDirection dir, uint8_t speed = 150);
    void forward(uint8_t speed = 150);
    void backward(uint8_t speed = 150);
    void stop();
    void runForDuration(MotorDirection dir, uint32_t durationMs, uint8_t speed = 150);
    
    // Conveyor-specific methods
    void startConveyor(uint8_t speed = 150);
    void stopConveyor();
    
    // State query methods
    bool isRunning() const;
    uint32_t getRunTimeMs() const;
};