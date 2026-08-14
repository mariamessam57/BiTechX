#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @class ServoControl
 * @brief Low-Level Driver موحد ومحمي لـ FreeRTOS للتحكم في أي سيرفو (180 أو 360).
 */
class ServoControl {
private:
    Servo myServo;                  // أوبجكت السيرفو الداخلي
    uint8_t servoPin;               // رقم الـ GPIO الموصول عليه السيرفو
    int currentAngle;               // الزاوية المخزنة حالياً
    SemaphoreHandle_t servoMutex;   // حماية FreeRTOS لمنع التداخل بين الـ Tasks

public:
    /**
     * @brief Construct a new Servo Control object
     * @param pin رقم الـ GPIO الموصول عليه السيرفو
     */
    explicit ServoControl(uint8_t pin);

    /**
     * @brief تهيئة السيرفو وإنشاء حماية الـ FreeRTOS
     */
    void begin();

    /**
     * @brief تحريك السيرفو فوراً لزاوية محددة (من 0 لـ 360)
     * @param angle الزاوية المطلوبة
     */
    void write(int angle);

    /**
     * @brief تحريك السيرفو بنعومة وبطء (ممتاز للبوابة ولل قرص الدوار)
     * @param targetAngle الزاوية النهائية
     * @param stepDelayMs الوقت بالملي ثانية بين كل درجة والتانية (مثلاً 15ms)
     */
    void writeSlowly(int targetAngle, uint32_t stepDelayMs = 15);

    /**
     * @brief قراءة الزاوية الحالية للسيرفو
     * @return int الزاوية الحالية
     */
    int read() const;

    /**
     * @brief فصل إشارة الـ PWM لتوفير الطاقة وتقليل هزات السيرفو (Jitter)
     */
    void detach();

    /**
     * @brief إعادة توصيل إشارة الـ PWM بالسيرفو
     */
    void attach();
};