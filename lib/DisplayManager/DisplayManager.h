#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class DisplayManager {
private:
    LiquidCrystal_I2C lcd;

public:
    DisplayManager();
    void begin();
    
    // الدوال بالأسماء القديمة بالضبط من الصورة
    void showMessage(const String &message);
    void showStatus(const String &status);
    
    // دالة إضافية لعرض سطرين بوضوح على LCD 16x2
    void showMessage(const String &line1, const String &line2);
    void clear();
    
    // دوال حالة الـ RGB LED
    void setStatusColor(bool red, bool green, bool blue);
    void showStatusReady();   // لون أخضر
    void showStatusBusy();    // لون أزرق
    void showStatusError();   // لون أحمر
};