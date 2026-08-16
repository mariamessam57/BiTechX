#include "Servo.h"

ServoControl::ServoControl(uint8_t pin) 
    : servoPin(pin), currentAngle(0), servoMutex(NULL) {}

void ServoControl::begin() {
    // 1. إنشاء Mutex خاص بالـ FreeRTOS لحماية السيرفو
    if (servoMutex == NULL) {
        servoMutex = xSemaphoreCreateMutex();
    }

    // 2. حجز تايمرات PWM الخاصة بـ ESP32 لضمان استقرار الإشارة
    //علشان لو السيرفوهين هيشتغلو مع بعض ميحلش تداخل 
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    myServo.setPeriodHertz(50); // معيار 50Hz القياسي للسيرفو

    // 3. ربط السيرفو بالـ Pin مع الحدود النبضية القياسية وتصفير الموضع
    myServo.attach(servoPin, 500, 2400);
    write(0);
}

void ServoControl::write(int angle) {
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
       
        if (angle < 0) angle = 0;
        if (angle > 360) angle = 360; 

        currentAngle = angle;

        if (!myServo.attached()) {
            myServo.attach(servoPin, 500, 2400);
        }

        myServo.write(currentAngle);

        xSemaphoreGive(servoMutex);
    }
}

void ServoControl::writeSlowly(int targetAngle, uint32_t stepDelayMs) {
    if (targetAngle < 0) targetAngle = 0;
    if (targetAngle > 360) targetAngle = 360;

    int startAngle = currentAngle;

    if (startAngle < targetAngle) {
        for (int a = startAngle; a <= targetAngle; a++) {
            write(a);
            vTaskDelay(pdMS_TO_TICKS(stepDelayMs)); // تأخير آمن لا يعطل باقي المهام
        }
    } else {
        for (int a = startAngle; a >= targetAngle; a--) {
            write(a);
            vTaskDelay(pdMS_TO_TICKS(stepDelayMs));
        }
    }
}

int ServoControl::read() const {
    return currentAngle;
}
//pwm فصل   
void ServoControl::detach() {
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (myServo.attached()) {
            myServo.detach();
        }
        xSemaphoreGive(servoMutex);
    }
}
//pwm تشغيل
void ServoControl::attach() {
    if (servoMutex != NULL && xSemaphoreTake(servoMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!myServo.attached()) {
            myServo.attach(servoPin, 500, 2400);
        }
        xSemaphoreGive(servoMutex);
    }
}