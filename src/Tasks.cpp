#include "Tasks.h"
#include <stdio.h>

Tasks::Tasks()
    : currentState(DispenserState::IDLE),
      previousState(DispenserState::IDLE),
      stateStartTime(0),
      lastTimeCheck(0),
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
    timeManager.begin();
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
        timeManager.updateTime();
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

bool Tasks::isCurrentTriggeredMinute(uint8_t hour, uint8_t minute) {
    uint32_t scheduleKey = (uint32_t(hour) * 60UL) + uint32_t(minute);

    if (timeManager.isScheduledTime(hour, minute)) {
        if (scheduleKey == lastTriggeredScheduleKey) {
            return false;
        }
        return true;
    }

    if (lastTriggeredScheduleKey == scheduleKey) {
        lastTriggeredScheduleKey = 0xFFFFFFFFUL;
    }
    return false;
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
            activeSection = schedules[i].section;
            uint32_t triggerKey = (uint32_t(schedules[i].hour) * 60UL) + uint32_t(schedules[i].minute);
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

void Tasks::runWaitForPersonState() {
    if (previousState != DispenserState::WAIT_FOR_PERSON) {
        Serial.println("[Tasks] WAIT_FOR_PERSON");
        displayManager.showIdleScreen(12, 0, "WAIT PERSON");
    }

    float distance = sensorManager.readUltrasonicDistance();
    if (distance > 0.0f && distance < 50.0f) {
        Serial.printf("[Tasks] PERSON_PRESENT at %.1f cm\n", distance);
        changeState(DispenserState::OPEN_DOOR);
    }
}

void Tasks::runOpenDoorState() {
    if (previousState != DispenserState::OPEN_DOOR) {
        Serial.println("[Tasks] OPEN_DOOR");
        displayManager.showDispensingScreen(1);
        if (doorManager) {
            doorManager->open();
        }
    }

    if (millis() - stateStartTime >= DOOR_OPEN_DELAY_MS) {
        changeState(DispenserState::ROTATE_DISPENSER);
    }
}

void Tasks::runRotateDispenserState() {
    if (previousState != DispenserState::ROTATE_DISPENSER) {
        const uint8_t targetSection = activeSection + 1U;
        Serial.printf("[Tasks] ROTATE_DISPENSER to section %u\n", targetSection);
        displayManager.showDispensingScreen(2);
        if (servoManager) {
            servoManager->rotateToSection(targetSection);
        }
    }

    if (millis() - stateStartTime >= DISPENSER_ROTATION_DELAY_MS) {
        changeState(DispenserState::DISPENSE_DOSE_SERVO);
    }
}

void Tasks::runDispenseDoseServoState() {
    if (previousState != DispenserState::DISPENSE_DOSE_SERVO) {
        const uint8_t targetSection = activeSection + 1U;
        Serial.printf("[Tasks] DISPENSE_DOSE_SERVO section %u\n", targetSection);
        displayManager.showDispensingScreen(3);
        if (servoManager) {
            servoManager->rotateToSection(targetSection);
        }
    }

    if (millis() - stateStartTime >= DOSE_SERVO_DELAY_MS) {
        if (servoManager) {
            servoManager->stop();
        }
        changeState(DispenserState::CHECK_MEDICINE_IR);
    }
}

void Tasks::runCheckMedicineIRState() {
    if (previousState != DispenserState::CHECK_MEDICINE_IR) {
        Serial.println("[Tasks] CHECK_MEDICINE_IR");
        displayManager.showDispensingScreen(4);
    }

    if (sensorManager.isIrActive()) {
        Serial.println("[Tasks] MEDICINE_DETECTED");
        changeState(DispenserState::CLOSE_DOOR);
        return;
    }

    if (millis() - stateStartTime >= IR_DETECTION_TIMEOUT_MS) {
        Serial.println("[Tasks] IR timeout -> ERROR");
        triggerError("MEDICINE NOT DETECTED");
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
