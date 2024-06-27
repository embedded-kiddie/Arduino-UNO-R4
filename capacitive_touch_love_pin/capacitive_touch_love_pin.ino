#include "R4_Touch.h"

/*----------------------------------------------------------------------
 * Output debug information to Serial Monitor
 *----------------------------------------------------------------------*/
#if 1
#define DEBUG_EXEC(x)   {x;}
#else
#define DEBUG_EXEC(x)
#endif

/*----------------------------------------------------------------------
 * The definitions for Calibration of Capacitive Touch Sensor
 *----------------------------------------------------------------------*/
#define MAX_SNUM        8     // Number of measurements (CTSUSO0.CTSUSNUM)
#define NUM_SAMPLES     10     // Number of samples for Sensor Counter (CTSUSC) and Reference Counter (CTSURC)
#define TARGET_RATIO    0.375 // Target ratio of Offset Tuning (37.5%)
#define TARGET_LIMIT    40960 // Theoretical upper limit of measurement range (100%)
#define TARGET_COUNT    15360 // Target value of Offset Tuning (TARGET_LIMIT * TARGET_RATIO)
#define TARGET_VALID    100   // A heuristic search criterion
#define TARGET_THRESH   (TARGET_COUNT + 1000) // Default touch threshold

#ifndef ABS
#define ABS(x)          ((x) >= 0 ? (x) : -(x))
#endif

/*----------------------------------------------------------------------
 * Moving Average
 *----------------------------------------------------------------------*/
#define USE_MOVING_AVE    1
#if     USE_MOVING_AVE == 0
#define readSensor  touchRead
#else
#define DEPTH_MOVING_AVE  5

static uint8_t  ma_head = 0;
static uint16_t ma_vals[NUM_ARDUINO_PINS][DEPTH_MOVING_AVE] = {{0,},};

void resetMovingAverate(void) {
  ma_head = 0;
  memset(ma_vals, 0, sizeof(ma_vals));
}

void fetchMovingAverage(void) {
  extern int num_configured_sensors;
  extern uint16_t results[][2];

  // just fetch sensor counter into the buffer of moving average
  for (uint8_t i = 0; i < num_configured_sensors; i++) {
    ma_vals[i][ma_head] = results[i][0];
  }

  // update head pointer
  ma_head = (ma_head + 1) % DEPTH_MOVING_AVE;
}

uint16_t readSensor(uint8_t pin) {
  extern int8_t pinToDataIndex[];
  uint8_t i = pinToDataIndex[pin];
  uint32_t sum = 0;

  for (uint8_t j = 0; j < DEPTH_MOVING_AVE; j++) {
    sum += ma_vals[i][j];
  }

  return sum / DEPTH_MOVING_AVE;
}
#endif // USE_MOVING_AVE

/*----------------------------------------------------------------------
 * Sensor Counter (CTSUSC) and Reference Counter (CTSURC) sampling
 *----------------------------------------------------------------------*/
static volatile uint32_t readCount = 0, readTotal = 0, reference = 0;
static volatile uint8_t target_pin = 0;

void resetSampleCount(uint8_t pin) {
  readCount = 0;
  readTotal = 0;
  reference = 0;
  target_pin = pin;
}

void sampleCallback(void) {
  readTotal += touchRead(target_pin);
  reference += touchReadReference(target_pin);
  readCount++;
}

inline uint32_t getNumOfSamples(void) {
  return readCount;
}

uint16_t getSensorCount(void) {
  return readTotal / readCount;
}

uint16_t getSampleReference(void) {
  return reference / readCount;
}

/*----------------------------------------------------------------------
 * Self-calibration (Offset Tuning)
 *----------------------------------------------------------------------*/
ctsu_pin_settings_t offsetTuning(uint8_t pin) {
  int sc, rc; // CTSUSC, CTSURC
  int min_sc, diff_sc;

  ctsu_pin_settings_t config = {
    .div          = CTSU_CLOCK_DIV_2,
    .gain         = CTSU_ICO_GAIN_100,
    .ref_current  = 0,
    .offset       = 0,
    .count        = MAX_SNUM
  };

  // Attach sampling callback
  attachMeasurementEndCallback(sampleCallback);

  /*------------------------------------------------------------
   * 1. Find the optimal frequency and number of measurements
   *------------------------------------------------------------*/
  applyTouchPinSettings(pin, config);

  DEBUG_EXEC(Serial.println("Pin: " + String(pin)));

  for (uint8_t i = MAX_SNUM; i >= 1; i-- /*i /= 2*/) {
    min_sc = 0xFFFF;
    setTouchPinMeasurementCount(pin, i);

    // Check the sensor drive pulse frequency in ascending order
    for (uint8_t j = CTSU_CLOCK_DIV_6; j <= CTSU_CLOCK_DIV_64; j++) {
      setTouchPinClockDiv(pin, (ctsu_clock_div_t)j);

      // Set ICO reference to get the upper limit of measurement range
      setTouchPinSensorOffset(pin, 0);
      setTouchPinReferenceCurrent(pin, 255);

      // Start sampling
      resetSampleCount(pin);
      TouchSensor::start();
      while (getNumOfSamples() < NUM_SAMPLES);
      TouchSensor::stop();

      // Read sampled counter
      sc = getSensorCount();
      rc = getSampleReference();

      DEBUG_EXEC(Serial.print("  SNUM = " + String(i) + ", SDPA = " + String(j) + ", RC = " + String(rc) + ", SC = " + String(sc)));

      // Check overflow
      if (rc == 0xFFFF || sc == 0xFFFF) {
        DEBUG_EXEC(Serial.println(" --> overflow"));
        break;
      }

      // Check if the reference counter is enough for target (TARGET_LIMIT)
      if (rc > TARGET_LIMIT) {
        DEBUG_EXEC(Serial.println(" --> RC: out of limit"));
        break;
      }

      // Check out of the upper limit of measurement range
      if (rc < sc) {
        DEBUG_EXEC(Serial.println(" --> SC: out of range"));
        continue;
      }

      // Find the smallest sensor counter
      diff_sc = ABS(sc - TARGET_COUNT);
      if (diff_sc > min_sc) {
        DEBUG_EXEC(Serial.println(" --> SC: inadequate"));
        continue;
      }

      min_sc = diff_sc;
      config.count = i;
      config.div = static_cast<ctsu_clock_div_t>(j);
      DEBUG_EXEC(Serial.println(" --> target candidate (diff SC = " + String(diff_sc) + ")"));
    }

    // Proceed to next SNUM in case of overflow or no candidate
    if (rc == 0xFFFF || sc == 0xFFFF || min_sc == 0xFFFF) {
      continue;
    }

    /*------------------------------------------------------------
    * 2. Find the optimal sensor offset adjustment
    *------------------------------------------------------------*/
    min_sc = 0xFFFF;
    config.offset = 0;
    applyTouchPinSettings(pin, config);

    for (uint16_t j = 0; j < 1024; j++) {
      setTouchPinSensorOffset(pin, j);

      resetSampleCount(pin);
      TouchSensor::start();
      while (getNumOfSamples() < NUM_SAMPLES);
      TouchSensor::stop();

      sc = getSensorCount();
      diff_sc = ABS(sc - TARGET_COUNT);

      DEBUG_EXEC(Serial.print("    offset: " + String(j) + ", SC = " + String(sc) + ", diff SC = " + String(diff_sc)));

      if (sc == 0xFFFF) {
        DEBUG_EXEC(Serial.println(" --> overflow"));
        break;
      }

      // Find the sensor offset adjustment closest to the target (TARGET_COUNT)
      if (diff_sc > min_sc) {
        DEBUG_EXEC(Serial.println(" --> overshoot"));
        break;
      }

      if (sc > TARGET_THRESH) {
        DEBUG_EXEC(Serial.println(" --> over threshold"));
        continue;
      }

      min_sc = diff_sc;
      config.offset = j;
      DEBUG_EXEC(Serial.println(" --> found candidate"));
    }

    // This is a heuristic search criterion.
    // There's a possibility of finding the minimum SC,
    // but further searches will reduce the sensitivity.
    // So stop the search here.
    if (min_sc < TARGET_VALID) {
      break;
    }
  }

  DEBUG_EXEC(Serial.println("Number of Measurements (SNUM, CTSUSO0.CTSUSNUM): " + String(config.count)));
  DEBUG_EXEC(Serial.println("Sensor Drive Pulse (SPDA, CTSUSO1.CTSUSDPA): " + String(config.div)));
  DEBUG_EXEC(Serial.println("Sensor offset (offset, CTSUSO0.CTSUSO): " + String(config.offset)));

  attachMeasurementEndCallback(nullptr);

  return config;
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
void showPinSettings(uint8_t pin) {
  ctsu_pin_settings_t set = getTouchPinSettings(pin);
  Serial.print  ("pin: "           + String(pin));
  Serial.print  (", div: "         + String(set.div));
  Serial.print  (", gain: "        + String(set.gain));
  Serial.print  (", ref_current: " + String(set.ref_current));
  Serial.print  (", offset: "      + String(set.offset));
  Serial.println(", count: "       + String(set.count));
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
void setup() {
  // Initilize LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  while (!Serial);

#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif

  // Love pin defined in R4_CTSU_Utils.cpp
  setTouchMode(20);

  DEBUG_EXEC(Serial.println("Start calibration..."));

  applyTouchPinSettings(20, offsetTuning(20));

  DEBUG_EXEC(Serial.println("Finished."));

  DEBUG_EXEC(
    showPinSettings(20);
  );

  digitalWrite(LED_BUILTIN, LOW);

#if USE_MOVING_AVE
  resetMovingAverate();
  attachMeasurementEndCallback(fetchMovingAverage);
#endif

  TouchSensor::start();
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
void loop() {
  int threshold = 16000;

  DEBUG_EXEC(
    // Just print the values ​to display on serial plotter
    Serial.print  (      String(readSensor(20)));
    Serial.println("," + String(threshold));
  );

  // Turn the LED on/off depending on the value of each touch sensor
  digitalWrite(LED_BUILTIN, readSensor(20) > threshold ? HIGH : LOW);

  delay(100);
}