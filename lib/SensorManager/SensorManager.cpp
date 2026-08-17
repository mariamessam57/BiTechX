#include "SensorManager.h"
#include "config.h"

SensorManager::SensorManager() {}

void SensorManager::begin() {
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    pinMode(IR_SENSOR_PIN, INPUT);
}

float SensorManager::readUltrasonicDistance() {
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    const unsigned long pulseWidthUs = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, ULTRASONIC_ECHO_TIMEOUT_US);
    if (pulseWidthUs == 0UL) {
        return -1.0f;
    }

    const float distanceCm = (pulseWidthUs * 0.0343f) / 2.0f;
    if (distanceCm <= 0.0f) {
        return -1.0f;
    }

    return distanceCm;
}

bool SensorManager::isIrActive() {
    return digitalRead(IR_SENSOR_PIN) == IR_SENSOR_ACTIVE_LEVEL;
}
