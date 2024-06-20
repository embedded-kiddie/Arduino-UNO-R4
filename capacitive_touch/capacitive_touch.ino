#include "R4_Touch.h"

/*----------------------------------------------------------------------
 * Output debug information to Serial Monitor
 *----------------------------------------------------------------------*/
#if 0
#define DEBUG_EXEC(x)   {(x);}
#else
#define DEBUG_EXEC(x)
#endif

/*----------------------------------------------------------------------
 * Peripheral Register Monitor
 *----------------------------------------------------------------------*/
#define MONITOR 0
#if MONITOR
#include "RTC.h"
#include "PeripheralMonitor.h"

static PeripheralMonitor monitor;
static bool flag;

/*----------------------------------------------------------------------
 * This callback function enables to kick Peripheral Register Monitor
 *----------------------------------------------------------------------*/
void periodicCallback() {
  flag = true;
}

/*----------------------------------------------------------------------
 * Initialize Peripheral Register Monitor
 *----------------------------------------------------------------------*/
void setupMonitor(void) {
  /*
   * PERIPHERAL_PORTS, // PORT0 〜 PORT9 (default)
   * PERIPHERAL_PORT,  // PORT0 〜 PORT9
   * PERIPHERAL_PFS,   // PmnPFS (P000 〜 P915)
   * PERIPHERAL_PINS,  // D0 〜 D19 (A0 〜 A5)
   * PERIPHERAL_AGT,   // AGT0 〜 AGT1
   * PERIPHERAL_CTSU,  // CTSU
   */
  monitor.begin(230400, PERIPHERAL_CTSU); // Baud rate: Mac 230400, Windows 921600

  // Initialize the RTC
  RTC.begin();

  // RTC.setTime() must be called for RTC.setPeriodicCallback to work, but it doesn't matter
  // what date and time it's set to
  RTCTime mytime(1, Month::JUNE, 2024, 0, 0, 0, DayOfWeek::THURSDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(mytime);

  // Recommend: N4_TIMES_EVERY_SEC, N8_TIMES_EVERY_SEC or N16_TIMES_EVERY_SEC
  RTC.setPeriodicCallback(periodicCallback, Period::N8_TIMES_EVERY_SEC);
}
#endif // MONITOR

/*----------------------------------------------------------------------
 * The definitions for Calibration of Capacitive Touch Sensor
 *----------------------------------------------------------------------*/
#define MAX_SNUM        8     // Number of measurements (CTSUSO0.CTSUSNUM)
#define TARGET_RATIO    0.375 // Target ratio of Offset Tuning
#define TARGET_LIMIT    40960 // Theoretical upper limit of measurement range (100%)
#define TARGET_COUNT    15360 // Target value of Offset Tuning (TARGET_LIMIT * TARGET_RATIO)
#define SAMPLE_COUNT    10    // Number of samples for Sensor Counter (CTSUSC) and Reference Counter (CTSURC)

#ifndef ABS
#define ABS(x)          ((x) >= 0 ? (x) : -(x))
#endif

/*----------------------------------------------------------------------
 * Sensor Counter (CTSUSC) and Reference Counter (CTSURC)
 *----------------------------------------------------------------------*/
static volatile int readCount = 0, readTotal = 0, reference = 0;
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

int getSampleCount(void) {
  return readTotal / readCount;
}

int getSampleReference(void) {
  return reference / readCount;
}

/*----------------------------------------------------------------------
 * Self-calibration (Offset Tuning)
 *----------------------------------------------------------------------*/
ctsu_pin_settings_t offsetTuning(uint8_t pin) {
  int count;
  int sc, rc; // CTSUSC, CTSURC

  ctsu_pin_settings_t config = {
    .div          = CTSU_CLOCK_DIV_2,
    .gain         = CTSU_ICO_GAIN_100,
    .ref_current  = 0,
    .offset       = 0,
    .count        = 1
  };

  // Attach sampling callback
  attachMeasurementEndCallback(sampleCallback);

  // Find the optimal frequency and number of measurements
  count = 0xFFFF;
  for (uint8_t i = MAX_SNUM; i >= 1; i /= 2) {
    setTouchPinMeasurementCount(pin, i);

    // Check the sensor drive pulse frequency in ascending order
    for (uint8_t j = CTSU_CLOCK_DIV_6; j <= CTSU_CLOCK_DIV_64; j++) {
      setTouchPinClockDiv(pin, (ctsu_clock_div_t)j);

      // Set ICO reference to get the upper limit of range
      setTouchPinReferenceCurrent(pin, 255);

      // Start sampling
      resetSampleCount(pin);
      TouchSensor::start();
      while (readCount < SAMPLE_COUNT);
      TouchSensor::stop();

      // Read sampled counter
      sc = getSampleCount();
      rc = getSampleReference();

      DEBUG_EXEC(Serial.print("SNUM = " + String(i) + ", SDPA = " + String(j) + ", RC = " + String(rc) + ", SC = " + String(sc)));

      // Check overflow
      if (rc == 0xFFFF || sc == 0xFFFF) {
        DEBUG_EXEC(Serial.println(" --> overflow"));
        break;
      }

      // Check out of the upper limit of range
      if (rc < sc) {
        DEBUG_EXEC(Serial.println(" --> SC: out of range"));
        continue;
      }

      // Check if the reference counter is enough for target
      if (rc < TARGET_LIMIT) {
        DEBUG_EXEC(Serial.println(" --> RC: narrow range"));
        continue;
      }

      // Find the smallest sensor counter
      if (count > sc) {
        count = sc;
        config.count = i;
        config.div = static_cast<ctsu_clock_div_t>(j);
        DEBUG_EXEC(Serial.println(" --> target candidate"));
      } else {
        DEBUG_EXEC(Serial.println(" --> target overshoot"));
        break;
      }
    }
  }

  DEBUG_EXEC(Serial.println("Number of Measurements (CTSUSO0.CTSUSNUM): " + String(config.count)));
  DEBUG_EXEC(Serial.println("Sensor Drive Pulse (CTSUSO1.CTSUSDPA): " + String(config.div)));

  // Find the optimal sensor offset adjustment
  count = 0xFFFF;
  for (uint16_t i = 0; i < 1024; i++) {
    setTouchPinSensorOffset(pin, i);

    resetSampleCount(pin);
    TouchSensor::start();
    while (readCount < SAMPLE_COUNT);
    TouchSensor::stop();

    sc = getSampleCount();
    rc = ABS(TARGET_COUNT - sc);

    DEBUG_EXEC(Serial.println("offset: " + String(i) + ", sensor count: " + String(sc) + ", diff: " + String(rc)));

    // Find the sensor offset adjustment closest to the target
    if (count > rc) {
      count = rc;
      config.offset = i;
    }
    else {
      break;
    }
  }

  DEBUG_EXEC(Serial.println("Sensor offset (CTSUSO0.CTSUSO): " + String(config.offset)));

  attachMeasurementEndCallback(nullptr);

  return config;
}

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

  // just fetch sensor counter to the buffer
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
 * Verify the characteristics of the ICO reference counter
 *----------------------------------------------------------------------*/
void verifyReferenceCount(uint8_t pin) {
  attachMeasurementEndCallback(sampleCallback);

  for (uint8_t i = 0; i < 256; i++) {
      // ICO 特性（ダイナミックレンジ）
      setTouchPinReferenceCurrent(pin, i);

      resetSampleCount(pin);
      TouchSensor::start();
      while (readCount < SAMPLE_COUNT);
      TouchSensor::stop();

      Serial.println(String(i) + "," + String(getSampleReference()));
  }

  attachMeasurementEndCallback(nullptr);
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
  digitalWrite(LED_BUILTIN, HIGH);

#if MONITOR
  setupMonitor();
#endif

  Serial.begin(115200);

#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif

  // LED の初期化
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode( 6, OUTPUT);
  pinMode( 5, OUTPUT);
  pinMode( 4, OUTPUT);

  // タッチセンサ部の初期化
  setTouchMode( 9);
  setTouchMode( 8);
  setTouchMode(15);
  setTouchMode(16);
  setTouchMode( 3);
  setTouchMode( 2);

  //verifyReferenceCount(9);

#if 1
  DEBUG_EXEC(Serial.println("Start calibration..."));

  applyTouchPinSettings( 9, offsetTuning( 9));
  applyTouchPinSettings( 8, offsetTuning( 8));
  applyTouchPinSettings(15, offsetTuning(15));
  applyTouchPinSettings(16, offsetTuning(16));
  applyTouchPinSettings( 3, offsetTuning( 3));
  applyTouchPinSettings( 2, offsetTuning( 2));

  DEBUG_EXEC(Serial.println("Finished."));
#endif

  DEBUG_EXEC(showPinSettings( 9));
  DEBUG_EXEC(showPinSettings( 8));
  DEBUG_EXEC(showPinSettings(15));
  DEBUG_EXEC(showPinSettings(16));
  DEBUG_EXEC(showPinSettings( 3));
  DEBUG_EXEC(showPinSettings( 2));

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
  int threshold = TARGET_COUNT + 1000;

#if 0
  // Just print values ​​for each touchpad
  Serial.print  (      String(readSensor( 9)));
  Serial.print  ("," + String(readSensor( 8)));
  Serial.print  ("," + String(readSensor(15)));
  Serial.print  ("," + String(readSensor(16)));
  Serial.print  ("," + String(readSensor( 3)));
  Serial.print  ("," + String(readSensor( 2)));
  Serial.println("," + String(threshold));
#else
  // Turn the LED on/off depending on the value of each touchpad
  digitalWrite(12, readSensor( 9) > threshold ? HIGH : LOW);
  digitalWrite(11, readSensor( 8) > threshold ? HIGH : LOW);
  digitalWrite(10, readSensor(15) > threshold ? HIGH : LOW);
  digitalWrite( 6, readSensor(16) > threshold ? HIGH : LOW);
  digitalWrite( 5, readSensor( 3) > threshold ? HIGH : LOW);
  digitalWrite( 4, readSensor( 2) > threshold ? HIGH : LOW);
#endif

#if MONITOR
  if (flag) {
    flag = false;
    monitor.scan_command();
    monitor.show_register();
  }
#endif

  delay(100);
}