#include "Tasks.h"
#include <stdio.h>

Tasks::Tasks()
    : currentState(DispenserState::IDLE),
      previousState(DispenserState::IDLE),
      stateStartTime(0),
      lastTimeCheck(0),
      personDetectionStartTime(0),
      lastPersonDetectionAttemptMs(0),
      personDetectionConfirmCount(0),
      medicineVerifyStartTime(0),
      lastMedicineVerifyAttemptMs(0),
      medicineDetectionConfirmCount(0),
      activeSection(0),
      lastTriggeredScheduleKey(0xFFFFFFFFUL),
      scheduleCount(0),
      audioManager(nullptr),
      doorManager(nullptr),
      servoManager(nullptr) {
}

void Tasks::begin() {
    Serial.println("[Tasks] Initializing system components...");

    displayManager.begin();
    if (!timeManager.begin()) {
        Serial.println("[Tasks] RTC initialization failed; scheduler blocked");
    }
    sensorManager.begin();

    audioManager = new AudioManager(AUDIO_SERIAL_RX_PIN, AUDIO_SERIAL_TX_PIN, AUDIO_BUSY_PIN);
    if (audioManager->begin(AUDIO_VOLUME)) {
        Serial.println("[Tasks] Audio initialized");
    } else {
        Serial.println("[Tasks] Audio init failed");
    }

    doorManager = new DoorManager(DOOR_SERVO_PIN);
    doorManager->begin();

    servoManager = new ServoManager(DISPENSER_SERVO_PIN);
    servoManager->begin();

    addMedicineSchedule(9, 0, 0);
    addMedicineSchedule(13, 0, 1);
    addMedicineSchedule(18, 0, 2);
    addMedicineSchedule(21, 0, 3);

    displayManager.showIdleScreen(12, 0, "READY");
    currentState = DispenserState::IDLE;
    previousState = DispenserState::IDLE;
    stateStartTime = millis();
    lastTimeCheck = millis();
}

void Tasks::loop() {
    uint32_t now = millis();

    if (now - lastTimeCheck >= MEDICINE_CHECK_INTERVAL_MS) {
        if (!timeManager.updateTime()) {
            Serial.println("[Tasks] RTC read failed during periodic update");
        }
        lastTimeCheck = now;
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
        case DispenserState::OPEN_DOOR:
            runOpenDoorState();
            break;
        case DispenserState::ROTATE_DISPENSER:
            runRotateDispenserState();
            break;
        case DispenserState::DISPENSE_DOSE_SERVO:
            runDispenseDoseServoState();
            break;
        case DispenserState::CHECK_MEDICINE_IR:
            runCheckMedicineIRState();
            break;
        case DispenserState::CLOSE_DOOR:
            runCloseDoorState();
            break;
        case DispenserState::ERROR:
            runErrorState();
            break;
    }
}

uint32_t Tasks::buildScheduleOccurrenceKey(uint8_t hour, uint8_t minute) {
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

bool Tasks::isCurrentTriggeredMinute(uint8_t hour, uint8_t minute) {
    if (!timeManager.isRtcValid()) {
        Serial.println("[Tasks] RTC invalid; schedule check skipped");
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

bool Tasks::isScheduleTriggered(const MedicineSchedule& schedule) {
    return isCurrentTriggeredMinute(schedule.hour, schedule.minute);
}

bool Tasks::isMedicineTimeNow() {
    for (uint8_t i = 0; i < scheduleCount; ++i) {
        if (isScheduleTriggered(schedules[i])) {
            return true;
        }
    }
    return false;
}

void Tasks::changeState(DispenserState nextState) {
    if (currentState == nextState) {
        return;
    }

    Serial.printf("[Tasks] %d -> %d\n", (int)currentState, (int)nextState);
    previousState = currentState;
    currentState = nextState;
    stateStartTime = millis();
}

void Tasks::runIdleState() {
    displayManager.showIdleScreen(12, 0, "READY");
    changeState(DispenserState::CHECK_TIME);
}

void Tasks::runCheckTimeState() {
    for (uint8_t i = 0; i < scheduleCount; ++i) {
        if (isScheduleTriggered(schedules[i])) {
            if (!timeManager.isRtcValid()) {
                Serial.println("[Tasks] RTC invalid; refusing to trigger medicine schedule");
                changeState(DispenserState::IDLE);
                return;
            }

            activeSection = schedules[i].section;
            uint32_t triggerKey = buildScheduleOccurrenceKey(schedules[i].hour, schedules[i].minute);
            lastTriggeredScheduleKey = triggerKey;
            Serial.printf("[Tasks] Medicine time match: %02d:%02d section=%u\n",
                          schedules[i].hour, schedules[i].minute, activeSection);
            changeState(DispenserState::ALERT);
            return;
        }
    }

    changeState(DispenserState::IDLE);
}

void Tasks::runAlertState() {
    if (previousState != DispenserState::ALERT) {
        Serial.println("[Tasks] ALERT");
        displayManager.showIdleScreen(12, 0, "MEDICINE TIME");
        if (audioManager && audioManager->isReady()) {
            audioManager->triggerMedicineAlarm();
        }
    }

    changeState(DispenserState::WAIT_FOR_PERSON);
}

void Tasks::resetPersonDetectionState() {
    personDetectionStartTime = millis();
    lastPersonDetectionAttemptMs = 0;
    personDetectionConfirmCount = 0;
}

void Tasks::resetMedicineVerifyState() {
    medicineVerifyStartTime = millis();
    lastMedicineVerifyAttemptMs = 0;
    medicineDetectionConfirmCount = 0;
}

void Tasks::runWaitForPersonState() {
    if (previousState != DispenserState::WAIT_FOR_PERSON) {
        Serial.println("[Tasks] WAIT_FOR_PERSON");
        displayManager.showIdleScreen(12, 0, "WAIT PERSON");
        resetPersonDetectionState();
    }

    const uint32_t now = millis();
    if ((now - personDetectionStartTime) >= PERSON_DETECTION_TIMEOUT_MS) {
        Serial.printf("[Tasks] PERSON_DETECTION_TIMEOUT after %lu ms; returning to IDLE\n",
                      static_cast<unsigned long>(PERSON_DETECTION_TIMEOUT_MS));
        resetPersonDetectionState();
        changeState(DispenserState::IDLE);
        return;
    }

    if ((now - lastPersonDetectionAttemptMs) < PERSON_DETECTION_RETRY_INTERVAL_MS) {
        return;
    }

    lastPersonDetectionAttemptMs = now;
    const float distance = sensorManager.readUltrasonicDistance();

    if (distance <= 0.0f || distance == -1.0f) {
        Serial.println("[Tasks] Invalid ultrasonic reading during person detection");
        personDetectionConfirmCount = 0;
        return;
    }

    if (distance <= PERSON_DETECTION_DISTANCE_CM) {
        personDetectionConfirmCount++;
        Serial.printf("[Tasks] Person detection confirmation %u/%u at %.2f cm\n",
                      personDetectionConfirmCount,
                      PERSON_DETECTION_REQUIRED_READINGS,
                      distance);

        if (personDetectionConfirmCount >= PERSON_DETECTION_REQUIRED_READINGS) {
            Serial.printf("[Tasks] PERSON_PRESENT confirmed at threshold %.1f cm\n",
                          PERSON_DETECTION_DISTANCE_CM);
            resetPersonDetectionState();
            changeState(DispenserState::OPEN_DOOR);
            return;
        }
    } else {
        Serial.printf("[Tasks] Distance %.2f cm is outside threshold %.1f cm\n",
                      distance,
                      PERSON_DETECTION_DISTANCE_CM);
        personDetectionConfirmCount = 0;
    }
}

void Tasks::runOpenDoorState() {
    const uint32_t elapsed = millis() - stateStartTime;

    if (previousState != DispenserState::OPEN_DOOR) {
        Serial.println("[Tasks] OPEN_DOOR");
        displayManager.showDispensingScreen(1);
        if (doorManager) {
            doorManager->open();
        }
    }

    if (elapsed >= DOOR_OPEN_DELAY_MS) {
        changeState(DispenserState::ROTATE_DISPENSER);
        return;
    }

    if (elapsed > DOOR_OPEN_DELAY_MS + 1000UL) {
        Serial.println("[Tasks] DOOR_OPEN timeout exceeded; entering ERROR");
        triggerError("DOOR OPEN TIMED OUT");
    }
}

void Tasks::runRotateDispenserState() {
    const uint32_t elapsed = millis() - stateStartTime;

    if (previousState != DispenserState::ROTATE_DISPENSER) {
        const uint8_t targetSection = activeSection + 1U;
        Serial.printf("[Tasks] ROTATE_DISPENSER to section %u\n", targetSection);
        displayManager.showDispensingScreen(2);
        if (servoManager) {
            servoManager->rotateToSection(targetSection);
        }
    }

    if (elapsed >= DISPENSER_ROTATION_DELAY_MS) {
        changeState(DispenserState::DISPENSE_DOSE_SERVO);
        return;
    }

    if (elapsed > DISPENSER_ROTATION_DELAY_MS + 1000UL) {
        Serial.println("[Tasks] DISPENSER rotation timeout exceeded; entering ERROR");
        triggerError("DISPENSER ROTATION TIMED OUT");
    }
}

void Tasks::runDispenseDoseServoState() {
    const uint32_t elapsed = millis() - stateStartTime;

    if (previousState != DispenserState::DISPENSE_DOSE_SERVO) {
        const uint8_t targetSection = activeSection + 1U;
        Serial.printf("[Tasks] DISPENSE_DOSE_SERVO section %u\n", targetSection);
        displayManager.showDispensingScreen(3);
        if (servoManager) {
            servoManager->stop();
        }
    }

    if (elapsed >= DOSE_SERVO_DELAY_MS) {
        if (servoManager) {
            servoManager->stop();
        }
        changeState(DispenserState::CHECK_MEDICINE_IR);
        return;
    }

    if (elapsed > DOSE_SERVO_DELAY_MS + 1000UL) {
        Serial.println("[Tasks] DOSE servo timeout exceeded; entering ERROR");
        triggerError("DOSE SERVO TIMED OUT");
    }
}

void Tasks::runCheckMedicineIRState() {
    // State-entry protection: (re)initialize the verification timer and
    // confirmation counters exactly once, on the first loop iteration after
    // entering this state. This also serves as the "wait for medicine
    // arrival" anchor point, so the wait period is not restarted on
    // subsequent loop iterations.
    if (previousState != DispenserState::CHECK_MEDICINE_IR) {
        Serial.println("[Tasks] CHECK_MEDICINE_IR");
        displayManager.showDispensingScreen(4);
        resetMedicineVerifyState();
        return;
    }

    const uint32_t now = millis();
    const uint32_t elapsedSinceEntry = now - medicineVerifyStartTime;

    // Give the medicine time to physically reach the IR sensor before the
    // first read is taken. A single early "inactive" reading here would be
    // meaningless, not a real failed detection.
    if (elapsedSinceEntry < MEDICINE_VERIFY_WAIT_MS) {
        return;
    }

    // Bound the whole verification window (arrival wait + confirmation
    // checks). If medicine is never confirmed within this window, fail safe
    // into ERROR rather than assuming success or retrying dispensing.
    if (elapsedSinceEntry >= (MEDICINE_VERIFY_WAIT_MS + MEDICINE_VERIFY_TIMEOUT_MS)) {
        Serial.println("[Tasks] MEDICINE_VERIFY_TIMEOUT -> ERROR");
        triggerError("MEDICINE NOT DETECTED");
        return;
    }

    // Poll the IR sensor periodically instead of every loop iteration.
    if ((now - lastMedicineVerifyAttemptMs) < MEDICINE_VERIFY_RETRY_INTERVAL_MS) {
        return;
    }
    lastMedicineVerifyAttemptMs = now;

    if (sensorManager.isMedicineDetected()) {
        medicineDetectionConfirmCount++;
        Serial.printf("[Tasks] Medicine detection confirmation %u/%u\n",
                      medicineDetectionConfirmCount,
                      MEDICINE_DETECTION_REQUIRED_READINGS);

        if (medicineDetectionConfirmCount >= MEDICINE_DETECTION_REQUIRED_READINGS) {
            Serial.println("[Tasks] MEDICINE_DETECTED (confirmed)");
            resetMedicineVerifyState();
            changeState(DispenserState::CLOSE_DOOR);
            return;
        }
    } else {
        // A single inactive reading breaks the consecutive-detection streak,
        // so a brief noisy pulse cannot count as confirmation.
        medicineDetectionConfirmCount = 0;
    }
}

void Tasks::runCloseDoorState() {
    if (previousState != DispenserState::CLOSE_DOOR) {
        Serial.println("[Tasks] CLOSE_DOOR");
        displayManager.showIdleScreen(12, 0, "CLOSING");
        if (doorManager) {
            doorManager->close();
        }
    }

    if (millis() - stateStartTime >= DOOR_CLOSE_DELAY_MS) {
        Serial.println("[Tasks] CLOSE_DOOR duration complete -> IDLE");
        resetToIdle();
    }
}

void Tasks::runErrorState() {
    if (audioManager && audioManager->isReady()) {
        // keep the safe error state visible; do not auto-transition back to idle
    }

    if (doorManager) {
        doorManager->close();
    }
    if (servoManager) {
        servoManager->stop();
    }
}

void Tasks::resetToIdle() {
    Serial.println("[Tasks] Reset to IDLE");
    if (doorManager) {
        doorManager->close();
    }
    if (servoManager) {
        servoManager->stop();
    }
    displayManager.showIdleScreen(12, 0, "READY");
    changeState(DispenserState::IDLE);
}

void Tasks::triggerError(const char* message) {
    Serial.printf("[Tasks] ERROR: %s\n", message);
    displayManager.showErrorScreen(message);

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

void Tasks::addMedicineSchedule(uint8_t hour, uint8_t minute, uint8_t section) {
    if (scheduleCount >= MAX_SCHEDULES) {
        return;
    }

    schedules[scheduleCount].hour = hour;
    schedules[scheduleCount].minute = minute;
    schedules[scheduleCount].section = section;
    scheduleCount++;
}

void Tasks::clearSchedules() {
    scheduleCount = 0;
    lastTriggeredScheduleKey = 0xFFFFFFFFUL;
}