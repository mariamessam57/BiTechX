#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

// TFT controller type (ST7789 - 1.47" 172x320 display)
#define ST7789_TFTWIDTH   172
#define ST7789_TFTHEIGHT  320

// ST7789 Commands
#define ST7789_NOP         0x00
#define ST7789_SWRESET     0x01
#define ST7789_RDDID       0x04
#define ST7789_RDDST       0x09
#define ST7789_SLPIN       0x10
#define ST7789_SLPOUT      0x11
#define ST7789_PTLON       0x12
#define ST7789_NORON       0x13
#define ST7789_INVOFF      0x20
#define ST7789_INVON       0x21
#define ST7789_DISPOFF     0x28
#define ST7789_DISPON      0x29
#define ST7789_CASET       0x2A
#define ST7789_RASET       0x2B
#define ST7789_RAMWR       0x2C
#define ST7789_RAMRD       0x2E
#define ST7789_PTLAR       0x30
#define ST7789_COLMOD      0x3A
#define ST7789_MADCTL      0x36
#define ST7789_FRMCTR1     0xB1
#define ST7789_FRMCTR2     0xB2
#define ST7789_FRMCTR3     0xB3
#define ST7789_INVCTR      0xB4
#define ST7789_DISSET5     0xB6
#define ST7789_GCTRL       0xB7
#define ST7789_GTADJ       0xB9
#define ST7789_VCOMS       0xBB
#define ST7789_LCMCTRL     0xC0
#define ST7789_IDSET       0xC1
#define ST7789_VDVVRHEN    0xC2
#define ST7789_VRHS        0xC3
#define ST7789_VDVSET      0xC4
#define ST7789_FRCTR2      0xC6
#define ST7789_PWCTRL1     0xD0
#define ST7789_PWCTRL2     0xD1
#define ST7789_PWCTRL3     0xD2
#define ST7789_GMCTRP1     0xE0
#define ST7789_GMCTRN1     0xE1

// RGB565 color macros (matches config.h style)
#define TFT_BLACK      0x0000
#define TFT_WHITE      0xFFFF
#define TFT_RED        0xF800
#define TFT_GREEN      0x07E0
#define TFT_BLUE       0x001F
#define TFT_YELLOW     0xFFE0
#define TFT_CYAN       0x07FF
#define TFT_MAGENTA    0xF81F

class TFT {
private:
    SPIClass* spi;
    uint16_t _width;
    uint16_t _height;
    uint8_t _rotation;
    uint16_t _textColor;
    uint16_t _bgColor;
    uint8_t _textSize;
    
#if TFT_USE_MUTEX
    SemaphoreHandle_t spiMutex;
#endif

    // SPI communication
    void writeCommand(uint8_t cmd);
    void writeData(const uint8_t* data, size_t len);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    
    // Initialization
    void hardwareReset();
    void initDisplay();
    
    // Internal drawing
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    
    // Helper to get lock
    inline void acquireLock() {
#if TFT_USE_MUTEX
        if (spiMutex) {
            xSemaphoreTake(spiMutex, portMAX_DELAY);
        }
#endif
    }
    
    inline void releaseLock() {
#if TFT_USE_MUTEX
        if (spiMutex) {
            xSemaphoreGive(spiMutex);
        }
#endif
    }

public:
    TFT();
    ~TFT();
    
    // Initialization
    void begin(uint32_t freq = TFT_SPI_FREQUENCY);
    void end();
    
    // Display control
    void displayOn();
    void displayOff();
    void setRotation(uint8_t r);
    void invertDisplay(boolean i);
    
    // Clearing and filling
    void fillScreen(uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    // Drawing primitives
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    
    // Text drawing
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setTextSize(uint8_t s);
    void setCursor(int16_t x, int16_t y);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawText(int16_t x, int16_t y, const String& text);
    void drawText(int16_t x, int16_t y, const String& text, uint16_t color, uint8_t size = 1);
    void drawText(int16_t x, int16_t y, const char* text);
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    
    // Getters
    uint16_t width() const { return _width; }
    uint16_t height() const { return _height; }
    uint8_t getRotation() const { return _rotation; }
};
