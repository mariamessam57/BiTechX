#include "DoorManager.h"

DoorManager::DoorManager(uint8_t pin) 
    : doorServo(pin), currentState(DoorState::UNKNOWN) {}

void DoorManager::begin() {
    doorServo.begin();
    close();
}

void DoorManager::open() {
    if (currentState == DoorState::OPEN) return;

    currentState = DoorState::OPENING;
    doorServo.writeSlowly(ANGLE_OPEN, STEP_SPEED_MS);
    currentState = DoorState::OPEN;
}

void DoorManager::close() {
    if (currentState == DoorState::CLOSED) return;

    currentState = DoorState::CLOSING;
    doorServo.writeSlowly(ANGLE_CLOSED, STEP_SPEED_MS);
    currentState = DoorState::CLOSED;
    
    // راحة الموتور بعد الغلق التام
    relax();
}

void DoorManager::relax() {
    doorServo.detach();
}

DoorState DoorManager::getState() const {
    return currentState;
}

bool DoorManager::isOpen() const {
    return currentState == DoorState::OPEN;
}

