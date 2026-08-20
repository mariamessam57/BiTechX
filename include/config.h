#pragma once

#include <stdint.h>

// =============================
// SPI bus and TFT hardware config
// TFT Model: ST7789 (1.47", 172x320 pixels, SPI, 8-pin)
// =============================
#ifndef LOW
#define LOW  0
#endif

#ifndef HIGH
#define HIGH 1
#endif

#define TFT_SPI_HOST         SPI3_HOST
#define TFT_MOSI_PIN         23
#define TFT_MISO_PIN         19
#define TFT_SCLK_PIN         18
#define TFT_CS_PIN           5
#define TFT_DC_PIN           26
#define TFT_RST_PIN          27
#define TFT_BACKLIGHT_PIN    4

#define TFT_WIDTH            172
#define TFT_HEIGHT           320
#define TFT_SPI_FREQUENCY    40000000
#define TFT_DEFAULT_ROTATION 1

// =============================
// Optional feature flags
// =============================
#define TFT_USE_MUTEX        1

// =============================
// Audio / dispenser control pins
// =============================
#define AUDIO_SERIAL_RX_PIN  17
#define AUDIO_SERIAL_TX_PIN  16
#define AUDIO_BUSY_PIN       25
#define AUDIO_VOLUME         25

#define DOOR_SERVO_PIN       32
#define DISPENSER_SERVO_PIN  33

// =============================
// Sensor pins
// =============================
// The original project did not define any sensor GPIOs, so these were selected
// from the remaining safe ESP32 pins after reserving TFT, audio UART, and servo pins.
// TRIG must be output-capable and the ECHO signal on HC-SR04-style modules can be
// 5 V; use a proper voltage divider / level shifter if the module is wired directly.
#define ULTRASONIC_TRIG_PIN          21
#define ULTRASONIC_ECHO_PIN          34
#define IR_SENSOR_PIN                35

// Default IR digital polarity. This is kept configurable because the exact module
// can vary between active-LOW and active-HIGH outputs depending on the sensor board.
#define IR_SENSOR_ACTIVE_LEVEL       LOW
#define PERSON_DETECTION_DISTANCE_CM  50.0f
#define PERSON_DETECTION_REQUIRED_READINGS 3U
#define PERSON_DETECTION_TIMEOUT_MS   30000UL
#define PERSON_DETECTION_RETRY_INTERVAL_MS 250UL

// =============================
// Timing configuration
// =============================
#define MEDICINE_CHECK_INTERVAL_MS    10000UL
#define DOOR_OPEN_DELAY_MS            1200UL
#define DISPENSER_ROTATION_DELAY_MS   800UL
#define DOSE_SERVO_DELAY_MS           2000UL
#define DOOR_CLOSE_DELAY_MS           1500UL
#define IR_DETECTION_TIMEOUT_MS       5000UL
#define ULTRASONIC_ECHO_TIMEOUT_US    25000UL

// =============================
// Medicine Verification (IR Sensor)
// =============================
#define MEDICINE_VERIFY_WAIT_MS           500UL
#define MEDICINE_DETECTION_REQUIRED_READINGS 3U
#define MEDICINE_VERIFY_RETRY_INTERVAL_MS 150UL
#define MEDICINE_VERIFY_TIMEOUT_MS       8000UL

// =============================
// Common RGB565 color values
// =============================
#define TFT_COLOR_BLACK      0x0000u
#define TFT_COLOR_WHITE      0xFFFFu
#define TFT_COLOR_RED        0xF800u
#define TFT_COLOR_GREEN      0x07E0u
#define TFT_COLOR_BLUE       0x001Fu
#define TFT_COLOR_YELLOW     0xFFE0u
#define TFT_COLOR_CYAN       0x07FFu
#define TFT_COLOR_MAGENTA    0xF81Fu