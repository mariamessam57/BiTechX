#include <Arduino.h>
#include "Tasks.h"

Tasks tasks;

void setup() {
    tasks.begin();
}

void loop() {
    tasks.loop();
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}