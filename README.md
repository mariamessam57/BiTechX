# 💊 BiTechX - Smart Medicine Dispenser

An embedded smart medicine dispensing system developed using **ESP32 Dev Module**, **ESP-IDF**, **PlatformIO**, and **FreeRTOS**.

The system automatically reminds patients of their medication schedule, detects patient presence, dispenses the correct medicine, and provides visual and audio feedback through a modular software architecture.

---

# ✨ Features

* ⏰ Real-Time Medicine Scheduling (DS3231 RTC)
* 💊 Automatic Medicine Dispensing
* 🚪 Servo-Controlled Door
* 🔄 Multi-Slot Medicine Storage
* 📺 16x2 I2C LCD Display
* 🔊 DFPlayer Mini Voice Alerts
* 🔔 Buzzer Notifications
* 🌈 RGB LED Status Indication
* 📡 Ultrasonic Patient Detection
* 📦 IR Sensor for Medicine Verification
* 🔘 Manual Dispense Button
* ♻️ Reset Button
* 🚧 Limit Switch Protection
* 🧵 FreeRTOS Multitasking
* 🧩 Modular C++ Architecture

---

# 🛠 Hardware

* ESP32 Dev Module
* PCA9685 Servo Driver
* DS3231 RTC Module
* 16x2 LCD (I2C PCF8574)
* DFPlayer Mini
* Speaker
* Servo Motors
* HC-SR04 Ultrasonic Sensor
* IR Sensor
* RGB LED
* Buzzer
* Push Buttons
* Limit Switch
* Buck Converter
* Power Supply

---

# 📂 Project Structure

```text
BiTechX/
├── include/
│   └── config.h
│
├── lib/
│   ├── ServoManager/
│   ├── DisplayManager/
│   ├── AudioManager/
│   ├── SensorManager/
│   ├── TimeManager/
│   ├── DoorManager/
│   └── SystemController/
│
├── src/
│   ├── Tasks.h
│   ├── Tasks.cpp
│   └── main.cpp
│
├── platformio.ini
└── .gitignore
```

---

# 📚 Software Architecture

## ServoManager

Responsible for controlling all servo motors, including:

* Main rotating servo
* Door servo
* Medicine dispensing servos

---

## DisplayManager

Responsible for:

* LCD messages
* RGB LED status indication

---

## AudioManager

Responsible for:

* DFPlayer Mini
* Buzzer notifications

---

## SensorManager

Responsible for reading:

* Ultrasonic Sensor
* IR Sensor
* Limit Switch

---

## TimeManager

Responsible for:

* Reading time from DS3231
* Managing medicine schedules

---

## DoorManager

Responsible for:

* Opening and closing the dispensing door
* Monitoring door status

---

## SystemController

The main controller that coordinates communication between all managers and controls the complete dispensing workflow.

---

# 🔄 System Workflow

```text
System Start
      │
      ▼
Initialize Hardware
      │
      ▼
Read RTC Time
      │
      ▼
Medicine Time?
      │
 ┌────┴─────┐
 │          │
No         Yes
 │          │
 ▼          ▼
Wait   Activate Reminder
            │
            ▼
 Detect Patient Presence
            │
     ┌──────┴──────┐
     │             │
    No            Yes
     │             │
     ▼             ▼
 Timeout      Open Door
                   │
                   ▼
          Rotate Main Servo
                   │
                   ▼
        Dispense Medicine
                   │
                   ▼
          Verify with IR Sensor
                   │
         ┌─────────┴─────────┐
         │                   │
      Success             Failure
         │                   │
         ▼                   ▼
    Close Door        Error Handling
         │
         ▼
      Return to Idle
```

---

# 🧵 FreeRTOS Tasks

* RTC Task
* Scheduler Task
* Sensor Task
* Display Task
* Audio Task
* Dispensing Task
* Button Task
* Error Task
* System Task

---

# 🛠 Development Environment

* ESP32 Dev Module
* ESP-IDF Framework
* PlatformIO
* C++17
* FreeRTOS

---

# 🚀 Future Improvements

* Wi-Fi connectivity
* Mobile application
* OTA Firmware Update
* Cloud synchronization
* Medication history logging
* Battery monitoring
* Remote notifications

---

# 📄 License

This project was developed for educational and embedded systems learning purposes.
