#pragma once

#include <Arduino.h>
#include "config.h"
#include "DisplayManager.h"
#include "TimeManager.h"
#include "SensorManager.h"
#include "AudioManager.h"
#include "DoorManager.h"
#include "ServoManager.h"
#include "MotorManager.h"
#include "SafetyManager.h"
#include "../tasks/MedicineTask.h"
#include "../tasks/PersonDetectionTask.h"
#include "../tasks/DispensingTask.h"

// Top-level states owned by the controller. The five granular dispensing
// sub-steps (rotate/door/dose/IR/close) are intentionally NOT modeled here -
// they are owned exclusively by DispensingTask so there is a single owner
// for that sequence instead of two state machines describing the same thing.
enum class DispenserState {
    IDLE,
    CHECK_TIME,
    ALERT,
    WAIT_FOR_PERSON,
    DISPENSING,
    ERROR
};

// DispenserController is the high-level coordinator. It owns the hardware
// managers and the task objects, and moves between the top-level states by
// delegating to the appropriate task. It does not implement any ultrasonic
// calculation, IR reading, servo PWM, RTC hardware, or display drawing
// itself - those all stay inside their respective manager/task.
class DispenserController {
private:
    DisplayManager displayManager;
    TimeManager timeManager;
    SensorManager sensorManager;
    AudioManager* audioManager;
    DoorManager* doorManager;
    ServoManager* servoManager;
    MotorManager* motorManager;
    SafetyManager safetyManager;

    MedicineTask* medicineTask;
    PersonDetectionTask* personDetectionTask;
    DispensingTask* dispensingTask;

    DispenserState currentState;
    DispenserState previousState;
    uint32_t stateStartTime;
    uint32_t lastTimeCheck;
    uint8_t activeSection;

    void changeState(DispenserState nextState);
    void runIdleState();
    void runCheckTimeState();
    void runAlertState();
    void runWaitForPersonState();
    void runDispensingState();
    void runErrorState();
    void resetToIdle();
    void triggerError(const char* message);

public:
    DispenserController();
    void begin();
    void loop();

    DisplayManager& getDisplayManager() { return displayManager; }
    void addMedicineSchedule(uint8_t hour, uint8_t minute, uint8_t section);
    void clearSchedules();
};
