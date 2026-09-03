#pragma once

#include <Arduino.h>
#include "config.h"
#include "DisplayManager.h"
#include "SensorManager.h"
#include "DoorManager.h"
#include "ServoManager.h"
#include "MotorManager.h"
#include "MedicineVerificationTask.h"
#include "SafetyManager.h"

class DispensingTask {
public:
    enum class Result {
        RUNNING,
        COMPLETE,
        ERROR
    };

    enum class Stage {
        ROTATE_DISPENSER,
        OPEN_DOOR,
        DISPENSE_DOSE,
        START_CONVEYOR,
        CHECK_MEDICINE_IR,
        STOP_CONVEYOR,
        CLOSE_DOOR
    };

    DispensingTask(ServoManager* servoManager, DoorManager* doorManager,
                    MotorManager* motorManager,
                    DisplayManager& displayManager, SensorManager& sensorManager);

    void setSafetyManager(SafetyManager* safetyManager);
    void start(uint8_t activeSection);
    Result update(const char** errorMessageOut);

    Stage getStage() const { return stage; }
    bool isRotationComplete() const { return stage != Stage::ROTATE_DISPENSER; }
    uint32_t getStageElapsedMs() const { return millis() - stageStartTime; }

private:
    ServoManager* servoManager;
    DoorManager* doorManager;
    MotorManager* motorManager;
    DisplayManager& displayManager;
    MedicineVerificationTask medicineVerification;
    SafetyManager* safetyManager;

    Stage stage;
    uint32_t stageStartTime;
    uint8_t targetSection;

    void enterOpenDoorStage();
    void enterDoseStage();
    void enterStartConveyorStage();
    void enterCheckMedicineStage();
    void enterStopConveyorStage();
    void enterCloseDoorStage();
};
