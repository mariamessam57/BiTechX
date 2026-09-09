#include "DispenserController.h"
#include <stdio.h>
#include <ArduinoJson.h>

// ============================================================
// MQTT message callback
// ============================================================

static DispenserController* g_dispenserController = nullptr;

static void mqttMessageCallback(const char* topic, const char* payload)
{
    if (g_dispenserController == nullptr) {
        return;
    }

    g_dispenserController->handleMqttCommand(topic, payload);
}


// ============================================================
// Constructor
// ============================================================

DispenserController::DispenserController()
    : currentState(DispenserState::IDLE),
      previousState(DispenserState::IDLE),
      stateStartTime(0),
      lastTimeCheck(0),
      activeSection(0),
      activeScheduledHour(0),
      activeScheduledMinute(0),
      audioManager(nullptr),
      doorManager(nullptr),
      servoManager(nullptr),
      motorManager(nullptr),
      medicineTask(nullptr),
      personDetectionTask(nullptr),
      dispensingTask(nullptr),
      wifiManager(nullptr),
      mqttManager(nullptr),
      telemetryManager(nullptr)
{
}


// ============================================================
// Begin
// ============================================================

void DispenserController::begin() {

    Serial.println("[DispenserController] Initializing system components...");

    // IoT initialization
    wifiManager = new WiFiManager();
    wifiManager->begin();

    mqttManager = new MQTTManager();
    mqttManager->begin();

    // Connect MQTT callback to controller
    g_dispenserController = this;
    mqttManager->setMessageCallback(mqttMessageCallback);

    telemetryManager = new TelemetryManager(*mqttManager);
    telemetryManager->begin();

    displayManager.begin();

    if (!timeManager.begin()) {
        Serial.println(
            "[DispenserController] RTC initialization failed; scheduler blocked"
        );
    }

    sensorManager.begin();

    audioManager = new AudioManager(
        AUDIO_SERIAL_RX_PIN,
        AUDIO_SERIAL_TX_PIN,
        AUDIO_BUSY_PIN
    );

    if (audioManager->begin(AUDIO_VOLUME)) {
        Serial.println("[DispenserController] Audio initialized");
    }
    else {
        Serial.println("[DispenserController] Audio init failed");
    }

    doorManager = new DoorManager(DOOR_SERVO_PIN);
    doorManager->begin();

    servoManager = new ServoManager(DISPENSER_SERVO_PIN);
    servoManager->begin();

    motorManager = new MotorManager(
        DC_MOTOR_IN1_PIN,
        DC_MOTOR_IN2_PIN
    );

    motorManager->begin();

    Serial.printf(
        "[DispenserController] Conveyor motor initialized (IN1=%u, IN2=%u)\n",
        DC_MOTOR_IN1_PIN,
        DC_MOTOR_IN2_PIN
    );

    safetyManager.begin();

    safetyManager.bind(
        servoManager,
        doorManager,
        motorManager
    );

    medicineTask = new MedicineTask(timeManager);

    personDetectionTask = new PersonDetectionTask(sensorManager);
    personDetectionTask->setSafetyManager(&safetyManager);

    dispensingTask = new DispensingTask(
        servoManager,
        doorManager,
        motorManager,
        displayManager,
        sensorManager
    );

    dispensingTask->setSafetyManager(&safetyManager);

    medicineTask->addSchedule(9, 0, 0);
    medicineTask->addSchedule(13, 0, 1);
    medicineTask->addSchedule(18, 0, 2);
    medicineTask->addSchedule(21, 0, 3);

    displayManager.showIdleScreen(
        12,
        0,
        "READY"
    );

    currentState = DispenserState::IDLE;
    previousState = DispenserState::IDLE;
    stateStartTime = millis();
    lastTimeCheck = millis();
}


// ============================================================
// Main Loop
// ============================================================

void DispenserController::loop() {

    uint32_t now = millis();

    // IoT update
    if (wifiManager) {
        wifiManager->update();
    }

    if (mqttManager) {
        mqttManager->update();
    }

    if (safetyManager.isFaultActive()) {
        runErrorState();
        return;
    }

    if (now - lastTimeCheck >= MEDICINE_CHECK_INTERVAL_MS) {

        if (!timeManager.updateTime()) {

            Serial.println(
                "[DispenserController] RTC read failed during periodic update"
            );
        }

        lastTimeCheck = now;
    }

    bool rotationComplete =
        (dispensingTask != nullptr)
            ? dispensingTask->isRotationComplete()
            : true;

    bool doorOpen =
        (doorManager != nullptr)
            ? doorManager->isOpen()
            : false;

    bool invalidSensorRead = false;

    bool rtcValid = timeManager.isRtcValid();

    uint8_t requestedSection = activeSection;

    bool motorRunning =
        (motorManager != nullptr)
            ? motorManager->isRunning()
            : false;

    uint32_t motorRunMs =
        (motorManager != nullptr)
            ? motorManager->getRunTimeMs()
            : 0;

    uint32_t dispenserElapsedMs =
        (dispensingTask != nullptr)
            ? dispensingTask->getStageElapsedMs()
            : 0;

    uint32_t doorElapsedMs = 0;

    if (!safetyManager.check(
            rtcValid,
            requestedSection,
            rotationComplete,
            doorOpen,
            motorRunning,
            motorRunMs,
            dispenserElapsedMs,
            doorElapsedMs,
            invalidSensorRead)) {

        triggerError(
            safetyManager.getFaultReason()
        );

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


// ============================================================
// State Management
// ============================================================

void DispenserController::changeState(
    DispenserState nextState
) {

    if (currentState == nextState) {
        return;
    }

    Serial.printf(
        "[DispenserController] %d -> %d\n",
        (int)currentState,
        (int)nextState
    );

    previousState = currentState;
    currentState = nextState;
    stateStartTime = millis();

    // Send current state to Dashboard
    if (telemetryManager) {

        const char* status = "unknown";

        switch (currentState) {

            case DispenserState::IDLE:
                status = "idle";
                break;

            case DispenserState::CHECK_TIME:
                status = "check_time";
                break;

            case DispenserState::ALERT:
                status = "alert";
                break;

            case DispenserState::WAIT_FOR_PERSON:
                status = "waiting_for_person";
                break;

            case DispenserState::DISPENSING:
                status = "dispensing";
                break;

            case DispenserState::ERROR:
                status = "error";
                break;
        }

        telemetryManager->sendStatus(status);
    }
}


// ============================================================
// IDLE
// ============================================================

void DispenserController::runIdleState() {

    displayManager.showIdleScreen(
        12,
        0,
        "READY"
    );

    changeState(
        DispenserState::CHECK_TIME
    );
}


// ============================================================
// CHECK TIME
// ============================================================

void DispenserController::runCheckTimeState() {

    uint8_t triggeredSection = 0;

    if (medicineTask->checkSchedule(triggeredSection)) {

        activeSection = triggeredSection;

        uint8_t currentHour = 0;
        uint8_t currentMinute = 0;
        uint8_t currentSecond = 0;
        uint8_t currentDay = 0;
        uint8_t currentMonth = 0;
        uint16_t currentYear = 0;

        if (timeManager.getCurrentTime(
                currentHour,
                currentMinute,
                currentSecond,
                currentDay,
                currentMonth,
                currentYear)) {

            activeScheduledHour = currentHour;
            activeScheduledMinute = currentMinute;
        }

        changeState(
            DispenserState::ALERT
        );

        return;
    }

    changeState(
        DispenserState::IDLE
    );
}


// ============================================================
// ALERT
// ============================================================

void DispenserController::runAlertState() {

    if (previousState != DispenserState::ALERT) {

        Serial.println(
            "[DispenserController] ALERT"
        );

        displayManager.showIdleScreen(
            12,
            0,
            "MEDICINE TIME"
        );

        if (audioManager && audioManager->isReady()) {
            audioManager->triggerMedicineAlarm();
        }
    }

    changeState(
        DispenserState::WAIT_FOR_PERSON
    );
}


// ============================================================
// WAIT FOR PERSON
// ============================================================

void DispenserController::runWaitForPersonState() {

    if (previousState != DispenserState::WAIT_FOR_PERSON) {

        Serial.println(
            "[DispenserController] WAIT_FOR_PERSON"
        );

        displayManager.showIdleScreen(
            12,
            0,
            "WAIT PERSON"
        );

        personDetectionTask->start();
    }

    PersonDetectionTask::Result result =
        personDetectionTask->update();

    if (result == PersonDetectionTask::Result::CONFIRMED) {

        Serial.printf(
            "[DispenserController] PERSON_PRESENT confirmed at threshold %.1f cm\n",
            PERSON_DETECTION_DISTANCE_CM
        );

        dispensingTask->start(activeSection);

        changeState(
            DispenserState::DISPENSING
        );

        return;
    }

    if (result == PersonDetectionTask::Result::TIMEOUT) {

        Serial.printf(
            "[DispenserController] PERSON_DETECTION_TIMEOUT after %lu ms; returning to IDLE\n",
            static_cast<unsigned long>(
                PERSON_DETECTION_TIMEOUT_MS
            )
        );

        changeState(
            DispenserState::IDLE
        );

        return;
    }
}


// ============================================================
// DISPENSING
// ============================================================

void DispenserController::runDispensingState() {

    const char* errorMessage = nullptr;

    DispensingTask::Result result =
        dispensingTask->update(&errorMessage);

    if (result == DispensingTask::Result::COMPLETE) {

        Serial.println(
            "[DispenserController] Dispensing sequence complete -> IDLE"
        );

        sendDoseTelemetry(true);

        resetToIdle();

        return;
    }

    if (result == DispensingTask::Result::ERROR) {

    sendDoseTelemetry(false);

    triggerError(
        errorMessage
            ? errorMessage
            : "DISPENSING ERROR"
    );

    return;
}
}


// ============================================================
// ERROR
// ============================================================

void DispenserController::runErrorState() {

    if (audioManager && audioManager->isReady()) {

        // Keep the safe error state visible.
        // Do not auto-transition back to IDLE.
    }

    safetyManager.emergencyStop();

    if (doorManager) {
        doorManager->close();
    }

    if (servoManager) {
        servoManager->stop();
    }
}


// ============================================================
// Reset to IDLE
// ============================================================

void DispenserController::resetToIdle() {

    Serial.println(
        "[DispenserController] Reset to IDLE"
    );

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

    displayManager.showIdleScreen(
        12,
        0,
        "READY"
    );

    changeState(
        DispenserState::IDLE
    );
}


// ============================================================
// Error Trigger
// ============================================================

void DispenserController::triggerError(
    const char* message
) {

    Serial.printf(
        "[DispenserController] ERROR: %s\n",
        message
    );

    displayManager.showErrorScreen(
        message
    );

    safetyManager.triggerFault(
        SafetyFaultCode::SAFETY_INVALID_STATE,
        message
            ? message
            : "System error"
    );

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

        audioManager->playTrack(
            SoundTrack::EMERGENCY_ALARM
        );
    }

    changeState(
        DispenserState::ERROR
    );
}


// ============================================================
// Medicine Schedule
// ============================================================

void DispenserController::addMedicineSchedule(
    uint8_t hour,
    uint8_t minute,
    uint8_t section
) {

    if (medicineTask) {

        medicineTask->addSchedule(
            hour,
            minute,
            section
        );
    }
}

void DispenserController::clearSchedules() {

    if (medicineTask) {

        medicineTask->clearSchedules();
    }
}
void DispenserController::handleMqttCommand(
    const char* topic,
    const char* payload
) {
    Serial.println("[DispenserController] MQTT command received");

    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Payload: ");
    Serial.println(payload);

    // Make sure the message came from the commands topic
    if (strcmp(topic, MQTT_TOPIC_COMMANDS) != 0) {
        Serial.println("[DispenserController] Unknown MQTT topic");
        return;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("[DispenserController] Invalid JSON: ");
        Serial.println(error.c_str());
        return;
    }

    const char* command = doc["command"];

    if (command == nullptr) {
        Serial.println("[DispenserController] Missing command field");
        return;
    }

    // ========================================================
    // SET SCHEDULES
    // ========================================================

    if (strcmp(command, "set_schedules") == 0) {

        JsonArray schedules = doc["schedules"].as<JsonArray>();

        if (schedules.isNull()) {
            Serial.println("[DispenserController] Missing schedules array");
            return;
        }

        clearSchedules();

        uint8_t addedSchedules = 0;

        for (JsonObject schedule : schedules) {

            if (!schedule["hour"].is<uint8_t>() ||
                !schedule["minute"].is<uint8_t>() ||
                !schedule["section"].is<uint8_t>()) {

                Serial.println(
                    "[DispenserController] Invalid schedule entry"
                );

                continue;
            }

            uint8_t hour = schedule["hour"];
            uint8_t minute = schedule["minute"];
            uint8_t section = schedule["section"];

            if (hour > 23 || minute > 59) {
                Serial.println(
                    "[DispenserController] Invalid time"
                );

                continue;
            }

            if (section >= 4) {
                Serial.println(
                    "[DispenserController] Invalid section"
                );

                continue;
            }

            addMedicineSchedule(
                hour,
                minute,
                section
            );

            addedSchedules++;

            Serial.printf(
                "[DispenserController] Schedule added: %02u:%02u -> section %u\n",
                hour,
                minute,
                section
            );
        }

        Serial.printf(
            "[DispenserController] Schedules updated: %u\n",
            addedSchedules
        );

        return;
    }

    // ========================================================
    // CLEAR SCHEDULES
    // ========================================================

    if (strcmp(command, "clear_schedules") == 0) {

        clearSchedules();

        Serial.println(
            "[DispenserController] All schedules cleared"
        );

        return;
    }

    // ========================================================
    // GET STATUS
    // ========================================================

    if (strcmp(command, "get_status") == 0) {

        Serial.println(
            "[DispenserController] Status request received"
        );

        // Status response will be implemented
        // after the command system is completed.

        return;
    }

    // ========================================================
    // RESET
    // ========================================================

    if (strcmp(command, "reset") == 0) {

        Serial.println(
            "[DispenserController] Reset command received"
        );

        resetToIdle();

        return;
    }

    // ========================================================
    // UNKNOWN COMMAND
    // ========================================================

    Serial.print(
        "[DispenserController] Unknown command: "
    );

    Serial.println(command);
}
void DispenserController::sendDoseTelemetry(bool taken)
{
    if (!telemetryManager) {
        return;
    }

    uint8_t currentHour = 0;
    uint8_t currentMinute = 0;
    uint8_t currentSecond = 0;
    uint8_t currentDay = 0;
    uint8_t currentMonth = 0;
    uint16_t currentYear = 0;

    if (!timeManager.getCurrentTime(
            currentHour,
            currentMinute,
            currentSecond,
            currentDay,
            currentMonth,
            currentYear)) {

        Serial.println(
            "[DispenserController] Cannot send telemetry: RTC read failed"
        );

        return;
    }

    char scheduledTime[6];
    snprintf(
        scheduledTime,
        sizeof(scheduledTime),
        "%02u:%02u",
        activeScheduledHour,
        activeScheduledMinute
    );

    char timestamp[20];
    snprintf(
        timestamp,
        sizeof(timestamp),
        "%04u-%02u-%02u %02u:%02u:%02u",
        currentYear,
        currentMonth,
        currentDay,
        currentHour,
        currentMinute,
        currentSecond
    );

    if (taken) {

        telemetryManager->sendDoseTaken(
            activeSection,
            scheduledTime,
            timestamp
        );
    }
    else {

        telemetryManager->sendDoseMissed(
            activeSection,
            scheduledTime,
            timestamp
        );
    }
}