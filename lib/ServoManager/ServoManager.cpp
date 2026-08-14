#include "ServoManager.h"

ServoManager::ServoManager() {}

void ServoManager::begin() {
    // ربط السيرفو بالطرف المحدد في config.h
    dispenserServo.attach(PIN_SERVO_DISPENSER);
    
    // العودة للسكشن الأول كوضع افتراضي عند التشغيل
    selectSlot(1);
}

void ServoManager::setAngle(float angle) {
    // حماية الحدود لسيرفو 360 درجة
    if (angle < 0) angle = 0;
    if (angle > 360) angle = 360; 
    
    dispenserServo.write(static_cast<int>(angle));
}

void ServoManager::selectSlot(uint8_t slotNumber) {
    // حماية: التأكد أن رقم السكشن بين 1 و 7
    if (slotNumber < 1 || slotNumber > TOTAL_SLOTS) {
        return;
    }
    
    // حساب الزاوية بناءً على رقم السكشن (Slot 1 = 0°, Slot 2 = 51.4°, ...)
    float targetAngle = (slotNumber - 1) * ANGLE_PER_SLOT;
    setAngle(targetAngle);
}

// ============================================================
//  تنفيذ دوال التوافق بنفس الأسماء القديمة بالضبط
// ============================================================

void ServoManager::moveDoorServo(float angle) {
    // تركناها فاضية لمنع الكراش، لأن البوابة أصبحت تُدار بموتور N20 عبر MotorManager
}

void ServoManager::moveMainServo(float angle) {
    setAngle(angle);
}

void ServoManager::moveMedicineServo(uint8_t slot, float angle) {
    // عند استدعاء هذه الدالة القديمة، توجّه السيرفو أوتوماتيكياً للسكشن المحدد
    selectSlot(slot);
}