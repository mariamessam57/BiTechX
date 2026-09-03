#include <Arduino.h>
#include "controllers/DispenserController.h"

DispenserController dispenserController;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n===== BiTechX Medicine Dispenser Initializing =====");
    Serial.println("Board: ESP32");
    Serial.println("Display: ST7789 TFT 172x320 (1.47\")");

    // Initialize all system tasks
    dispenserController.begin();

    Serial.println("Initialization complete!");
    Serial.println("==================================================\n");
}

void loop() {
    // Main event loop
    dispenserController.loop();

    // Small delay to prevent watchdog timeout
    delay(100);
}