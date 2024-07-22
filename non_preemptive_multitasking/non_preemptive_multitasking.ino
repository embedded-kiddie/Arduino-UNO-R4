/*
 * Test program for non-preemptive multi task.
 *
 * This program checks the effect on the periodicity of each task 
 * depending on the location where the base time is measured by millis().
 */
#define INTERVAL1 110 // [msec]
#define INTERVAL2 120 // [msec]
#define INTERVAL3 130 // [msec]

void ExecTask(uint32_t ms) {
  if (ms == 1) {
    Serial.println(millis()); // Output start time of each task
  }

  delay(ms); // Pseudo processing time
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
}

#if 0

uint32_t previousMillis1 = 0;
uint32_t previousMillis2 = 0;
uint32_t previousMillis3 = 0;

void loop() {
  uint32_t currentMillis = millis();

  // Task 1
  if (currentMillis - previousMillis1 >= INTERVAL1) {
    previousMillis1 = currentMillis;
    ExecTask(1);
  }

  // Task 2
  if (currentMillis - previousMillis2 >= INTERVAL2) {
    previousMillis2 = currentMillis;
    ExecTask(2);
  }

  // Task 3
  if (currentMillis - previousMillis3 >= INTERVAL3) {
    previousMillis3 = currentMillis;
    ExecTask(3);
  }
}

#elif 0

uint32_t previousMillis1 = 0;
uint32_t previousMillis2 = 0;
uint32_t previousMillis3 = 0;

void loop() {
  uint32_t currentMillis;

  // Task 1
  if ((currentMillis = millis()) - previousMillis1 >= INTERVAL1) {
    previousMillis1 = currentMillis;
    ExecTask(1);
  }

  // Task 2
  if ((currentMillis = millis()) - previousMillis2 >= INTERVAL2) {
    previousMillis2 = currentMillis;
    ExecTask(2);
  }

  // Task 3
  if ((currentMillis = millis()) - previousMillis3 >= INTERVAL3) {
    previousMillis3 = currentMillis;
    ExecTask(3);
  }
}

#else

void loop() {
#define DO_EVERY(period, prev)  static uint32_t prev = 0; for (uint32_t now = millis(); now - prev >= period; prev = now)

  // Task 1
  DO_EVERY(INTERVAL1, previousMillis1) {
    ExecTask(1);
  }

  // Task 2
  DO_EVERY(INTERVAL2, previousMillis2) {
    ExecTask(2);
  }

  // Task 3
  DO_EVERY(INTERVAL3, previousMillis3) {
    ExecTask(3);
  }
}

#endif