/*
  This code initializes an OLED display (SSD1306) using the Adafruit SSD1306 library, 
  and displays various text, numbers, and scroll animations on the screen.

  Board: Arduino Uno R4 (or R3)
  Component:  OLED (SSD1306)
  Library: https://github.com/adafruit/Adafruit_SSD1306 (Adafruit SSD1306 by Adafruit)  
           https://github.com/adafruit/Adafruit-GFX-Library (Adafruit GFX Library by Adafruit) 
*/
// Libraries in use: "Adafruit SSD1306", "Adafruit BusIO", "Adafruit GFX"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET      (-1)  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*--------------------------------------------------
 * Definition for lap timer
 *--------------------------------------------------*/
#define DEBUG_PRINT     0     // Serial output for debug when 1
#define USE_INTERRUPT   1     // Use interrupt by FC-51 IR sensor module
#define IR_SENSOR_PIN   2     // Pin numbers for FC-51 IR sensor module
#define BLINK_COUNT     3     // Number of blinking
#define BLINK_INTERVAL  250   // Blinking interval [msec]
#define LAP_HYSTERESIS  4000  // Interval until start [msec]
#define TIMER_CURSOR_X  16    // Coordinate X on OLED
#define TIMER_CURSOR_Y  10    // Coordinate X on OLED

/*--------------------------------------------------
 * Definition for notification sound
 *--------------------------------------------------*/
#define MELODY_PIN      9     // Pin number for BGM
#define BUZZER_PIN      8     // Pin number for lap timer
#define BUZZER_DURATION 16    // Note durations [msec]
#define BUZZER_ON       255   // level for analogWrite()
#define BUZZER_OFF      0     // level for analogWrite()
#define BUZZER_FREQUENCY 2794 // NOTE_F7 [Hz]

/*--------------------------------------------------
 * Global variables to handle in interrup handler
 *--------------------------------------------------*/
// for lap timer
volatile static bool start;
volatile static char strLap[] = "00:00:00";
volatile static u_int32_t timerT0, timerT1;

// for lap blinking
volatile static int blinkCount;
volatile static uint32_t blinkT0;
volatile static u_int32_t buzzer_on;

/*--------------------------------------------------
 * Definition for background music
 *--------------------------------------------------*/
#include "BGMusic.h"
static BGMusic music;

// pitch and duration pairs in the melody:
#include "pitches.h"
#include "note_duration.h"
static int melody[] = {
#include "truth.h"
};

#define TEMPO         155   // change this to make the song slower or faster
#define REPEAT        true  // playback repeatedly
#define N_NOTES(s)    (sizeof(s) / sizeof((s)[0]) / 2)  // number of notes in musical score.
#define QUARTER_NOTE  ((60000 / N4) / TEMPO)  // quarter note duration in milliseconds based on the number of beats in 60 second

// Calculate note length from note length symbol
int calc_duration(int duration) {
  return QUARTER_NOTE * duration;
}

/*--------------------------------------------------
 * Setup background music
 *--------------------------------------------------*/
void setupBackgroundMusic(void) {
  music.set_duration_function(calc_duration);
  music.begin(MELODY_PIN, melody, N_NOTES(melody), TEMPO, REPEAT);
}

/*--------------------------------------------------
 * Setup OLED
 *--------------------------------------------------*/
void setupDisplay() {
  // initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.begin(9600);
    while (true) {
      Serial.println(F("SSD1306 allocation failed"));
      delay(1000);
    }
  }

  // Clear the buffer.
  display.clearDisplay();

  // Display Text
  display.setTextSize(2);       // Set text size
  display.setTextColor(WHITE);  // Set text color
}

/*--------------------------------------------------
 * Setup IR sensor
 *--------------------------------------------------*/
void setupSensor() {
  pinMode(IR_SENSOR_PIN, INPUT);

#if USE_INTERRUPT
  // Interrupt handler when the state of IR sensor changes
  // Note: The behavior of FALLING seems to be the same as CHANGE
  void checkStateIR(void);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), checkStateIR, CHANGE);
#endif
}

/*--------------------------------------------------
 * Initialize timer
 *--------------------------------------------------*/
void resetTimer(void) {
  // for lap timer
  start = false;
  timerT0 = timerT1 = 0;
  strcpy((char*)strLap, "00:00:00");

  // for lap blinking
  blinkT0 = 0;
  blinkCount = 0;
}

/*--------------------------------------------------
 * Convert time to string (00:00:00 - 59:59:99)
 *--------------------------------------------------*/
char* time2str(uint32_t msec, char *str) {
  uint32_t m, s, x;

  // 1/1000[sec] --> 1/100[sec]
  x = (msec /=  10UL) % 100UL;
  s = (msec /= 100UL) %  60UL;
  m = (msec /=  60UL) %  60UL;
  sprintf(str, "%02d:%02d:%02d", m, s, x);

  return str;
}

/*--------------------------------------------------
 * State transition for start and lap
 *--------------------------------------------------*/
void checkStateIR(void) {
  bool status = digitalRead(IR_SENSOR_PIN);

#if DEBUG_PRINT
  Serial.println(status);
#endif

  if (!status) {
    if (start) {
      uint32_t delta = timerT1 - timerT0;
      if (delta >= LAP_HYSTERESIS) {
        time2str(delta, (char*)strLap);
        timerT0 = timerT1;

        blinkT0 = timerT0;
        blinkCount = 1;

        buzzer_on = millis();
        analogWrite(BUZZER_PIN, BUZZER_ON);
        //tone(BUZZER_PIN, BUZZER_FREQUENCY, BUZZER_DURATION);
      }
    }

    // This is just for starting.
    else {
      start = true;
      timerT0 = timerT1;

      buzzer_on = millis();
      analogWrite(BUZZER_PIN, BUZZER_ON);
      //tone(BUZZER_PIN, BUZZER_FREQUENCY, BUZZER_DURATION);
      music.start();
    }
  }
}

/*--------------------------------------------------
 * Setup the OLED, IR sensor and initialize timer
 *--------------------------------------------------*/
void setup() {
  // OLED
  setupDisplay();

  // FC-51 infrared avoidance sensor
  setupSensor();

  // Initialize global variables
  resetTimer();

  // initialize the BGM player
  setupBackgroundMusic();

#if DEBUG_PRINT
  Serial.begin(9600);
#endif
}

/*--------------------------------------------------
 * Main loop
 *--------------------------------------------------*/
void loop() {
  // for timer
  char strNow[10];
  uint32_t delta;

  timerT1 = millis();

  // This is just for starting.
  if (!start) {
    timerT0 = timerT1;
  }

  delta = timerT1 - timerT0;

#if ! USE_INTERRUPT
  checkStateIR();
#endif

  display.clearDisplay();
  display.setCursor(TIMER_CURSOR_X, TIMER_CURSOR_Y);
  display.println(time2str(delta, strNow));

  // This is safe without conflict with interrupt because of hysteresis.
  if (blinkCount) {
    if ((timerT1 - blinkT0) >= BLINK_INTERVAL) {
      blinkT0 = timerT1;
      if (blinkCount++ > (BLINK_COUNT * 2 - 1)) {
        blinkCount = 0;
      }
    }
  }

  if (blinkCount % 2 == 0) {
    display.setCursor(TIMER_CURSOR_X, TIMER_CURSOR_Y + 32);
    display.println((char*)strLap);
  }

  display.display();

  if (buzzer_on && buzzer_on + millis() > BUZZER_DURATION) {
    buzzer_on = 0;
    analogWrite(BUZZER_PIN, BUZZER_OFF);
  }

#if DEBUG_PRINT
  Serial.print(strNow);
  Serial.print("\t");
  Serial.print((char*)strLap);
  Serial.println();
#endif
}