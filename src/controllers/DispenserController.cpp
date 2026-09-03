#include "DispenserController.h"
#include <stdio.h>

DispenserController::DispenserController()
    : currentState(DispenserState::IDLE),
      previousState(DispenserState::IDLE),
      stateStartTime(0),
      lastTimeCheck(0),
      activeSection(0),
      audioManager(nullptr),
      doorManager(nullptr),
      servoManager(nullptr),
      motorManager(nullptr),
      medicineTask(nullptr),
      personDetectionTask(nullptr),
      dispensingTask(nullptr) {
}

void DispenserController::begin() {
    Serial.println("[DispenserController] Initializing system components...");

    displayManager.begin();
    if (!timeManager.begin()) {
        Serial.println("[DispenserController] RTC initialization failed; scheduler blocked");
    }
    sensorManager.begin();

    audioManager = new AudioManager(AUDIO_SERIAL_RX_PIN, AUDIO_SERIAL_TX_PIN, AUDIO_BUSY_PIN);
    if (audioManager->begin(AUDIO_VOLUME)) {
        Serial.println("[DispenserController] Audio initialized");
    } else {
        Serial.println("[DispenserController] Audio init failed");
    }

    doorManager = new DoorManager(DOOR_SERVO_PIN);
    doorManager->begin();

    servoManager = new ServoManager(DISPENSER_SERVO_PIN);
    servoManager->begin();

    motorManager = new MotorManager(DC_MOTOR_IN1_PIN, DC_MOTOR_IN2_PIN);
    motorManager->begin();
    Serial.printf("[DispenserController] Conveyor motor initialized (IN1=%u, IN2=%u)\n", 
                  DC_MOTOR_IN1_PIN, DC_MOTOR_IN2_PIN);

    safetyManager.begin();
    safetyManager.bind(servoManager, doorManager, motorManager);

    medicineTask = new MedicineTask(timeManager);
    personDetectionTask = new PersonDetectionTask(sensorManager);
    personDetectionTask->setSafetyManager(&safetyManager);
    dispensingTask = new DispensingTask(servoManager, doorManager, motorManager, displayManager, sensorManager);
    dispensingTask->setSafetyManager(&safetyManager);

    medicineTask->addSchedule(9, 0, 0);
    medicineTask->addSchedule(13, 0, 1);
    medicineTask->addSchedule(18, 0, 2);
    medicineTask->addSchedule(21, 0, 3);

    displayManager.showIdleScreen(12, 0, "READY");
    currentState = DispenserState::IDLE;
    previousState = DispenserState::IDLE;
    stateStartTime = millis();
    lastTimeCheck = millis();
}

void DispenserController::loop() {
    uint32_t now = millis();

    if (safetyManager.isFaultActive()) {
        runErrorState();
        return;
    }

    if (now - lastTimeCheck >= MEDICINE_CHECK_INTERVAL_MS) {
        if (!timeManager.updateTime()) {
            Serial.println("[DispenserController] RTC read failed during periodic update");
        }
        lastTimeCheck = now;
    }

    bool rotationComplete = (dispensingTask != nullptr) ? dispensingTask->isRotationComplete() : true;
    bool doorOpen = (doorManager != nullptr) ? doorManager->isOpen() : false;
    bool invalidSensorRead = false;
    bool rtcValid = timeManager.isRtcValid();
    uint8_t requestedSection = activeSection;
    bool motorRunning = (motorManager != nullptr) ? motorManager->isRunning() : false;
    uint32_t motorRunMs = (motorManager != nullptr) ? motorManager->getRunTimeMs() : 0;
    uint32_t dispenserElapsedMs = (dispensingTask != nullptr) ? dispensingTask->getStageElapsedMs() : 0;
    uint32_t doorElapsedMs = 0;

    if (!safetyManager.check(rtcValid, requestedSection, rotationComplete, doorOpen,
                            motorRunning, motorRunMs, dispenserElapsedMs,
                            doorElapsedMs, invalidSensorRead)) {
        triggerError(safetyManager.getFaultReason());
        return;
    }

    switch (currentState) {
        case DispenserState::IDLE:
            runIdleState();
            break;
        case DispenserState::CHECK_TIME:
            runCheckTimeState();
            break;
        case DispenserState::ALERT:
            runAlertState();
            break;
        case DispenserState::WAIT_FOR_PERSON:
            runWaitForPersonState();
            break;
        case DispenserState::DISPENSING:
            runDispensingState();
            break;
        case DispenserState::ERROR:
            runErrorState();
            break;
    }
}

void DispenserController::changeState(DispenserState nextState) {
    if (currentState == nextState) {
        return;
    }

    Serial.printf("[DispenserController] %d -> %d\n", (int)currentState, (int)nextState);
    previousState = currentState;
    currentState = nextState;
    stateStartTime = millis();
}

void DispenserController::runIdleState() {
    displayManager.showIdleScreen(12, 0, "READY");
    changeState(DispenserState::CHECK_TIME);
}

void DispenserController::runCheckTimeState() {
    uint8_t triggeredSection = 0;

    if (medicineTask->checkSchedule(triggeredSection)) {
        activeSection = triggeredSection;
        changeState(DispenserState::ALERT);
        return;
    }

    changeState(DispenserState::IDLE);
}

void DispenserController::runAlertState() {
    if (previousState != DispenserState::ALERT) {
        Serial.println("[DispenserController] ALERT");
        displayManager.showIdleScreen(12, 0, "MEDICINE TIME");
        if (audioManager && audioManager->isReady()) {
            audioManager->triggerMedicineAlarm();
        }
    }

    changeState(DispenserState::WAIT_FOR_PERSON);
}

void DispenserController::runWaitForPersonState() {
    if (previousState != DispenserState::WAIT_FOR_PERSON) {
        Serial.println("[DispenserController] WAIT_FOR_PERSON");
        displayManager.showIdleScreen(12, 0, "WAIT PERSON");
        personDetectionTask->start();
    }

    PersonDetectionTask::Result result = personDetectionTask->update();

    if (result == PersonDetectionTask::Result::CONFIRMED) {
        Serial.printf("[DispenserController] PERSON_PRESENT confirmed at threshold %.1f cm\n",
                      PERSON_DETECTION_DISTANCE_CM);
        dispensingTask->start(activeSection);
        changeState(DispenserState::DISPENSING);
        return;
    }

    if (result == PersonDetectionTask::Result::TIMEOUT) {
        Serial.printf("[DispenserController] PERSON_DETECTION_TIMEOUT after %lu ms; returning to IDLE\n",
                      static_cast<unsigned long>(PERSON_DETECTION_TIMEOUT_MS));
        changeState(DispenserState::IDLE);
        return;
    }
}

void DispenserController::runDispensingState() {
    const char* errorMessage = nullptr;
    DispensingTask::Result result = dispensingTask->update(&errorMessage);

    if (result == DispensingTask::Result::COMPLETE) {
        Serial.println("[DispenserController] Dispensing sequence complete -> IDLE");
        resetToIdle();
        return;
    }

    if (result == DispensingTask::Result::ERROR) {
        triggerError(errorMessage ? errorMessage : "DISPENSING ERROR");
        return;
    }
}

void DispenserController::runErrorState() {
    if (audioManager && audioManager->isReady()) {
        // keep the safe error state visible; do not auto-transition back to idle
    }

    safetyManager.emergencyStop();
    if (doorManager) {
        doorManager->close();
    }
    if (servoManager) {
        servoManager->stop();
    }
}

void DispenserController::resetToIdle() {
    Serial.println("[DispenserController] Reset to IDLE");
    safetyManager.clearFault();
    if (motorManager) {
        motorManager->stopConveyor();
    }
    if (doorManager) {
        doorManager->close();
    }
    if (servoManager) {
        servoManager->stop();
    }
    displayManager.showIdleScreen(12, 0, "READY");
    changeState(DispenserState::IDLE);
}

void DispenserController::triggerError(const char* message) {
    Serial.printf("[DispenserController] ERROR: %s\n", message);
    displayManager.showErrorScreen(message);

    safetyManager.triggerFault(SafetyFaultCode::SAFETY_INVALID_STATE, message ? message : "System error");

    if (motorManager) {
        motorManager->stopConveyor();
    }
    if (doorManager) {
        doorManager->close();
    }
    if (servoManager) {
        servoManager->stop();
    }

    if (audioManager && audioManager->isReady()) {
        audioManager->playTrack(SoundTrack::EMERGENCY_ALARM);
    }

    changeState(DispenserState::ERROR);
}

void DispenserController::addMedicineSchedule(uint8_t hour, uint8_t minute, uint8_t section) {
    if (medicineTask) {
        medicineTask->addSchedule(hour, minute, section);
    }
}

void DispenserController::clearSchedules() {
    if (medicineTask) {
        medicineTask->clearSchedules();
    }
}
