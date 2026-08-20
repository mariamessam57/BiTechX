#pragma once

#include <Arduino.h>
#include <RTClib.h>

class TimeManager {
public:
    TimeManager();

    bool begin();
    bool updateTime();
    bool getCurrentTime(uint8_t& hour, uint8_t& minute, uint8_t& second,
                       uint8_t& day, uint8_t& month, uint16_t& year);
    bool isRtcValid() const;
    bool isScheduledTime(uint8_t hour, uint8_t minute);

private:
    RTC_DS3231 rtc;
    bool rtcInitialized;
    bool rtcValid;
    DateTime lastGoodTime;
};
