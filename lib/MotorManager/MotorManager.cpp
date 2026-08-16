#include "MotorManager.h"

MotorManager::MotorManager(uint8_t m1_in1, uint8_t m1_in2, uint8_t m2_in1, uint8_t m2_in2)
    : m1_in1Pin(m1_in1), m1_in2Pin(m1_in2), 
      m2_in1Pin(m2_in1), m2_in2Pin(m2_in2), 
      motorMutex(NULL) {}

void MotorManager::begin() {
    if (motorMutex == NULL) {
        motorMutex = xSemaphoreCreateMutex();
    }

    pinMode(m1_in1Pin, OUTPUT);
    pinMode(m1_in2Pin, OUTPUT);
    pinMode(m2_in1Pin, OUTPUT);
    pinMode(m2_in2Pin, OUTPUT);

    stop(MotorID::BOTH);
}

void MotorManager::setMotorState(uint8_t in1, uint8_t in2, MotorDirection dir, uint8_t speed) {
    switch (dir) {
        case MotorDirection::FORWARD:
            analogWrite(in1, speed);
            analogWrite(in2, 0);
            break;
        case MotorDirection::BACKWARD:
            analogWrite(in1, 0);
            analogWrite(in2, speed);
            break;
        case MotorDirection::STOP:
        default:
            analogWrite(in1, 0);
            analogWrite(in2, 0);
            break;
    }
}

void MotorManager::run(MotorID motor, MotorDirection dir, uint8_t speed) {
    if (motorMutex != NULL && xSemaphoreTake(motorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (motor == MotorID::MOTOR_1 || motor == MotorID::BOTH) {
            setMotorState(m1_in1Pin, m1_in2Pin, dir, speed);
        }
        if (motor == MotorID::MOTOR_2 || motor == MotorID::BOTH) {
            setMotorState(m2_in1Pin, m2_in2Pin, dir, speed);
        }
        xSemaphoreGive(motorMutex);
    }
}

void MotorManager::forward(MotorID motor, uint8_t speed) {
    run(motor, MotorDirection::FORWARD, speed);
}

void MotorManager::backward(MotorID motor, uint8_t speed) {
    run(motor, MotorDirection::BACKWARD, speed);
}

void MotorManager::stop(MotorID motor) {
    run(motor, MotorDirection::STOP, 0);
}

void MotorManager::runForDuration(MotorID motor, MotorDirection dir, uint32_t durationMs, uint8_t speed) {
    run(motor, dir, speed);
    vTaskDelay(pdMS_TO_TICKS(durationMs)); // انتظار آمن في FreeRTOS
    stop(motor);
}