#include "ServoManager.h"

ServoManager::ServoManager(uint8_t pin)
    : carouselServo(pin), currentSection(1) {}

void ServoManager::begin() {
    carouselServo.begin();
    stop();
    currentSection = 1;
}

void ServoManager::stop() {
    carouselServo.write(SERVO_STOP);
}

void ServoManager::rotateToNextSection() {
   
    carouselServo.write(SERVO_FORWARD_SPEED);
    vTaskDelay(pdMS_TO_TICKS(STEP_ROTATION_MS));

   
    stop();
    vTaskDelay(pdMS_TO_TICKS(150)); // استقرار ميكانيكي

    currentSection++;
    if (currentSection > 7) {
        currentSection = 1;
    }
}

void ServoManager::rotateToSection(uint8_t targetSection) {
    if (targetSection < 1 || targetSection > 7) return;

    while (currentSection != targetSection) {
        rotateToNextSection();
    }
}

void ServoManager::resetToFirstSection() {
    currentSection = 1;
    stop();
}

uint8_t ServoManager::getCurrentSection() const {
    return currentSection;
}