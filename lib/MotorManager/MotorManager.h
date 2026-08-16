#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum class MotorID {
    MOTOR_1,
    MOTOR_2,
    BOTH
};

enum class MotorDirection {
    FORWARD,
    BACKWARD,
    STOP
};

class MotorManager {
private:
   
    uint8_t m1_in1Pin;
    uint8_t m1_in2Pin;

    
    uint8_t m2_in1Pin;
    uint8_t m2_in2Pin;

    SemaphoreHandle_t motorMutex;

    void setMotorState(uint8_t in1, uint8_t in2, MotorDirection dir, uint8_t speed);

public:
    MotorManager(uint8_t m1_in1, uint8_t m1_in2, uint8_t m2_in1, uint8_t m2_in2);

    void begin();
    
    // دوال التحكم الأساسية في الاتجاه والسرعة
    void run(MotorID motor, MotorDirection dir, uint8_t speed = 150);
    void forward(MotorID motor = MotorID::BOTH, uint8_t speed = 150);
    void backward(MotorID motor = MotorID::BOTH, uint8_t speed = 150);
    void stop(MotorID motor = MotorID::BOTH);

    // تشغيل المحرك لفترة زمنية محددة دون إيقاف باقي مهام الـ FreeRTOS
    void runForDuration(MotorID motor, MotorDirection dir, uint32_t durationMs, uint8_t speed = 150);
};