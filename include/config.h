#pragma once

#include <stdint.h>

// =============================
// I²C Configuration (RTC & Display I²C Devices)
// RTC: DS3231 (Real-Time Clock for medication scheduling)
// Protocol: I²C (Two-wire: SDA + SCL)
// Standard ESP32 I²C pins (Wire library defaults)
// =============================
#define RTC_I2C_SDA_PIN      21  // I²C SDA line (shared bus)
#define RTC_I2C_SCL_PIN      22  // I²C SCL line (shared bus)
// Note: DS3231 addresses 0x68 on I²C bus
// Multiple I²C devices can share SDA/SCL if using different addresses

// =============================
// SPI Configuration (TFT Display)
// TFT Model: ST7789 (1.47", 172x320 pixels, SPI, 8-pin)
// Protocol: SPI (Four-wire: SCK + MOSI + MISO + CS, plus control pins DC/RST)
// 
// IMPORTANT: The ST7789 module may have a pin labeled "SDA" - this is NOT I²C SDA.
// On the ST7789 in SPI mode, the physical "SDA" label is the SPI MOSI (Master Out Slave In) line.
// Do NOT connect the TFT's "SDA" to the I²C SDA/SCL lines.
// The TFT must use its dedicated SPI pins defined below.
// =============================
#ifndef LOW
#define LOW  0
#endif

#ifndef HIGH
#define HIGH 1
#endif

#define TFT_SPI_HOST         SPI3_HOST
#define TFT_MOSI_PIN         23   // SPI data from ESP32 to TFT (labeled "SDA" on some modules, but is MOSI)
#define TFT_MISO_PIN         19   // SPI data from TFT to ESP32 (read feedback)
#define TFT_SCLK_PIN         18   // SPI clock
#define TFT_CS_PIN           5    // Chip Select (active low)
#define TFT_DC_PIN           26   // Data/Command (1=data, 0=command)
#define TFT_RST_PIN          27   // Reset (active low)
#define TFT_BACKLIGHT_PIN    4    // Backlight PWM

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
// DC Motor (Conveyor Belt) pins
// =============================
// IMPORTANT: These GPIO pins must be assigned based on actual hardware.
// The ESP32 has dual H-bridge motor control capability.
// IN1 and IN2 control direction; PWM on these pins controls speed.
// Verify against actual hardware schematic before deployment.
#define DC_MOTOR_IN1_PIN     12  // Forward direction (PWM-capable)
#define DC_MOTOR_IN2_PIN     13  // Backward direction (PWM-capable)

// =============================
// Sensor pins
// =============================
// The original project did not define any sensor GPIOs, so these were selected
// from the remaining safe ESP32 pins after reserving TFT, audio UART, and servo pins.
// TRIG must be output-capable and the ECHO signal on HC-SR04-style modules can be
// 5 V; use a proper voltage divider / level shifter if the module is wired directly.
// IMPORTANT: ULTRASONIC_TRIG_PIN MUST NOT be GPIO 21 (RTC I²C SDA).
// Using GPIO 20 for TRIG to avoid conflicts.
#define ULTRASONIC_TRIG_PIN          20
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
// Safety and actuator limits
// =============================
#define SAFETY_CHECK_INTERVAL_MS          100UL
#define DC_MOTOR_MAX_RUNTIME_MS           3000UL
#define DISPENSER_OPERATION_TIMEOUT_MS    5000UL
#define DOOR_OPERATION_TIMEOUT_MS         3000UL

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




#define WIFI_SSID "اسم الشبكة"
#define WIFI_PASSWORD "الباسورد"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883

// =============================
// BiTechX MQTT Configuration
// =============================

#define MQTT_DEVICE_ID "BiTechX_001"

// ESP32 → Dashboard
#define MQTT_TOPIC_TELEMETRY "bitechx/device/telemetry"
#define MQTT_TOPIC_STATUS    "bitechx/device/status"
#define MQTT_TOPIC_ALERTS    "bitechx/device/alerts"

// Dashboard → ESP32
#define MQTT_TOPIC_COMMANDS  "bitechx/device/commands"