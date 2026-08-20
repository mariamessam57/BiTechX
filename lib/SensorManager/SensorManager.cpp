#include "SensorManager.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

static const char *TAG = "SensorManager";

SensorManager::SensorManager() {}

void SensorManager::begin() {
    const gpio_num_t trig_pin = static_cast<gpio_num_t>(ULTRASONIC_TRIG_PIN);
    const gpio_num_t echo_pin = static_cast<gpio_num_t>(ULTRASONIC_ECHO_PIN);
    const gpio_num_t ir_pin = static_cast<gpio_num_t>(IR_SENSOR_PIN);

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << trig_pin) | (1ULL << echo_pin) | (1ULL << ir_pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << trig_pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(trig_pin, 0);
    ESP_LOGI(TAG, "SensorManager initialized");
}

float SensorManager::readUltrasonicDistance() {
    const gpio_num_t trig_pin = static_cast<gpio_num_t>(ULTRASONIC_TRIG_PIN);
    const gpio_num_t echo_pin = static_cast<gpio_num_t>(ULTRASONIC_ECHO_PIN);
    const int64_t echo_timeout_us = static_cast<int64_t>(ULTRASONIC_ECHO_TIMEOUT_US);

    gpio_set_level(trig_pin, 0);
    esp_rom_delay_us(2);

    gpio_set_level(trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_pin, 0);

    int64_t wait_start_us = esp_timer_get_time();
    while (gpio_get_level(echo_pin) == 0) {
        if ((esp_timer_get_time() - wait_start_us) >= echo_timeout_us) {
            ESP_LOGW(TAG, "Ultrasonic echo timeout waiting for HIGH (%.1f ms)", static_cast<float>(echo_timeout_us) / 1000.0f);
            return -1.0f;
        }
        esp_rom_delay_us(1);
    }

    const int64_t echo_start_us = esp_timer_get_time();
    wait_start_us = echo_start_us;
    while (gpio_get_level(echo_pin) == 1) {
        if ((esp_timer_get_time() - wait_start_us) >= echo_timeout_us) {
            ESP_LOGW(TAG, "Ultrasonic echo timeout waiting for LOW (%.1f ms)", static_cast<float>(echo_timeout_us) / 1000.0f);
            return -1.0f;
        }
        esp_rom_delay_us(1);
    }

    const int64_t echo_end_us = esp_timer_get_time();
    const int64_t pulse_width_us = echo_end_us - echo_start_us;

    if (pulse_width_us <= 0) {
        ESP_LOGE(TAG, "Ultrasonic pulse width invalid: %lld us", pulse_width_us);
        return -1.0f;
    }

    const float distance_cm = (pulse_width_us * 0.0343f) / 2.0f;
    ESP_LOGI(TAG, "Ultrasonic distance: %.2f cm", distance_cm);

    if (distance_cm <= 0.0f) {
        return -1.0f;
    }

    return distance_cm;
}

bool SensorManager::isPersonPresent() {
    const float distance_cm = readUltrasonicDistance();

    if (distance_cm == -1.0f) {
        ESP_LOGW(TAG, "Person not detected: ultrasonic read invalid");
        return false;
    }

    const bool present = (distance_cm > 0.0f) && (distance_cm <= PERSON_DETECTION_DISTANCE_CM);
    if (present) {
        ESP_LOGI(TAG, "Person detected at %.2f cm", distance_cm);
    } else {
        ESP_LOGI(TAG, "Person not detected at %.2f cm", distance_cm);
    }
    return present;
}

bool SensorManager::isMedicineDetected() {
    const gpio_num_t ir_pin = static_cast<gpio_num_t>(IR_SENSOR_PIN);
    const bool detected = (gpio_get_level(ir_pin) == IR_SENSOR_ACTIVE_LEVEL);
    if (detected) {
        ESP_LOGI(TAG, "Medicine detected");
    } else {
        ESP_LOGI(TAG, "Medicine not detected");
    }
    return detected;
}

bool SensorManager::isIrActive() {
    return isMedicineDetected();
}
