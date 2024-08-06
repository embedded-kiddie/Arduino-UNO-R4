/*
 * Copyright (c) 2024 embedded-kiddie
 * Copyright (c) 2015 boochow
 * Released under the MIT license
 * https://opensource.org/license/mit
 */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define DEBUG 0
#if DEBUG
#define DEBUG_EXEC(x) {if (play.level >= 1) {x;}}
#else
#define DEBUG_EXEC(x)
#endif

/* SPI pin definition for Arduino UNO R3 and R4
  | ST7789 | PIN  |  R3  |   R4   |     Description      |
  |--------|------|------|--------|----------------------|
  | SCL    |  D13 | SCK  | RSPCKA | Serial clock         |
  | SDA    | ~D11 | COPI | COPIA  | Serial data input    |
  | RES    | ~D9  | PB1  | P303   | Reset signal         |
  | DC     |  D8  | PB0  | P304   | Display data/command |
*/
#define TFT_CS 10
#define TFT_RST 9  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC  8

#define DEVICE_WIDTH  240
#define DEVICE_HEIGHT 240
#define DEVICE_ORIGIN 2

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define PIN_RACKET  A5  // Potentiometer or Joystick
#define PIN_SOUND   7   // Buzzer

// Pseudo screen scaling
#define SCREEN_SCALE  2 // 2 (60 x 60) or 3 (30 x 30)
#define SCREEN_DEV(v) ((int)(v) << SCREEN_SCALE) // Screen to Device
#define DEV_SCREEN(v) ((int)(v) >> SCREEN_SCALE) // Device to Screen
#define SCREEN_WIDTH  DEV_SCREEN(DEVICE_WIDTH)
#define SCREEN_HEIGHT DEV_SCREEN(DEVICE_HEIGHT)

// Block (Screen coordinate system)
#define BLOCK_ROWS    5
#define BLOCK_COLS    10
#define BLOCK_WIDTH   (SCREEN_WIDTH / BLOCK_COLS)
#define BLOCK_HEIGHT  DEV_SCREEN( 8)
#define BLOCK_TOP     DEV_SCREEN(20)
#define BLOCK_END(t)  ((t) + BLOCK_ROWS * BLOCK_HEIGHT - 1)

// Ball
#define BALL_SIZE     7 // [px] (Device coordinate system)
#define BALL_MOVE_X   (4 - SCREEN_SCALE) // Screen coordinate system
#define BALL_MOVE_Y   (4 - SCREEN_SCALE) // Screen coordinate system
#define BALL_CYCLE    40 // [msec]
#define DEMO_CYCLE    16 // [msec]

// Racket (Screen coordinate system)
#define RACKET_WIDTH  DEV_SCREEN(44)
#define RACKET_HEIGHT DEV_SCREEN( 8)
#define RACKET_TOP    (SCREEN_HEIGHT - RACKET_HEIGHT)
#define RACKET_CYCLE  16

// Wall (Screen coordinate system)
#define WALL_TOP      0
#define WALL_LEFT     0
#define WALL_RIGHT    (SCREEN_WIDTH - 1)

// Font size for setTextSize(2)
#define FONT_WIDTH    12 // [px] (Device coordinate system)
#define FONT_HEIGHT   16 // [px] (Device coordinate system)

// Drawing level and score
#define DRAW_SCORE    2
#define DRAW_ALL      3

// Tone frequency
#include "pitches.h"
#define HIT_BLOCK   NOTE_C4
#define HIT_RACKET  NOTE_C3

// Colors by 16-bit (R5-G6-B5)
#define BLACK     ST77XX_BLACK
#define WHITE     ST77XX_WHITE
#define RED       ST77XX_RED
#define GREEN     ST77XX_GREEN
#define BLUE      ST77XX_BLUE
#define CYAN      ST77XX_CYAN
#define MAGENTA   ST77XX_MAGENTA
#define YELLOW    ST77XX_YELLOW
#define ORANGE    ST77XX_ORANGE

// Misc functions
#define SIGN(a)   ((a) > (0) ? (1) : (-1))
#define NARR(a, t) (sizeof(a) / sizeof(t))

#define ClearScreen() tft.fillScreen(BLACK)
#define ClearMessage() tft.fillRect(0, DEVICE_HEIGHT / 2, DEVICE_WIDTH - 1, FONT_HEIGHT * 2, BLACK)

#if (SCREEN_SCALE <= 2)
#define DrawBall(ball, tft, color) tft.fillCircle(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), (BALL_SIZE >> 1), (color))
#else
#define DrawBall(ball, tft, color) tft.fillRect(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), BALL_SIZE, BALL_SIZE, (color))
#endif
#define DrawRacket(x, tft, color) tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(RACKET_TOP), SCREEN_DEV(RACKET_WIDTH), SCREEN_DEV(RACKET_HEIGHT), (color))

// Type definitions
typedef enum {
  OPENING,
  START,
  PLAYING,
  CLEAR,
  GAMEOVER,
} Status_t;

typedef struct {
  bool      demo;
  Status_t  status;
  uint8_t   level;
  uint8_t   balls;
  uint8_t   block_top;
  uint8_t   block_end;
  uint8_t   ball_cycle;
  uint8_t   racket_width;
  uint8_t   combo;
  int8_t    spin;
  uint16_t  score;
  uint32_t  pause;
} Play_t;

typedef struct {
  int16_t   x, y;
  int16_t   dx, dy;
} Ball_t;

typedef struct {
  int16_t   x;
  int16_t   x_prev;
  uint8_t   count;
} Racket_t;

// Global variables
Play_t play;
Ball_t ball;
Racket_t racket;
bool blocks[BLOCK_ROWS][BLOCK_COLS];

void GameInit(bool demo);

void DrawMessage(uint32_t pause, uint16_t x, const char* msg) {
  tft.setTextSize(3);
  tft.setCursor(x, DEVICE_HEIGHT / 2);

  size_t len = strlen_P(msg);
  for (int i = 0 ; i < len ; i++) {
    tft.print((char)pgm_read_byte(msg++));
  }

  play.pause = millis() + pause;
}

void DrawScore(int refresh = 0) {
  tft.setTextSize(2);
  tft.setCursor(4, 0);
  tft.print("Lv:");

  // Level (3 digits)
  if (refresh == DRAW_ALL) {
    tft.fillRect(40, 0, FONT_WIDTH * 3, FONT_HEIGHT, BLACK);
  }
  if (refresh != DRAW_SCORE) {
    tft.setCursor(40, 0);
    tft.print(play.level);
  }

#if (DEBUG == 0)
  // Score (5 digits)
  if (refresh & DRAW_SCORE) {
    tft.fillRect(96, 0, FONT_WIDTH * 5, FONT_HEIGHT, BLACK);
  }
  char buf[6];
  sprintf(buf, "%05d", play.score);
  tft.setCursor(96, 0);
  tft.print(buf);

  // Balls (5 digits)
  if (refresh == DRAW_ALL) {
    tft.fillRect(175, 0, DEVICE_WIDTH - 175, FONT_HEIGHT, BLACK);
  }
  if (refresh != DRAW_SCORE) {
    for (int i = 0; i < play.balls; i++) {
      tft.fillCircle(230 - (i * BALL_SIZE * 3 / 2), BALL_SIZE >> 1, BALL_SIZE >> 1, YELLOW);
    }
  }
#endif
}

// Block related method
void BlocksInit() {
  memset((void*)blocks, (int)true, NARR(blocks, bool));
}

int8_t BlocksCount() {
  int8_t n = 0;
  bool *p = (bool*)blocks;

  for (int8_t i = 0; i < NARR(blocks, bool); i++) {
    n += (int8_t)*p++;
  }

  return n;
}

void BlocksDrawAll() {
  static const uint16_t colors[] PROGMEM = {CYAN, MAGENTA, YELLOW, RED, GREEN, ORANGE};

  int16_t x, y;
  int16_t c = 0;
  bool *p = (bool*)blocks;

  for(y = play.block_top; y <= play.block_end; y += BLOCK_HEIGHT, c = (c + 1) % NARR(colors, uint16_t)) {
    for(x = 0; x < SCREEN_WIDTH; x += BLOCK_WIDTH) {
      if (*p++) {
        tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), pgm_read_word(&colors[c]));
        tft.drawRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
      }
    }
  }
}

void BlocksEraseOne(int16_t row, int16_t col) {
  int16_t x = col * BLOCK_WIDTH;
  int16_t y = row * BLOCK_HEIGHT + play.block_top;

  DEBUG_EXEC(delay(500); Serial.println(String(x) + ", " + String(y)));

  tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
  tone(PIN_SOUND, HIT_BLOCK, 20);
  blocks[row][col] = false;
  play.score += ++play.combo;
  DrawScore(DRAW_SCORE);

  DEBUG_EXEC(delay(500));
}

bool BlockExist(int16_t x, int16_t y) {
  int16_t row = (y - play.block_top);
  int16_t col = (x - WALL_LEFT     );

  if (row >= 0 && col >= 0) {
    row /= BLOCK_HEIGHT;
    col /= BLOCK_WIDTH;

    if (row < BLOCK_ROWS && col < BLOCK_COLS && blocks[row][col]) {
      BlocksEraseOne(row, col);
      return true;
    }
  }

  return false;
}

void BlocksCheckHit(void) {
  if (BlockExist(ball.x + ball.dx, ball.y)) {
    ball.dx = -ball.dx;
  }

  if (BlockExist(ball.x, ball.y + ball.dy)) {
    ball.dy = -ball.dy;
  }

  if (BlockExist(ball.x + ball.dx, ball.y + ball.dy)) {
    ball.dx = -ball.dx;
    ball.dy = -ball.dy;
  }
}

// Ball related method
void BallInit(void) {
  DrawBall(ball, tft, BLACK);

  int16_t x = random(1, SCREEN_WIDTH - 1);
  ball = {
    .x  = (int16_t)x,
    .y  = (int16_t)(play.block_end + BLOCK_HEIGHT),
    .dx = (int16_t)(x > (SCREEN_WIDTH >> 1) ? -BALL_MOVE_X : BALL_MOVE_X),
    .dy = (int16_t)BALL_MOVE_Y
  };
}

bool BallLost(void) {
  return ball.y >= RACKET_TOP ? true : false;
}

void BallMove(void) {
  if (play.balls && play.pause == 0) {
    int16_t nx = abs(ball.dx);
    int16_t ny = abs(ball.dy);
    int16_t dx = SIGN(ball.dx);
    int16_t dy = SIGN(ball.dy);

    do {
      DrawBall(ball, tft, BLACK);

      if (nx > 0) {
        nx--;
        ball.x += dx;
        if (ball.x == SCREEN_WIDTH - 1 || ball.x == 0) {
          ball.dx = -ball.dx;
          dx = -dx;
        }
      }

      if (ny > 0) {
        ny--;
        ball.y += dy;
        if (ball.y == RACKET_TOP - 1) {
          if (racket.x - 1 <= ball.x && ball.x <= racket.x + RACKET_WIDTH) {
#if (SCREEN_SCALE <= 2)
            int8_t d = ball.x - (racket.x + (RACKET_WIDTH >> 1));
            if (abs(d) < (RACKET_WIDTH >> 2)) {
              ball.dx = SIGN(ball.dx) * (BALL_MOVE_X >> 1); // center
            } else {
              ball.dx = SIGN(ball.dx) * (BALL_MOVE_X); // edge
            }
#endif
            play.combo = 0;
            ball.dy = -ball.dy;
            dy = -dy;
            DEBUG_EXEC(DrawBall(ball, tft, YELLOW); delay(500));
            tone(PIN_SOUND, HIT_RACKET, 20);
          }
        }
      }

      if (ball.y == WALL_TOP) {
        ball.dy = -ball.dy;
        dy = -dy;
      }

      DrawBall(ball, tft, YELLOW);
      BlocksCheckHit();
    } while (nx > 0 || ny > 0);

    // Redraw game score when ball is inside the drawing area
    if (ball.y <= DEV_SCREEN(FONT_HEIGHT) + DEV_SCREEN(BALL_SIZE)) {
      DrawScore();
    }
  }
}

// Racket related method
void RacketInit() {
  racket = { racket.x, racket.x, 0 };
}

void RacketMove(void) {
  int16_t x, before = racket.x;

  x = map(analogRead(PIN_RACKET), 0, 1023, -5, SCREEN_WIDTH - RACKET_WIDTH + 5);
  x = constrain(x, WALL_LEFT, WALL_RIGHT - RACKET_WIDTH + 1);

  if (play.demo == false) {
    racket.x = x;
  } else {
    // Once user moves the racket sufficiently, demo mode will be disabled
    int16_t dx = x - racket.x_prev;
    if (abs(dx) > 1 && ++racket.count > 1) {
      racket.x = x;
      GameInit(false); // --> demo = false, status = OPENING
    } else {
      racket.x_prev = x;
      racket.x = ball.x - (RACKET_WIDTH >> 1);
      racket.x = min(max(racket.x, WALL_LEFT), WALL_RIGHT - RACKET_WIDTH + 1);
    }
  }

  if (before != racket.x) {
    DrawRacket(before, tft, BLACK);
  }

  DrawRacket(racket.x, tft, WHITE);  
}

// Play control method
void PlayInit(bool demo) {
  play = { demo, OPENING, 1, 5, BLOCK_TOP, BLOCK_END(BLOCK_TOP), (uint8_t)(demo ? DEMO_CYCLE : BALL_CYCLE), RACKET_WIDTH, 0, };
}

void PlayNext(void) {
  play.level++;
  play.ball_cycle -= 1;
  play.ball_cycle = max(play.ball_cycle, (play.demo ? DEMO_CYCLE : BALL_CYCLE >> 1));
  play.block_top += (BLOCK_HEIGHT >> 1);
  play.block_top = min(play.block_top, (BLOCK_TOP + BLOCK_HEIGHT * 5));
  play.block_end = BLOCK_END(play.block_top);
}

void PlayControl(void) {
  if (play.pause == 0) {
    switch (play.status) {
      case OPENING:
        ClearScreen();
        GameStart();
        play.status = START;
        break;
      case START:
        BallInit();
        play.status = PLAYING;
        if (play.demo == false) {
          DrawMessage(1000, 70, PSTR("Ready?"));
        }
        break;
      case PLAYING:
        if (BlocksCount() == 0) {
          play.status = CLEAR;
        } else if (BallLost()) {
          play.status = (--play.balls ? START : GAMEOVER);
          DrawScore(DRAW_ALL);
          DrawMessage(1000, 80, PSTR("Oops!"));
        }
        break;
      case CLEAR:
        PlayNext();
        play.status = OPENING;
        if (play.demo == false) {
          DrawMessage(1000, 80, PSTR("Nice!"));
        }
        break;
      case GAMEOVER:
        GameInit(true); // --> demo = true, status = OPENING
        DrawMessage(2000, 40, PSTR("Game Over"));
        break;
    }
  } else if (millis() >= play.pause) {
    ClearMessage();
    play.pause = 0;
  }
}

// Game initialize method
void GameInit(bool demo) {
  PlayInit(demo);
  RacketInit();
}

void GameStart(void) {
  BallInit();
  BlocksInit();
  BlocksDrawAll();
  DrawScore(DRAW_ALL);
}

void setup() {
#if DEBUG
  Serial.begin(115200);
  while (!Serial);
#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
#endif

  tft.init(DEVICE_WIDTH, DEVICE_HEIGHT, SPI_MODE2); // SPI_MODE2 or SPI_MODE3
  tft.setRotation(DEVICE_ORIGIN);
  tft.setTextColor(WHITE);

  GameInit(true);
}

// Non-preemptive multitasking
#define DO_EVERY(period, prev)  static uint32_t prev = 0; for (uint32_t now = millis(); now - prev >= period; prev = now)

void loop() {
  PlayControl();

  DO_EVERY(play.ball_cycle, TimeBall) {
    BallMove();
  }

  DO_EVERY(RACKET_CYCLE, TimeRacket) {
    RacketMove();
  }
}