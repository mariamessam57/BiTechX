#pragma once

#include <Arduino.h>
#include "config.h"
#include "DisplayManager.h"
#include "TimeManager.h"
#include "SensorManager.h"
#include "AudioManager.h"
#include "DoorManager.h"
#include "ServoManager.h"

enum class DispenserState {
    IDLE,
    CHECK_TIME,
    ALERT,
    WAIT_FOR_PERSON,
    OPEN_DOOR,
    ROTATE_DISPENSER,
    DISPENSE_DOSE_SERVO,
    CHECK_MEDICINE_IR,
    CLOSE_DOOR,
    ERROR
};

class Tasks {
private:
    DisplayManager displayManager;
    TimeManager timeManager;
    SensorManager sensorManager;
    AudioManager* audioManager;
    DoorManager* doorManager;
    ServoManager* servoManager;

    DispenserState currentState;
    DispenserState previousState;
    uint32_t stateStartTime;
    uint32_t lastTimeCheck;
    uint32_t personDetectionStartTime;
    uint32_t lastPersonDetectionAttemptMs;
    uint8_t personDetectionConfirmCount;
    uint32_t medicineVerifyStartTime;
    uint32_t lastMedicineVerifyAttemptMs;
    uint8_t medicineDetectionConfirmCount;
    uint8_t activeSection;
    uint32_t lastTriggeredScheduleKey;
    bool isCurrentTriggeredMinute(uint8_t hour, uint8_t minute);
    void resetPersonDetectionState();
    void resetMedicineVerifyState();
    uint32_t buildScheduleOccurrenceKey(uint8_t hour, uint8_t minute);

    struct MedicineSchedule {
        uint8_t hour;
        uint8_t minute;
        uint8_t section;
    };

    static const uint8_t MAX_SCHEDULES = 4;
    MedicineSchedule schedules[MAX_SCHEDULES];
    uint8_t scheduleCount;

    bool isScheduleTriggered(const MedicineSchedule& schedule);
    bool isMedicineTimeNow();
    void changeState(DispenserState nextState);
    void runIdleState();
    void runCheckTimeState();
    void runAlertState();
    void runWaitForPersonState();
    void runOpenDoorState();
    void runRotateDispenserState();
    void runDispenseDoseServoState();
    void runCheckMedicineIRState();
    void runCloseDoorState();
    void runErrorState();
    void resetToIdle();
    void triggerError(const char* message);

public:
    Tasks();
    void begin();
    void loop();

    DisplayManager& getDisplayManager() { return displayManager; }
    void addMedicineSchedule(uint8_t hour, uint8_t minute, uint8_t section);
    void clearSchedules();
};
