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
#include "../iot/WiFiManager.h"
#include "../iot/MQTTManager.h"
#include "../iot/TelemetryManager.h"

// Top-level states owned by the controller.
enum class DispenserState {
    IDLE,
    CHECK_TIME,
    ALERT,
    WAIT_FOR_PERSON,
    DISPENSING,
    ERROR
};

// DispenserController is the high-level coordinator.
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
    uint8_t activeScheduledHour;
    uint8_t activeScheduledMinute;

    void changeState(DispenserState nextState);
    void runIdleState();
    void runCheckTimeState();
    void runAlertState();
    void runWaitForPersonState();
    void runDispensingState();
    void runErrorState();
    void resetToIdle();
    void triggerError(const char* message);
    void sendDoseTelemetry(bool taken);

    

    // IoT managers
    WiFiManager* wifiManager;
    MQTTManager* mqttManager;
    TelemetryManager* telemetryManager;

public:
    DispenserController();

    void begin();
    void loop();

    // MQTT command handler
    void handleMqttCommand(const char* topic, const char* payload);

    DisplayManager& getDisplayManager() {
        return displayManager;
    }

    void addMedicineSchedule(
        uint8_t hour,
        uint8_t minute,
        uint8_t section
    );

    void clearSchedules();
};