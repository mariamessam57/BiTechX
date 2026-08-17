#pragma once

#include <Arduino.h>
#include "TFT.h"

// Display state
enum DisplayState {
    DISPLAY_IDLE,
    DISPLAY_DISPENSING,
    DISPLAY_ERROR,
    DISPLAY_SCHEDULED
};

class DisplayManager {
private:
    TFT tftDisplay;
    DisplayState currentState;
    uint32_t lastUpdateTime;
    
    // UI drawing helpers
    void drawHeader(const char* title);
    void drawCenteredText(int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    void drawSeparator(int16_t y, uint16_t color);
    
public:
    DisplayManager();
    
    // Initialization
    void begin();
    void end();
    
    // Display control
    void clear();
    void turnOn();
    void turnOff();
    void setRotation(uint8_t r);
    
    // State updates
    void setState(DisplayState state);
    DisplayState getState() const;
    
    // UI Update methods
    void showIdleScreen(uint8_t hour, uint8_t minute, const char* status = "READY");
    void showScheduledScreen(uint8_t hour, uint8_t minute, uint8_t nextHour, uint8_t nextMinute, const char* medicine);
    void showDispensingScreen(uint8_t medicineNumber);
    void showErrorScreen(const char* errorMsg);
    void showMaintenanceScreen();
    
    // Simple status display
    void displayTime(uint8_t hour, uint8_t minute);
    void displayStatus(const char* status);
    void displayMessage(const char* msg, uint16_t color = TFT_WHITE);
    
    // Low-level drawing (for custom screens)
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color = TFT_WHITE, uint8_t size = 1);
    
    // Getters
    uint16_t width() const { return tftDisplay.width(); }
    uint16_t height() const { return tftDisplay.height(); }
};
