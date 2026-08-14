#include "Servo.h"

ServoControl::ServoControl(uint8_t pin) 
    : servoPin(pin), currentAngle(0), servoMutex(NULL) {}

void ServoControl::begin() {
    // 1. إنشاء Mutex خاص بالـ FreeRTOS لحماية السيرفو
    if (servoMutex == NULL) {
        servoMutex = xSemaphoreCreateMutex();
    }

    // 2. ربط السيرفو بالـ Pin وتصفير الزاوية
    myServo.attach(servoPin);
    write(0);
}

void ServoControl::write(int angle) {
    // تأمين الـ FreeRTOS (حجز السيرفو للـ Task الحالية)
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        // شروط الأمان لتحديد حدود الزوايا المسموحة (حتى 360 درجة)
        if (angle < 0) angle = 0;
        if (angle > 360) angle = 360; 

        currentAngle = angle;

        // التعرّف التلقائي على حالة التوصيل
        if (!myServo.attached()) {
            myServo.attach(servoPin);
        }

        // إرسال الإشارة المباشرة للسيرفو
        myServo.write(currentAngle);

        // تحرير السيرفو لتستطيع الـ Tasks الأخرى استخدامه
        xSemaphoreGive(servoMutex);
    }
}

void ServoControl::writeSlowly(int targetAngle, uint32_t stepDelayMs) {
    // شروط الأمان للحدود
    if (targetAngle < 0) targetAngle = 0;
    if (targetAngle > 360) targetAngle = 360;

    int startAngle = currentAngle;

    // حركة تدريجية للأمام أو الخلف بسلام مع FreeRTOS Delay
    if (startAngle < targetAngle) {
        for (int a = startAngle; a <= targetAngle; a++) {
            write(a);
            vTaskDelay(pdMS_TO_TICKS(stepDelayMs)); // تأخير آمن لا يعطل الـ CPU
        }
    } else {
        for (int a = startAngle; a >= targetAngle; a--) {
            write(a);
            vTaskDelay(pdMS_TO_TICKS(stepDelayMs)); // تأخير آمن لا يعطل الـ CPU
        }
    }
}

int ServoControl::read() const {
    return currentAngle;
}

void ServoControl::detach() {
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (myServo.attached()) {
            myServo.detach();
        }
        xSemaphoreGive(servoMutex);
    }
}

void ServoControl::attach() {
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!myServo.attached()) {
            myServo.attach(servoPin);
        }
        xSemaphoreGive(servoMutex);
    }
}