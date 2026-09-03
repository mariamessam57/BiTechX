#include "MotorManager.h"

MotorManager::MotorManager(uint8_t in1, uint8_t in2)
    : in1Pin(in1), in2Pin(in2), motorMutex(NULL), 
      motorRunning(false), motorStartTime(0), currentDirection(MotorDirection::STOP) {}

void MotorManager::begin() {
    if (motorMutex == NULL) {
        motorMutex = xSemaphoreCreateMutex();
    }

    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);

    stop();
}

void MotorManager::setMotorState(uint8_t in1, uint8_t in2, MotorDirection dir, uint8_t speed) {
    // Track motor state changes for runtime monitoring
    if (dir != MotorDirection::STOP && !motorRunning) {
        motorRunning = true;
        motorStartTime = millis();
        currentDirection = dir;
    } else if (dir == MotorDirection::STOP && motorRunning) {
        motorRunning = false;
        currentDirection = MotorDirection::STOP;
    }
    
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

void MotorManager::run(MotorDirection dir, uint8_t speed) {
    if (motorMutex != NULL && xSemaphoreTake(motorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        setMotorState(in1Pin, in2Pin, dir, speed);
        xSemaphoreGive(motorMutex);
    }
}

void MotorManager::forward(uint8_t speed) {
    run(MotorDirection::FORWARD, speed);
}

void MotorManager::backward(uint8_t speed) {
    run(MotorDirection::BACKWARD, speed);
}

void MotorManager::stop() {
    run(MotorDirection::STOP, 0);
}

void MotorManager::runForDuration(MotorDirection dir, uint32_t durationMs, uint8_t speed) {
    run(dir, speed);
    vTaskDelay(pdMS_TO_TICKS(durationMs));
    stop();
}

void MotorManager::startConveyor(uint8_t speed) {
    Serial.printf("[MotorManager] Starting conveyor with speed %u\n", speed);
    forward(speed);
}

void MotorManager::stopConveyor() {
    Serial.println("[MotorManager] Stopping conveyor");
    stop();
}

bool MotorManager::isRunning() const {
    return motorRunning;
}

uint32_t MotorManager::getRunTimeMs() const {
    if (!motorRunning) {
        return 0;
    }
    return millis() - motorStartTime;
}