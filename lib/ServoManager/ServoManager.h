#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class ServoManager {
private:
    Servo dispenserServo;                   // سيرفو الديسبينسر/القرص الدوار
    const uint8_t TOTAL_SLOTS = 7;          // إجمالي عدد السكاشن (7 slots)
    const float ANGLE_PER_SLOT = 360.0f / 7.0f; // 51.428 درجة لكل سكشن
                                           
public:
    ServoManager();
    void begin();
    
    // الدوال الجديدة المنظمة
    void setAngle(float angle);             // تحريك السيرفو بزاوية مباشرة (0-360)
    void selectSlot(uint8_t slotNumber);    // توجيه القرص الدوار لسكشن معّين (1 إلى 7)

    // ============================================================
    //  دوال التوافق (Backward Compatibility) للحفاظ على كود زميلك
    // ============================================================
    void moveDoorServo(float angle);                      // كانت للبوابة (تترك فاضية لأن البوابة بقت موتور N20)
    void moveMainServo(float angle);                      // تحريك سيرفو الاختيار الرئيسي
    void moveMedicineServo(uint8_t slot, float angle);    // توجيه الجرعة للسكشن المطلوب
};