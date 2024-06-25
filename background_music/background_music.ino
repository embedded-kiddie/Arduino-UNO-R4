/*
  Example of music score player in background

  Copyright (c) 2024 embedded-kiddie All Rights Reserved.

  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
*/
#include "Arduino.h"
#include "BGMusic.h"

// notes (pairs of frequency and duration) in musical score.
// duration: N4 = quarter note, N8 = eighth note, etc.
#include "pitches.h"
#include "note_duration.h"
static int score[] = {
#include "truth.h"
};

#define BUZZER_PIN    9     // pin number connected to the buzzer
#define TEMPO         155   // change this to make the song slower or faster
#define REPEAT        true  // playback repeatedly
#define N_NOTES(s)    (sizeof(s) / sizeof((s)[0]) / 2)  // number of notes in musical score.
#define QUARTER_NOTE  ((60000 / N4) / TEMPO)  // ratio of the duration for one quarter note per beat

// Calculate note length from note length symbol
int calc_duration(int duration) {
  return QUARTER_NOTE * duration;
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BUZZER_PIN,  OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize and start the BGM player
  static BGMusic music;
  music.set_duration_function(calc_duration);
  music.begin(BUZZER_PIN, score, N_NOTES(score), TEMPO, REPEAT);
  music.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 256; i++) {
    analogWrite(LED_BUILTIN, i);
    delay(2);
  }

  for (int i = 255; i >= 0; i--) {
    analogWrite(LED_BUILTIN, i);
    delay(2);
  }

  delay(100);
}
