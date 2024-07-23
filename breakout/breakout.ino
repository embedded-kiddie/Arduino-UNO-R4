#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define DEMO_MODE 1

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > (0) ? (a) : -(a))
#define SIGN(a)   ((a) > (0) ? (1) : (-1))

/* SPI pin definition for Arduino UNO R3 and R4
  | ST7798 | PIN  |  R3  |   R4   |     Description      |
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

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Pseudo screen scaling
#define SCREEN_SCALE  2 // 2 (60 x 60) or 3 (30 x 30)
#define SCREEN_WIDTH  (DEVICE_WIDTH  >> SCREEN_SCALE)
#define SCREEN_HEIGHT (DEVICE_HEIGHT >> SCREEN_SCALE)
#define SCREEN_DEV(v) ((int)(v) << SCREEN_SCALE)  // Screen to Device

// Ball
#define BALL_SIZE     8 // size on the device
#define BALL_MOVE_X   (4 - SCREEN_SCALE) // coodinate on the screen
#define BALL_MOVE_Y   (4 - SCREEN_SCALE) // coodinate on the screen

// Racket (coodinate on the screen)
#define RACKET_WIDTH  (40 >> SCREEN_SCALE)
#define RACKET_HEIGHT ( 8 >> SCREEN_SCALE)
#define RACKET_LINE   (SCREEN_HEIGHT - RACKET_HEIGHT)

// Block (coodinate on the screen)
#define BLOCK_ROWS    5
#define BLOCK_COLS    10
#define BLOCK_WIDTH   (SCREEN_WIDTH / BLOCK_COLS)
#define BLOCK_HEIGHT  (8 >> SCREEN_SCALE)
#define BLOCK_TOP     (6 -  SCREEN_SCALE)
#define BLOCK_BOTTOM  (BLOCK_TOP + BLOCK_ROWS * BLOCK_HEIGHT - 1)
#define N_BLOCKS      (BLOCK_ROWS * SCREEN_WIDTH / BLOCK_WIDTH)

// Colors by 16-bit (R5-G6-B5)
#define BLACK     ST77XX_BLACK
#define BLUE      ST77XX_BLUE
#define RED       ST77XX_RED
#define GREEN     ST77XX_GREEN
#define CYAN      ST77XX_CYAN
#define MAGENTA   ST77XX_MAGENTA
#define YELLOW    ST77XX_YELLOW
#define WHITE     ST77XX_WHITE

// Frequency of note for tone()
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165
#define NOTE_F3   175
#define NOTE_FS3  185
#define NOTE_G3   196
#define NOTE_GS3  208
#define NOTE_A3   220
#define NOTE_AS3  233
#define NOTE_B3   247
#define NOTE_C4   262

#define HIT_BLOCK   NOTE_C4
#define HIT_RACKET  NOTE_C3

#define PIN_RACKET  A5
#define PIN_SOUND   7

typedef struct {
  int16_t x, y;
  int16_t dx, dy;
} Ball;

typedef enum {
  OPENING,
  START,
  PLAYING,
  CLEAR,
  GAMEOVER,
} Status;

#define ClearScreen() tft.fillScreen(ST77XX_BLACK)

#if SCREEN_SCALE == 2
#define DrawBall(ball, tft, color)  tft.fillCircle(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), BALL_SIZE >> 1, color)
#else
#define DrawBall(ball, tft, color)  tft.fillRect(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), BALL_SIZE, BALL_SIZE, color)
#endif
#define DrawRacket(racket, tft, color)  tft.fillRect(SCREEN_DEV(racket), SCREEN_DEV(RACKET_LINE), SCREEN_DEV(RACKET_WIDTH), SCREEN_DEV(RACKET_HEIGHT), color)

// Global variables
Ball ball;
bool blocks[N_BLOCKS];
int16_t racket;

// Ball related methods
void BallInit(Ball &ball) {
  int16_t x = random(0, SCREEN_WIDTH); // 30, 19, 56, 36, 3
  ball = {
    .x  = (int16_t)x,
    .y  = (int16_t)(BLOCK_BOTTOM + BLOCK_HEIGHT),
    .dx = (int16_t)(x > (SCREEN_WIDTH >> 1) ? -BALL_MOVE_X : BALL_MOVE_X),
    .dy = (int16_t)BALL_MOVE_Y
  };
}

bool BallLost(void) {
  if (ball.y >= RACKET_LINE) {
    return true;
  } else {
    return false;
  }
}

void BlocksInit() {
  for(int8_t i = 0; i < N_BLOCKS; i++) {
    blocks[i] = true;
  }
}

uint8_t BlocksCount() {
  int8_t n = 0;
  
  for (int8_t i = 0; i < N_BLOCKS; i++) {
    if (blocks[i])
      n++;
  }

  return n;
}

void BlocksDrawAll() {
  static const uint16_t colors[BLOCK_ROWS] PROGMEM = {CYAN, MAGENTA, YELLOW, RED, GREEN};

  int16_t x, y;
  int8_t i = 0;
  int8_t c = 0;

  for(y = BLOCK_TOP; y <= BLOCK_BOTTOM; y += BLOCK_HEIGHT) {
    for(x = 0; x < SCREEN_WIDTH; x += BLOCK_WIDTH) {
      if (blocks[i++]) {
        tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), pgm_read_word(&colors[c]));
        tft.drawRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
      }
    }
    c++;
  }
}

void BlocksEraseOne(int8_t block) {
  int16_t x = (block % BLOCK_COLS) * BLOCK_WIDTH;
  int16_t y = (block / BLOCK_COLS) * BLOCK_HEIGHT + BLOCK_TOP;
  tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
}

int8_t BlocksFind(int16_t x, int16_t y) {
  if ((x < 0) || (SCREEN_WIDTH <= x)) {
    return -1;
  }

  if ((y < BLOCK_TOP) || (y > BLOCK_BOTTOM)) {
    return -1;
  }

  int8_t block = (y - BLOCK_TOP) / BLOCK_HEIGHT * BLOCK_COLS + x / BLOCK_WIDTH;
  if (0 <= block && block < N_BLOCKS) {
    return block;
  } else {
    return -1;
  }
}

// Block related methods
int8_t BlocksHit(int16_t x, int16_t y, int16_t &dx, int16_t &dy) {
  int8_t block;

  block = BlocksFind((int16_t)(x + dx), (int16_t)(y));
  if ((block >= 0) && blocks[block]) {
    dx = -dx;
    return block;
  }

  block = BlocksFind((int16_t)(x), (int16_t)(y + dy));
  if ((block >= 0) && blocks[block]) {
    dy = -dy;
    return block;
  }

  block = BlocksFind((int16_t)(x + dx), (int16_t)(y + dy));
  if ((block >= 0) && blocks[block]) {
    dx = -dx;
    dy = -dy;
    return block;
  }

  return -1;
}

void BlocksCheckHit(void) {
  int8_t block;

  do {
    block = BlocksHit(ball.x, ball.y, ball.dx, ball.dy);
    if (block >= 0) {
      tone(PIN_SOUND, HIT_BLOCK, 40);
      blocks[block] = false;
      BlocksEraseOne(block);
    }
  } while (block != -1);
}

// Move Ball and Racket
void MoveBall(void) {
  int16_t nx = ABS(ball.dx);
  int16_t ny = ABS(ball.dy);
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
      if (ball.y == RACKET_LINE - RACKET_HEIGHT) {
        if (ball.x >= racket && ball.x < racket + RACKET_WIDTH) {
          ball.dy = -ball.dy;
          dy = -dy;
          tone(PIN_SOUND, HIT_RACKET, 20);
        }
      }
    }

    if (ball.y == 0) {
      ball.dy = -ball.dy;
      dy = -dy;
    }

    BlocksCheckHit();
    DrawBall(ball, tft, YELLOW);
  } while (nx > 0 || ny > 0);
}

void MoveRacket(void) {
  int16_t before = racket;

#if DEMO_MODE
  racket = ball.x - (RACKET_WIDTH >> 1);
  racket = MAX(0, racket);
  racket = MIN(racket, SCREEN_WIDTH - RACKET_WIDTH);
#else
  racket = map(analogRead(PIN_RACKET), 0, 1023, -5, SCREEN_WIDTH - RACKET_WIDTH + 5);
  racket = constrain(racket, 0, SCREEN_WIDTH - RACKET_WIDTH);
#endif

  if (before != racket) {
    DrawRacket(before, tft, BLACK);
  }

  DrawRacket(racket, tft, WHITE);  
}

void StartPlaying(void) {
  ClearScreen();
  BallInit(ball);
  BlocksInit();
  BlocksDrawAll();
}

void UpdateStatus(void) {
  static Status status = OPENING;

  switch (status) {
    case OPENING:
      status = START;
      break;
    case START:
      StartPlaying();
      status = PLAYING;
      break;
    case PLAYING:
      if (BlocksCount() == 0) {
        status = CLEAR;
      } else if (BallLost()) {
        BallInit(ball);
      }
      break;
    case CLEAR:
      DrawBall(ball, tft, BLACK);
      status = START;
      break;
    case GAMEOVER:
      break;
  }
}

void setup() {
#if 0
  Serial.begin(115200);
  while (!Serial);
#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
#endif

  // Init ST7789
  tft.init(DEVICE_WIDTH, DEVICE_HEIGHT, SPI_MODE2);
  tft.setRotation(2);
}

// Non-preemptive multitasking
#define DO_EVERY(period, prev)  static uint32_t prev = 0; for (uint32_t now = millis(); now - prev >= period; prev = now)

void loop() {
  UpdateStatus();

  DO_EVERY(16, TimeBall) {
    MoveBall();
  }

  DO_EVERY(10, TimeRacket) {
    MoveRacket();
  }
}
