#include "TimeManager.h"

#include "esp_log.h"

static const char *TAG = "TimeManager";

TimeManager::TimeManager()
    : rtcInitialized(false),
      rtcValid(false),
      lastGoodTime(2000, 1, 1, 0, 0, 0) {}

bool TimeManager::begin() {
    rtcInitialized = rtc.begin();
    if (!rtcInitialized) {
        rtcValid = false;
        ESP_LOGE(TAG, "RTC initialization failed");
        return false;
    }

    if (rtc.lostPower()) {
        ESP_LOGW(TAG, "RTC lost power; time may be invalid");
    }

    rtcValid = updateTime();
    if (!rtcValid) {
        ESP_LOGE(TAG, "RTC read failed during initial sync");
    } else {
        ESP_LOGI(TAG, "RTC initialized successfully");
    }

    return rtcValid;
}

bool TimeManager::updateTime() {
    if (!rtcInitialized || !rtc.begin()) {
        rtcValid = false;
        ESP_LOGE(TAG, "RTC read failed: device not initialized or not responding");
        return false;
    }

    DateTime now = rtc.now();
    if (now.year() < 2024 || now.year() > 2100) {
        rtcValid = false;
        ESP_LOGE(TAG, "RTC read failed: invalid date/time returned");
        return false;
    }

    rtcValid = true;
    lastGoodTime = now;
    return true;
}

bool TimeManager::getCurrentTime(uint8_t& hour, uint8_t& minute, uint8_t& second,
                                uint8_t& day, uint8_t& month, uint16_t& year) {
    if (!rtcValid) {
        if (!updateTime()) {
            return false;
        }
    }

    DateTime now = rtc.now();
    hour = now.hour();
    minute = now.minute();
    second = now.second();
    day = now.day();
    month = now.month();
    year = static_cast<uint16_t>(now.year());
    return true;
}

bool TimeManager::isRtcValid() const {
    return rtcInitialized && rtcValid;
}

bool TimeManager::isScheduledTime(uint8_t hour, uint8_t minute) {
    uint8_t currentHour = 0;
    uint8_t currentMinute = 0;
    uint8_t currentSecond = 0;
    uint8_t currentDay = 0;
    uint8_t currentMonth = 0;
    uint16_t currentYear = 0;

    if (!getCurrentTime(currentHour, currentMinute, currentSecond, currentDay, currentMonth, currentYear)) {
        return false;
    }

    return (currentHour == hour) && (currentMinute == minute);
}
