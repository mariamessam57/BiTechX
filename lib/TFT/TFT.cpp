#include "TFT.h"
#include "config.h"

// Simple 5x7 font for text drawing
static const uint8_t font5x7[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
    0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
    0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
    0x18, 0x3C, 0x7E, 0x3C, 0x18,
    0x1C, 0x57, 0x7D, 0x57, 0x1C,
    0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
    0x00, 0x18, 0x3C, 0x18, 0x00,
    0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
    0x00, 0x18, 0x24, 0x18, 0x00,
    0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
    0x30, 0x48, 0x3A, 0x06, 0x0E,
    0x26, 0x29, 0x29, 0x29, 0x32,
    0x48, 0x7E, 0x48, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x55, 0x22, 0x50,
    0x00, 0x05, 0x03, 0x00, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x14, 0x08, 0x3E, 0x08, 0x14,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x50, 0x30, 0x00, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x60, 0x60, 0x00, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x26, 0x49, 0x49, 0x49, 0x3E,
    0x00, 0x36, 0x36, 0x00, 0x00,
    0x00, 0x56, 0x36, 0x00, 0x00,
    0x08, 0x14, 0x22, 0x41, 0x00,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
};

TFT::TFT() 
    : spi(NULL), _width(TFT_WIDTH), _height(TFT_HEIGHT), 
      _rotation(TFT_DEFAULT_ROTATION), _textColor(TFT_WHITE), 
      _bgColor(TFT_BLACK), _textSize(1) {
#if TFT_USE_MUTEX
    spiMutex = xSemaphoreCreateMutex();
#endif
}

TFT::~TFT() {
#if TFT_USE_MUTEX
    if (spiMutex) {
        vSemaphoreDelete(spiMutex);
    }
#endif
}

void TFT::begin(uint32_t freq) {
    // Configure GPIO pins
    pinMode(TFT_DC_PIN, OUTPUT);
    pinMode(TFT_CS_PIN, OUTPUT);
    pinMode(TFT_RST_PIN, OUTPUT);
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    
    // Initialize SPI
    SPI.begin(TFT_SCLK_PIN, TFT_MISO_PIN, TFT_MOSI_PIN, TFT_CS_PIN);
    SPI.setFrequency(freq);
    SPI.setDataMode(SPI_MODE0);
    spi = &SPI;
    
    // Hardware reset
    hardwareReset();
    
    // Initialize display
    initDisplay();
    
    // Enable backlight
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
}

void TFT::end() {
    digitalWrite(TFT_BACKLIGHT_PIN, LOW);
    SPI.end();
}

void TFT::hardwareReset() {
    digitalWrite(TFT_RST_PIN, LOW);
    delay(10);
    digitalWrite(TFT_RST_PIN, HIGH);
    delay(120);
}

void TFT::initDisplay() {
    // ST7789 initialization sequence for 1.47" 172x320 display
    
    // Software Reset
    writeCommand(ST7789_SWRESET);
    delay(150);
    
    // Sleep Out
    writeCommand(ST7789_SLPOUT);
    delay(120);
    
    // Pixel Format: 16-bit RGB565
    writeCommand(ST7789_COLMOD);
    writeData(0x55);
    delay(10);
    
    // Memory Data Access Control (MADCTL)
    // bit7=MY, bit6=MX, bit5=MV, bit4=ML, bit3=RGB/BGR, bit2=MH
    writeCommand(ST7789_MADCTL);
    writeData(0x00);  // Default orientation
    
    // Column Address Set (CASET) - 172 pixel width
    writeCommand(ST7789_CASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0xAB);  // 172 = 0xAC (0-based indexing)
    
    // Row Address Set (RASET) - 320 pixel height
    writeCommand(ST7789_RASET);
    writeData(0x00);
    writeData(0x00);
    writeData(0x01);
    writeData(0x3F);  // 320 = 0x140 (0-based indexing)
    
    // Frame Rate Control
    writeCommand(ST7789_FRMCTR2);
    writeData(0x0C);
    writeData(0x0C);
    writeData(0x00);
    writeData(0x33);
    writeData(0x33);
    delay(10);
    
    // Gate Control
    writeCommand(ST7789_GCTRL);
    writeData(0x75);
    
    // VCOMS Setting
    writeCommand(ST7789_VCOMS);
    writeData(0x19);
    
    // LCM Control
    writeCommand(ST7789_LCMCTRL);
    writeData(0x2C);
    
    // VDV and VRH Command Enable
    writeCommand(ST7789_VDVVRHEN);
    writeData(0x01);
    
    // VRH Set (Voltage Regulator High)
    writeCommand(ST7789_VRHS);
    writeData(0x12);
    
    // VDV Set (Voltage Vdd Vss Set)
    writeCommand(ST7789_VDVSET);
    writeData(0x20);
    
    // Frame Rate Control in Normal Mode
    writeCommand(ST7789_FRCTR2);
    writeData(0x0F);
    delay(10);
    
    // Power Control 1
    writeCommand(ST7789_PWCTRL1);
    writeData(0xA4);
    writeData(0xA1);
    
    // Positive Voltage Gamma Correction (PVGAMCTRL)
    writeCommand(ST7789_GMCTRP1);
    writeData(0xD0);
    writeData(0x04);
    writeData(0x0D);
    writeData(0x11);
    writeData(0x13);
    writeData(0x2B);
    writeData(0x3F);
    writeData(0x54);
    writeData(0x4C);
    writeData(0x18);
    writeData(0x0D);
    writeData(0x0B);
    writeData(0x1F);
    writeData(0x23);
    
    // Negative Voltage Gamma Correction (NVGAMCTRL)
    writeCommand(ST7789_GMCTRN1);
    writeData(0xD0);
    writeData(0x04);
    writeData(0x0C);
    writeData(0x11);
    writeData(0x13);
    writeData(0x2C);
    writeData(0x3F);
    writeData(0x44);
    writeData(0x51);
    writeData(0x2F);
    writeData(0x1F);
    writeData(0x1F);
    writeData(0x20);
    writeData(0x23);
    delay(10);
    
    // Sleep Out
    writeCommand(ST7789_SLPOUT);
    delay(120);
    
    // Display ON
    writeCommand(ST7789_DISPON);
    delay(120);
    
    // Apply rotation setting
    setRotation(_rotation);
}

void TFT::writeCommand(uint8_t cmd) {
    acquireLock();
    digitalWrite(TFT_DC_PIN, LOW);
    digitalWrite(TFT_CS_PIN, LOW);
    SPI.write(cmd);
    digitalWrite(TFT_CS_PIN, HIGH);
    digitalWrite(TFT_DC_PIN, HIGH);
    releaseLock();
}

void TFT::writeData(uint8_t data) {
    digitalWrite(TFT_DC_PIN, HIGH);
    digitalWrite(TFT_CS_PIN, LOW);
    SPI.write(data);
    digitalWrite(TFT_CS_PIN, HIGH);
}

void TFT::writeData(const uint8_t* data, size_t len) {
    digitalWrite(TFT_DC_PIN, HIGH);
    digitalWrite(TFT_CS_PIN, LOW);
    for (size_t i = 0; i < len; i++) {
        SPI.write(data[i]);
    }
    digitalWrite(TFT_CS_PIN, HIGH);
}

void TFT::writeData16(uint16_t data) {
    digitalWrite(TFT_DC_PIN, HIGH);
    digitalWrite(TFT_CS_PIN, LOW);
    SPI.write(data >> 8);
    SPI.write(data & 0xFF);
    digitalWrite(TFT_CS_PIN, HIGH);
}

void TFT::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(ST7789_CASET);
    writeData16(x0);
    writeData16(x1);
    
    writeCommand(ST7789_RASET);
    writeData16(y0);
    writeData16(y1);
    
    writeCommand(ST7789_RAMWR);
}

void TFT::displayOn() {
    writeCommand(ST7789_DISPON);
}

void TFT::displayOff() {
    writeCommand(ST7789_DISPOFF);
}

void TFT::setRotation(uint8_t r) {
    acquireLock();
    _rotation = r % 4;
    
    // ST7789 MADCTL register values for different rotations
    // 172x320 display orientation
    switch (_rotation) {
        case 0:
            // 0°: 172 width, 320 height
            writeCommand(ST7789_MADCTL);
            writeData(0x00);
            _width = 172;
            _height = 320;
            break;
        case 1:
            // 90°: 320 width, 172 height
            writeCommand(ST7789_MADCTL);
            writeData(0xA0);
            _width = 320;
            _height = 172;
            break;
        case 2:
            // 180°: 172 width, 320 height
            writeCommand(ST7789_MADCTL);
            writeData(0xC0);
            _width = 172;
            _height = 320;
            break;
        case 3:
            // 270°: 320 width, 172 height
            writeCommand(ST7789_MADCTL);
            writeData(0x60);
            _width = 320;
            _height = 172;
            break;
    }
    releaseLock();
}

void TFT::invertDisplay(boolean i) {
    writeCommand(i ? ST7789_INVON : ST7789_INVOFF);
}

void TFT::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

void TFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    acquireLock();
    
    if ((x >= _width) || (y >= _height)) {
        releaseLock();
        return;
    }
    
    if ((x + w - 1) >= _width) {
        w = _width - x;
    }
    if ((y + h - 1) >= _height) {
        h = _height - y;
    }
    
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    
    digitalWrite(TFT_DC_PIN, HIGH);
    digitalWrite(TFT_CS_PIN, LOW);
    
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        SPI.write(hi);
        SPI.write(lo);
    }
    
    digitalWrite(TFT_CS_PIN, HIGH);
    releaseLock();
}

void TFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) {
        return;
    }
    
    acquireLock();
    setAddressWindow(x, y, x, y);
    writeData16(color);
    releaseLock();
}

void TFT::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if ((x < 0) || (x >= _width) || (y >= _height)) {
        return;
    }
    
    if ((y + h - 1) >= _height) {
        h = _height - y;
    }
    
    if (h < 1) {
        return;
    }
    
    fillRect(x, y, 1, h, color);
}

void TFT::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if ((y < 0) || (y >= _height) || (x >= _width)) {
        return;
    }
    
    if ((x + w - 1) >= _width) {
        w = _width - x;
    }
    
    if (w < 1) {
        return;
    }
    
    fillRect(x, y, w, 1, color);
}

void TFT::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while (1) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void TFT::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void TFT::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);
    
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void TFT::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircle(x0, y0, r - 1, color);
}

void TFT::setTextColor(uint16_t c) {
    _textColor = c;
}

void TFT::setTextColor(uint16_t c, uint16_t bg) {
    _textColor = c;
    _bgColor = bg;
}

void TFT::setTextSize(uint8_t s) {
    _textSize = (s > 0) ? s : 1;
}

void TFT::setCursor(int16_t x, int16_t y) {
    // Cursor position for text (can be extended)
}

void TFT::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x + 6 * size - 1) >= _width) return;
    if ((y + 8 * size - 1) >= _height) return;
    
    for (int8_t i = 0; i < 5; i++) {
        uint8_t line;
        if (i == 5)
            line = 0x0;
        else
            line = pgm_read_byte(&font5x7[c * 5 + i]);
        
        for (int8_t j = 0; j < 8; j++) {
            if (line & 0x1) {
                if (size == 1)
                    drawPixel(x + i, y + j, color);
                else
                    fillRect(x + i * size, y + j * size, size, size, color);
            } else if (bg != color) {
                if (size == 1)
                    drawPixel(x + i, y + j, bg);
                else
                    fillRect(x + i * size, y + j * size, size, size, bg);
            }
            line >>= 1;
        }
    }
}

void TFT::drawText(int16_t x, int16_t y, const String& text) {
    drawText(x, y, text.c_str());
}

void TFT::drawText(int16_t x, int16_t y, const String& text, uint16_t color, uint8_t size) {
    uint16_t prevColor = _textColor;
    uint8_t prevSize = _textSize;
    _textColor = color;
    _textSize = size;
    drawText(x, y, text.c_str());
    _textColor = prevColor;
    _textSize = prevSize;
}

void TFT::drawText(int16_t x, int16_t y, const char* text) {
    int16_t cursor_x = x;
    
    if (!text) return;
    
    while (*text) {
        if (*text == '\n') {
            cursor_x = x;
            y += 8 * _textSize;
        } else {
            drawChar(cursor_x, y, *text, _textColor, _bgColor, _textSize);
            cursor_x += 6 * _textSize;
        }
        text++;
    }
}

void TFT::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    uint16_t prevColor = _textColor;
    uint8_t prevSize = _textSize;
    _textColor = color;
    _textSize = size;
    drawText(x, y, text);
    _textColor = prevColor;
    _textSize = prevSize;
}

