#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Config {

//==================================================
// GPIO Definitions
//==================================================
constexpr gpio_num_t PIN_SDA               = GPIO_NUM_21;
constexpr gpio_num_t PIN_SCL               = GPIO_NUM_22;

constexpr gpio_num_t PIN_UART_TX           = GPIO_NUM_1;
constexpr gpio_num_t PIN_UART_RX           = GPIO_NUM_3;

constexpr gpio_num_t PIN_ULTRASONIC_TRIG   = GPIO_NUM_4;
constexpr gpio_num_t PIN_ULTRASONIC_ECHO   = GPIO_NUM_5;

constexpr gpio_num_t PIN_IR_SENSOR         = GPIO_NUM_18;
constexpr gpio_num_t PIN_BUZZER            = GPIO_NUM_19;

constexpr gpio_num_t PIN_RESET_BTN         = GPIO_NUM_25;
constexpr gpio_num_t PIN_MANUAL_BTN        = GPIO_NUM_26;
constexpr gpio_num_t PIN_LIMIT_SWITCH      = GPIO_NUM_27;

constexpr gpio_num_t PIN_RGB_RED           = GPIO_NUM_12;
constexpr gpio_num_t PIN_RGB_GREEN         = GPIO_NUM_13;
constexpr gpio_num_t PIN_RGB_BLUE          = GPIO_NUM_14;


//==================================================
// I2C Configuration
//==================================================
constexpr i2c_port_t I2C_PORT_NUM          = I2C_NUM_0;
constexpr uint32_t I2C_FREQ_HZ             = 100000;

constexpr uint8_t LCD_I2C_ADDR             = 0x27;
constexpr uint8_t PCA9685_I2C_ADDR         = 0x40;
constexpr uint8_t DS3231_I2C_ADDR          = 0x68;


//==================================================
// UART Configuration
//==================================================
constexpr uart_port_t UART_PORT_NUM        = UART_NUM_2;
constexpr int DFPLAYER_BAUD_RATE           = 9600;


//==================================================
// PCA9685 Servo Channels
//==================================================
constexpr uint8_t SERVO_CHANNEL_DOOR       = 0;
constexpr uint8_t SERVO_CHANNEL_MAIN       = 1;
constexpr uint8_t SERVO_CHANNEL_MED_1      = 2;
constexpr uint8_t SERVO_CHANNEL_MED_2      = 3;
constexpr uint8_t SERVO_CHANNEL_MED_3      = 4;
constexpr uint8_t SERVO_CHANNEL_MED_4      = 5;


//==================================================
// Servo Angles (Degrees)
//==================================================
constexpr float ANGLE_DOOR_CLOSED          = 0.0f;
constexpr float ANGLE_DOOR_OPEN            = 90.0f;

constexpr float ANGLE_MED_IDLE             = 0.0f;
constexpr float ANGLE_MED_DISPENSE         = 120.0f;

constexpr uint8_t TOTAL_MEDICINE_SLOTS     = 4;

constexpr float MAIN_SLOT_ANGLES[TOTAL_MEDICINE_SLOTS] =
{
    0.0f,
    45.0f,
    90.0f,
    135.0f
};


//==================================================
// RGB LED
//==================================================
struct RgbColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

constexpr RgbColor COLOR_IDLE        = {0,   0,   255};
constexpr RgbColor COLOR_REMINDER    = {0,   255, 0};
constexpr RgbColor COLOR_SUCCESS     = {255, 255, 255};
constexpr RgbColor COLOR_ERROR       = {255, 0,   0};
constexpr RgbColor COLOR_DISPENSING  = {255, 165, 0};


//==================================================
// LCD Configuration
//==================================================
constexpr uint8_t LCD_ROWS = 2;
constexpr uint8_t LCD_COLS = 16;


//==================================================
// Thresholds & Timeouts
//==================================================
constexpr float ULTRASONIC_THRESHOLD_CM = 40.0f;

constexpr uint8_t RETRY_LIMIT = 3;

constexpr TickType_t PATIENT_TIMEOUT =
    pdMS_TO_TICKS(30000);

constexpr TickType_t DOOR_TIMEOUT =
    pdMS_TO_TICKS(5000);


//==================================================
// Medicine Schedule
//==================================================
struct MedicineScheduleItem
{
    uint8_t hour;
    uint8_t minute;

    uint8_t medicine_id;

    uint8_t slot_number;

    uint8_t dose_count;
};

constexpr uint8_t SCHEDULE_ITEMS_COUNT = 3;

constexpr MedicineScheduleItem MEDICINE_SCHEDULE[SCHEDULE_ITEMS_COUNT] =
{
    {8,  0, 1, 0, 1},
    {14, 0, 2, 1, 2},
    {20, 0, 3, 2, 1}
};


//==================================================
// FreeRTOS Stack Sizes
//==================================================
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_RTC          = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_SCHEDULER    = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_SENSOR       = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_DISPLAY      = 3072;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_AUDIO        = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_DISPENSING   = 4096;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_BUTTON       = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_ERROR        = 2048;
constexpr configSTACK_DEPTH_TYPE STACK_SIZE_SYSTEM       = 4096;


//==================================================
// FreeRTOS Priorities
//==================================================
constexpr UBaseType_t PRIORITY_RTC          = 3;
constexpr UBaseType_t PRIORITY_SCHEDULER    = 3;
constexpr UBaseType_t PRIORITY_SENSOR       = 4;
constexpr UBaseType_t PRIORITY_DISPLAY      = 2;
constexpr UBaseType_t PRIORITY_AUDIO        = 2;
constexpr UBaseType_t PRIORITY_DISPENSING   = 4;
constexpr UBaseType_t PRIORITY_BUTTON       = 4;
constexpr UBaseType_t PRIORITY_ERROR        = 5;
constexpr UBaseType_t PRIORITY_SYSTEM       = 5;


//==================================================
// Feature Flags
//==================================================
constexpr bool FEATURE_BUZZER_ENABLED   = true;
constexpr bool FEATURE_DFPLAYER_ENABLED = true;

} // namespace Config