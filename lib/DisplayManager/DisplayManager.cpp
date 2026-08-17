#include "DisplayManager.h"
#include "config.h"

DisplayManager::DisplayManager() : currentState(DISPLAY_IDLE), lastUpdateTime(0) {
}

void DisplayManager::begin() {
    tftDisplay.begin();
    clear();
    displayMessage("INITIALIZING...", TFT_YELLOW);
}

void DisplayManager::end() {
    tftDisplay.end();
}

void DisplayManager::clear() {
    tftDisplay.fillScreen(TFT_BLACK);
}

void DisplayManager::turnOn() {
    tftDisplay.displayOn();
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
}

void DisplayManager::turnOff() {
    digitalWrite(TFT_BACKLIGHT_PIN, LOW);
    tftDisplay.displayOff();
}

void DisplayManager::setRotation(uint8_t r) {
    tftDisplay.setRotation(r);
}

void DisplayManager::setState(DisplayState state) {
    currentState = state;
}

DisplayState DisplayManager::getState() const {
    return currentState;
}

void DisplayManager::drawHeader(const char* title) {
    tftDisplay.fillRect(0, 0, tftDisplay.width(), 30, TFT_BLUE);
    tftDisplay.setTextColor(TFT_WHITE, TFT_BLUE);
    tftDisplay.setTextSize(2);
    
    // Center the title
    int16_t textWidth = strlen(title) * 12;
    int16_t x = (tftDisplay.width() - textWidth) / 2;
    tftDisplay.drawText(x, 8, title, TFT_WHITE, 2);
    
    tftDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    tftDisplay.setTextSize(1);
}

void DisplayManager::drawCenteredText(int16_t y, const char* text, uint16_t color, uint8_t size) {
    tftDisplay.setTextSize(size);
    tftDisplay.setTextColor(color);
    
    int16_t textWidth = strlen(text) * 6 * size;
    int16_t x = (tftDisplay.width() - textWidth) / 2;
    tftDisplay.drawText(x, y, text, color, size);
}

void DisplayManager::drawSeparator(int16_t y, uint16_t color) {
    tftDisplay.drawLine(10, y, tftDisplay.width() - 10, y, color);
}

void DisplayManager::showIdleScreen(uint8_t hour, uint8_t minute, const char* status) {
    clear();
    drawHeader("MEDICINE");
    
    // Time display
    tftDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDisplay.setTextSize(3);
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
    drawCenteredText(80, timeStr, TFT_YELLOW, 3);
    
    // Status
    drawSeparator(140, TFT_CYAN);
    tftDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
    tftDisplay.setTextSize(2);
    drawCenteredText(160, "STATUS:", TFT_GREEN, 2);
    drawCenteredText(190, status, TFT_GREEN, 2);
    
    currentState = DISPLAY_IDLE;
    lastUpdateTime = millis();
}

void DisplayManager::showScheduledScreen(uint8_t hour, uint8_t minute, uint8_t nextHour, uint8_t nextMinute, const char* medicine) {
    clear();
    drawHeader("NEXT DOSE");
    
    // Current time
    tftDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    tftDisplay.setTextSize(2);
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
    drawCenteredText(50, timeStr, TFT_WHITE, 2);
    
    drawSeparator(85, TFT_CYAN);
    
    // Next dose time
    tftDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDisplay.setTextSize(2);
    drawCenteredText(110, "NEXT:", TFT_YELLOW, 2);
    
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", nextHour, nextMinute);
    tftDisplay.setTextSize(3);
    drawCenteredText(150, timeStr, TFT_RED, 3);
    
    // Medicine info
    tftDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
    tftDisplay.setTextSize(1);
    drawCenteredText(200, medicine, TFT_GREEN, 2);
    
    currentState = DISPLAY_SCHEDULED;
    lastUpdateTime = millis();
}

void DisplayManager::showDispensingScreen(uint8_t medicineNumber) {
    clear();
    drawHeader("DISPENSING");
    
    // Dispensing status
    tftDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDisplay.setTextSize(2);
    drawCenteredText(100, "PLEASE WAIT...", TFT_YELLOW, 2);
    
    // Medicine number
    char medStr[20];
    snprintf(medStr, sizeof(medStr), "Medicine #%d", medicineNumber);
    tftDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
    drawCenteredText(160, medStr, TFT_CYAN, 2);
    
    // Animated progress indicator
    tftDisplay.fillRect(50, 220, 220, 4, TFT_BLUE);
    
    currentState = DISPLAY_DISPENSING;
    lastUpdateTime = millis();
}

void DisplayManager::showErrorScreen(const char* errorMsg) {
    clear();
    drawHeader("ERROR");
    
    // Error icon (simple X)
    tftDisplay.setTextColor(TFT_RED, TFT_BLACK);
    tftDisplay.setTextSize(4);
    drawCenteredText(60, "X", TFT_RED, 4);
    
    drawSeparator(130, TFT_RED);
    
    // Error message
    tftDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    tftDisplay.setTextSize(2);
    drawCenteredText(160, errorMsg, TFT_WHITE, 2);
    
    currentState = DISPLAY_ERROR;
    lastUpdateTime = millis();
}

void DisplayManager::showMaintenanceScreen() {
    clear();
    drawHeader("MAINTENANCE");
    
    tftDisplay.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tftDisplay.setTextSize(2);
    drawCenteredText(100, "SYSTEM", TFT_MAGENTA, 2);
    drawCenteredText(140, "MAINTENANCE", TFT_MAGENTA, 2);
    drawCenteredText(180, "IN PROGRESS", TFT_MAGENTA, 2);
    
    currentState = DISPLAY_IDLE;
    lastUpdateTime = millis();
}

void DisplayManager::displayTime(uint8_t hour, uint8_t minute) {
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
    tftDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    tftDisplay.setTextSize(2);
    tftDisplay.drawText(10, 50, timeStr, TFT_YELLOW, 2);
}

void DisplayManager::displayStatus(const char* status) {
    tftDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
    tftDisplay.setTextSize(2);
    tftDisplay.drawText(10, 150, status, TFT_GREEN, 2);
}

void DisplayManager::displayMessage(const char* msg, uint16_t color) {
    tftDisplay.setTextColor(color, TFT_BLACK);
    tftDisplay.setTextSize(1);
    int16_t textWidth = strlen(msg) * 6;
    int16_t x = (tftDisplay.width() - textWidth) / 2;
    tftDisplay.drawText(x, tftDisplay.height() / 2, msg, color, 1);
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
    tftDisplay.drawPixel(x, y, color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    tftDisplay.drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tftDisplay.drawRect(x, y, w, h, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tftDisplay.fillRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    tftDisplay.drawCircle(x0, y0, r, color);
}

void DisplayManager::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    tftDisplay.fillCircle(x0, y0, r, color);
}

void DisplayManager::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    tftDisplay.setTextColor(color);
    tftDisplay.setTextSize(size);
    tftDisplay.drawText(x, y, text, color);
}
