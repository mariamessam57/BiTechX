#include "MedicineTask.h"

MedicineTask::MedicineTask(TimeManager& timeManager)
    : timeManager(timeManager),
      scheduleCount(0),
      lastTriggeredScheduleKey(0xFFFFFFFFUL) {
}

void MedicineTask::addSchedule(uint8_t hour, uint8_t minute, uint8_t section) {
    if (scheduleCount >= MAX_SCHEDULES) {
        return;
    }

    schedules[scheduleCount].hour = hour;
    schedules[scheduleCount].minute = minute;
    schedules[scheduleCount].section = section;
    scheduleCount++;
}

void MedicineTask::clearSchedules() {
    scheduleCount = 0;
    lastTriggeredScheduleKey = 0xFFFFFFFFUL;
}

uint32_t MedicineTask::buildScheduleOccurrenceKey(uint8_t hour, uint8_t minute) {
    uint8_t currentDay = 0;
    uint8_t currentMonth = 0;
    uint16_t currentYear = 0;
    uint8_t currentHour = 0;
    uint8_t currentMinute = 0;
    uint8_t currentSecond = 0;

    if (!timeManager.getCurrentTime(currentHour, currentMinute, currentSecond, currentDay, currentMonth, currentYear)) {
        return 0xFFFFFFFFUL;
    }

    const uint32_t dateKey = (static_cast<uint32_t>(currentYear) * 10000UL) +
                             (static_cast<uint32_t>(currentMonth) * 100UL) +
                             static_cast<uint32_t>(currentDay);
    return (dateKey * 10000UL) + (static_cast<uint32_t>(hour) * 100UL) + static_cast<uint32_t>(minute);
}

bool MedicineTask::isCurrentTriggeredMinute(uint8_t hour, uint8_t minute) {
    if (!timeManager.isRtcValid()) {
        Serial.println("[MedicineTask] RTC invalid; schedule check skipped");
        return false;
    }

    const uint32_t scheduleKey = buildScheduleOccurrenceKey(hour, minute);
    if (scheduleKey == 0xFFFFFFFFUL) {
        return false;
    }

    if (!timeManager.isScheduledTime(hour, minute)) {
        if (lastTriggeredScheduleKey == scheduleKey) {
            lastTriggeredScheduleKey = 0xFFFFFFFFUL;
        }
        return false;
    }

    if (scheduleKey == lastTriggeredScheduleKey) {
        return false;
    }

    return true;
}

bool MedicineTask::isScheduleTriggered(const MedicineSchedule& schedule) {
    return isCurrentTriggeredMinute(schedule.hour, schedule.minute);
}

bool MedicineTask::checkSchedule(uint8_t& outSection) {
    for (uint8_t i = 0; i < scheduleCount; ++i) {
        if (isScheduleTriggered(schedules[i])) {
            if (!timeManager.isRtcValid()) {
                Serial.println("[MedicineTask] RTC invalid; refusing to trigger medicine schedule");
                return false;
            }

            outSection = schedules[i].section;
            uint32_t triggerKey = buildScheduleOccurrenceKey(schedules[i].hour, schedules[i].minute);
            lastTriggeredScheduleKey = triggerKey;
            Serial.printf("[MedicineTask] Medicine time match: %02d:%02d section=%u\n",
                          schedules[i].hour, schedules[i].minute, outSection);
            return true;
        }
    }

    return false;
}
