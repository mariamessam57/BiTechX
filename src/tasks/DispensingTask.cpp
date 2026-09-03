#include "DispensingTask.h"

DispensingTask::DispensingTask(ServoManager* servoManager, DoorManager* doorManager,
                               MotorManager* motorManager,
                               DisplayManager& displayManager, SensorManager& sensorManager)
    : servoManager(servoManager),
      doorManager(doorManager),
      motorManager(motorManager),
      displayManager(displayManager),
      medicineVerification(sensorManager),
      safetyManager(nullptr),
      stage(Stage::ROTATE_DISPENSER),
      stageStartTime(0),
      targetSection(0) {
}

void DispensingTask::setSafetyManager(SafetyManager* manager) {
    safetyManager = manager;
    medicineVerification.setSafetyManager(manager);
}

void DispensingTask::start(uint8_t activeSection) {
    targetSection = activeSection + 1U;
    stage = Stage::ROTATE_DISPENSER;
    stageStartTime = millis();

    if (targetSection < 1 || targetSection > 7) {
        if (safetyManager) {
            safetyManager->triggerFault(SafetyFaultCode::SAFETY_INVALID_SECTION, "Invalid target section");
        }
        return;
    }

    Serial.printf("[DispensingTask] ROTATE_DISPENSER to section %u\n", targetSection);
    displayManager.showDispensingScreen(1);
    if (servoManager) {
        servoManager->rotateToSection(targetSection);
    }
}

void DispensingTask::enterOpenDoorStage() {
    stage = Stage::OPEN_DOOR;
    stageStartTime = millis();

    Serial.println("[DispensingTask] OPEN_DOOR");
    displayManager.showDispensingScreen(2);
    if (doorManager) {
        doorManager->open();
    }
}

void DispensingTask::enterDoseStage() {
    stage = Stage::DISPENSE_DOSE;
    stageStartTime = millis();

    Serial.printf("[DispensingTask] DISPENSE_DOSE section %u\n", targetSection);
    displayManager.showDispensingScreen(3);
    if (servoManager) {
        servoManager->stop();
    }
}

void DispensingTask::enterStartConveyorStage() {
    stage = Stage::START_CONVEYOR;
    stageStartTime = millis();

    Serial.println("[DispensingTask] START_CONVEYOR");
    displayManager.showDispensingScreen(3);
    if (motorManager) {
        motorManager->startConveyor(200);
    }
}

void DispensingTask::enterCheckMedicineStage() {
    stage = Stage::CHECK_MEDICINE_IR;
    stageStartTime = millis();

    Serial.println("[DispensingTask] CHECK_MEDICINE_IR");
    displayManager.showDispensingScreen(4);
    medicineVerification.start();
}

void DispensingTask::enterStopConveyorStage() {
    stage = Stage::STOP_CONVEYOR;
    stageStartTime = millis();

    Serial.println("[DispensingTask] STOP_CONVEYOR");
    if (motorManager) {
        motorManager->stopConveyor();
    }
}

void DispensingTask::enterCloseDoorStage() {
    stage = Stage::CLOSE_DOOR;
    stageStartTime = millis();

    Serial.println("[DispensingTask] CLOSE_DOOR");
    displayManager.showIdleScreen(12, 0, "CLOSING");
    if (doorManager) {
        doorManager->close();
    }
}

DispensingTask::Result DispensingTask::update(const char** errorMessageOut) {
    const uint32_t elapsed = millis() - stageStartTime;

    switch (stage) {
        case Stage::ROTATE_DISPENSER:
            if (elapsed >= DISPENSER_ROTATION_DELAY_MS) {
                enterOpenDoorStage();
                return Result::RUNNING;
            }
            if (elapsed > DISPENSER_ROTATION_DELAY_MS + 1000UL) {
                Serial.println("[DispensingTask] DISPENSER rotation timeout exceeded");
                if (safetyManager) {
                    safetyManager->triggerFault(SafetyFaultCode::SAFETY_DISPENSER_TIMEOUT, "Dispenser rotation timeout");
                }
                *errorMessageOut = "DISPENSER ROTATION TIMED OUT";
                return Result::ERROR;
            }
            return Result::RUNNING;

        case Stage::OPEN_DOOR:
            if (doorManager && !doorManager->isOpen() && elapsed > DOOR_OPEN_DELAY_MS + 1000UL) {
                Serial.println("[DispensingTask] DOOR_OPEN timeout exceeded");
                if (safetyManager) {
                    safetyManager->triggerFault(SafetyFaultCode::SAFETY_DOOR_TIMEOUT, "Door open timeout");
                }
                *errorMessageOut = "DOOR OPEN TIMED OUT";
                return Result::ERROR;
            }
            if (elapsed >= DOOR_OPEN_DELAY_MS) {
                if (doorManager && !doorManager->isOpen()) {
                    doorManager->open();
                }
                enterDoseStage();
                return Result::RUNNING;
            }
            return Result::RUNNING;

        case Stage::DISPENSE_DOSE:
            if (elapsed >= DOSE_SERVO_DELAY_MS) {
                if (servoManager) {
                    servoManager->stop();
                }
                enterStartConveyorStage();
                return Result::RUNNING;
            }
            if (elapsed > DOSE_SERVO_DELAY_MS + 1000UL) {
                Serial.println("[DispensingTask] DOSE servo timeout exceeded");
                if (safetyManager) {
                    safetyManager->triggerFault(SafetyFaultCode::SAFETY_MOTOR_TIMEOUT, "Dose dispense timeout");
                }
                *errorMessageOut = "DOSE SERVO TIMED OUT";
                return Result::ERROR;
            }
            return Result::RUNNING;

        case Stage::START_CONVEYOR:
            // Conveyor starts immediately; proceed to medicine verification
            if (elapsed >= 100UL) {
                enterCheckMedicineStage();
                return Result::RUNNING;
            }
            return Result::RUNNING;

        case Stage::CHECK_MEDICINE_IR: {
            // Check if conveyor has exceeded max runtime
            if (motorManager && motorManager->isRunning()) {
                if (motorManager->getRunTimeMs() > DC_MOTOR_MAX_RUNTIME_MS) {
                    Serial.printf("[DispensingTask] Conveyor timeout: %lu ms exceeds %lu ms limit\n",
                                  motorManager->getRunTimeMs(), (unsigned long)DC_MOTOR_MAX_RUNTIME_MS);
                    if (safetyManager) {
                        safetyManager->triggerFault(SafetyFaultCode::SAFETY_MOTOR_TIMEOUT, "Conveyor runtime exceeded");
                    }
                    *errorMessageOut = "CONVEYOR TIMEOUT";
                    return Result::ERROR;
                }
            }

            MedicineVerificationTask::Result verifyResult = medicineVerification.update();

            if (verifyResult == MedicineVerificationTask::Result::CONFIRMED) {
                Serial.println("[DispensingTask] MEDICINE_DETECTED");
                enterStopConveyorStage();
                return Result::RUNNING;
            }

            if (verifyResult == MedicineVerificationTask::Result::TIMEOUT) {
                if (safetyManager) {
                    safetyManager->triggerFault(SafetyFaultCode::SAFETY_INVALID_SENSOR, "Medicine verification timeout");
                }
                *errorMessageOut = "MEDICINE NOT DETECTED";
                return Result::ERROR;
            }

            return Result::RUNNING;
        }

        case Stage::STOP_CONVEYOR:
            // Motor is stopped; proceed to close door
            if (elapsed >= 100UL) {
                enterCloseDoorStage();
                return Result::RUNNING;
            }
            return Result::RUNNING;

        case Stage::CLOSE_DOOR:
            if (elapsed >= DOOR_CLOSE_DELAY_MS) {
                Serial.println("[DispensingTask] CLOSE_DOOR duration complete");
                return Result::COMPLETE;
            }
            return Result::RUNNING;
    }

    return Result::RUNNING;
}
