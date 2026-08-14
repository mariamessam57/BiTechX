#include "DisplayManager.h"

DisplayManager::DisplayManager() : lcd(LCD_I2C_ADDR, 16, 2) {}

void DisplayManager::begin() {
    // تهيئة شاشة الـ LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // تهيئة أطراف الـ RGB LED
    pinMode(PIN_RGB_RED, OUTPUT);
    pinMode(PIN_RGB_GREEN, OUTPUT);
    pinMode(PIN_RGB_BLUE, OUTPUT);
    
    showStatusReady();
    showMessage("BiTechX System", "Initializing...");
}

// الدالة القديمة المباشرة (تعرض في السطر الأول)
void DisplayManager::showMessage(const String &message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

// الدالة القديمة للحالة (تعرض في السطر الثاني مع إضاءة الحالة)
void DisplayManager::showStatus(const String &status) {
    lcd.setCursor(0, 1);
    lcd.print("                "); // مسح السطر الثاني
    lcd.setCursor(0, 1);
    lcd.print(status);
}

// دالة جديدة لعرض سطرين متكاملين
void DisplayManager::showMessage(const String &line1, const String &line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    
    if (line2.length() > 0) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

void DisplayManager::clear() {
    lcd.clear();
}

void DisplayManager::setStatusColor(bool red, bool green, bool blue) {
    digitalWrite(PIN_RGB_RED, red ? HIGH : LOW);
    digitalWrite(PIN_RGB_GREEN, green ? HIGH : LOW);
    digitalWrite(PIN_RGB_BLUE, blue ? HIGH : LOW);
}

void DisplayManager::showStatusReady() {
    setStatusColor(false, true, false); // أخضر
}

void DisplayManager::showStatusBusy() {
    setStatusColor(false, false, true); // أزرق
}

void DisplayManager::showStatusError() {
    setStatusColor(true, false, false); // أحمر
}