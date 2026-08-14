#pragma once

#include <Arduino.h>

class MotorManager {
private:
    // أطراف ماتورين السير (Conveyor Motors - M1 & M2)
    uint8_t pinBeltM1_IN1;
    uint8_t pinBeltM1_IN2;
    uint8_t pinBeltM2_IN1;
    uint8_t pinBeltM2_IN2;

    // أطراف ماتور البوابة (Door Motor - M3)
    uint8_t pinDoor_IN1;
    uint8_t pinDoor_IN2;

    bool isBeltRunning;
    bool isDoorOpenStatus;

public:
    // Constructor يستلم أطراف الـ 3 مواتير (2 للسير + 1 للبوابة)
    MotorManager(uint8_t belt1_in1 = 23, uint8_t belt1_in2 = 19, 
                 uint8_t belt2_in1 = 17, uint8_t belt2_in2 = 5,
                 uint8_t door_in1 = 18,  uint8_t door_in2 = 4);
    
    void begin();
    
    // ==================== 1. التحكم في السير (Conveyor Belt) ====================
    void moveBeltForward();
    void moveBeltBackward();
    void stopBelt();
    // تشغيل السير بزمن مرن (الافتراضي 2000ms وتقدر تبعت أي زمن زي 5000ms)
    void dispenseConveyorSequence(uint32_t runDurationMs = 2000);

    // ==================== 2. التحكم في البوابة (Door Motor) ====================
    void openDoor();
    void closeDoor();
    void stopDoor();
    // تسلسل كامل: فتح البوابة -> مهلة أخذ الدواء -> إغلاق البوابة (FreeRTOS-Safe)
    void operateDoorSequence(uint32_t openDurationMs = 3000);

    // متابعة الحالة
    bool isConveyorRunning() const { return isBeltRunning; }
    bool isDoorOpen() const { return isDoorOpenStatus; }
};