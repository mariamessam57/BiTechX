#pragma once

#include <Arduino.h>
#include "config.h"
#include "TimeManager.h"

// MedicineTask owns the medicine schedule table and decides, based on the
// RTC, whether a scheduled dose is due right now. It does NOT talk to any
// sensor, servo, display, or audio hardware directly - it only reasons about
// time and which dispenser section is due.
class MedicineTask {
public:
    explicit MedicineTask(TimeManager& timeManager);

    void addSchedule(uint8_t hour, uint8_t minute, uint8_t section);
    void clearSchedules();

    // Returns true if a schedule is triggered right now (and has not already
    // been triggered for this exact minute). On true, outSection receives the
    // dispenser section associated with the triggered schedule.
    bool checkSchedule(uint8_t& outSection);

private:
    struct MedicineSchedule {
        uint8_t hour;
        uint8_t minute;
        uint8_t section;
    };

    static const uint8_t MAX_SCHEDULES = 4;

    TimeManager& timeManager;
    MedicineSchedule schedules[MAX_SCHEDULES];
    uint8_t scheduleCount;
    uint32_t lastTriggeredScheduleKey;

    uint32_t buildScheduleOccurrenceKey(uint8_t hour, uint8_t minute);
    bool isCurrentTriggeredMinute(uint8_t hour, uint8_t minute);
    bool isScheduleTriggered(const MedicineSchedule& schedule);
};
