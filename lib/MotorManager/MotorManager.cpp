#include "MotorManager.h"

MotorManager::MotorManager(uint8_t belt1_in1, uint8_t belt1_in2, 
                           uint8_t belt2_in1, uint8_t belt2_in2,
                           uint8_t door_in1,  uint8_t door_in2) 
    : pinBeltM1_IN1(belt1_in1), pinBeltM1_IN2(belt1_in2),
      pinBeltM2_IN1(belt2_in1), pinBeltM2_IN2(belt2_in2),
      pinDoor_IN1(door_in1),    pinDoor_IN2(door_in2),
      isBeltRunning(false),     isDoorOpenStatus(false) {}

void MotorManager::begin() {
    // إعداد أطراف مواتير السير
    pinMode(pinBeltM1_IN1, OUTPUT);
    pinMode(pinBeltM1_IN2, OUTPUT);
    pinMode(pinBeltM2_IN1, OUTPUT);
    pinMode(pinBeltM2_IN2, OUTPUT);

    // إعداد أطراف ماتور البوابة
    pinMode(pinDoor_IN1, OUTPUT);
    pinMode(pinDoor_IN2, OUTPUT);

    // إيقاف جميع المواتير فوراً عند التشغيل للحماية
    stopBelt();
    stopDoor();
}

// ----------------- التحكم في السير -----------------
void MotorManager::moveBeltForward() {
    digitalWrite(pinBeltM1_IN1, HIGH);
    digitalWrite(pinBeltM1_IN2, LOW);
    digitalWrite(pinBeltM2_IN1, HIGH);
    digitalWrite(pinBeltM2_IN2, LOW);
    isBeltRunning = true;
}

void MotorManager::moveBeltBackward() {
    digitalWrite(pinBeltM1_IN1, LOW);
    digitalWrite(pinBeltM1_IN2, HIGH);
    digitalWrite(pinBeltM2_IN1, LOW);
    digitalWrite(pinBeltM2_IN2, HIGH);
    isBeltRunning = true;
}

void MotorManager::stopBelt() {
    digitalWrite(pinBeltM1_IN1, LOW);
    digitalWrite(pinBeltM1_IN2, LOW);
    digitalWrite(pinBeltM2_IN1, LOW);
    digitalWrite(pinBeltM2_IN2, LOW);
    isBeltRunning = false;
}

void MotorManager::dispenseConveyorSequence(uint32_t runDurationMs) {
    moveBeltForward();
    vTaskDelay(pdMS_TO_TICKS(runDurationMs)); // زمن تحريك السير المرن دون تعطيل الـ CPU
    stopBelt();
}

// ----------------- التحكم في البوابة -----------------
void MotorManager::openDoor() {
    digitalWrite(pinDoor_IN1, HIGH);
    digitalWrite(pinDoor_IN2, LOW);
    isDoorOpenStatus = true;
}

void MotorManager::closeDoor() {
    digitalWrite(pinDoor_IN1, LOW);
    digitalWrite(pinDoor_IN2, HIGH);
    isDoorOpenStatus = false;
}

void MotorManager::stopDoor() {
    digitalWrite(pinDoor_IN1, LOW);
    digitalWrite(pinDoor_IN2, LOW);
}

void MotorManager::operateDoorSequence(uint32_t openDurationMs) {
    // 1. فتح البوابة (دوران الماتور لمدة ثانية)
    openDoor();
    vTaskDelay(pdMS_TO_TICKS(1000));
    stopDoor();

    // 2. الانتظار لمهلة أخذ المريض للدواء
    vTaskDelay(pdMS_TO_TICKS(openDurationMs));

    // 3. إغلاق البوابة (دوران عكسي لمدة ثانية)
    closeDoor();
    vTaskDelay(pdMS_TO_TICKS(1000));
    stopDoor();
}