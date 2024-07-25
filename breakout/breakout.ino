#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define DEMO_MODE 1

#define PIN_RACKET  A5  // Potentiometer or Joystick
#define PIN_SOUND   7   // Buzzer

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

// Wall (coodinate on the screen)
#define WALL_TOP      0
#define WALL_LEFT     0
#define WALL_RIGHT    (SCREEN_WIDTH - 1)

// Ball
#define BALL_SIZE     8 // size on the device
#define BALL_MOVE_X   (4 - SCREEN_SCALE) // coodinate on the screen
#define BALL_MOVE_Y   (4 - SCREEN_SCALE) // coodinate on the screen
#if DEMO_MODE == 1
#define BALL_CYCLE    16  // [msec]
#else
#define BALL_CYCLE    40  // [msec]
#endif

// Paddle (coodinate on the screen)
#define PADDLE_WIDTH  (40 >> SCREEN_SCALE)
#define PADDLE_HEIGHT ( 8 >> SCREEN_SCALE)
#define PADDLE_TOP    (SCREEN_HEIGHT - PADDLE_HEIGHT)
#define PADDLE_CYCLE  16

// Block (coodinate on the screen)
#define BLOCK_ROWS    5
#define BLOCK_COLS    10
#define BLOCK_WIDTH   (SCREEN_WIDTH / BLOCK_COLS)
#define BLOCK_HEIGHT  (8 >> SCREEN_SCALE)
#define BLOCK_TOP     (6 -  SCREEN_SCALE + WALL_TOP)
#define BLOCK_END(t)  ((t) + BLOCK_ROWS * BLOCK_HEIGHT - 1)
#define N_BLOCKS      (BLOCK_ROWS * SCREEN_WIDTH / BLOCK_WIDTH)

// Font size for setTextSize(2)
#define FONT_SIZE_X   12
#define FONT_SIZE_Y   15

// Game score
#define REFRESH_SCORE 2
#define REFRESH_ALL   3

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

// Tone frequency
#define HIT_BLOCK   NOTE_C4
#define HIT_RACKET  NOTE_C3

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > (0) ? (a) : -(a))
#define SIGN(a)   ((a) > (0) ? (1) : (-1))

#define ClearScreen() tft.fillScreen(BLACK)

#if SCREEN_SCALE == 2
#define DrawBall(ball, tft, color)  tft.fillCircle(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), BALL_SIZE >> 1, color)
#else
#define DrawBall(ball, tft, color)  tft.fillRect(SCREEN_DEV(ball.x), SCREEN_DEV(ball.y), BALL_SIZE, BALL_SIZE, color)
#endif
#define DrawPaddle(paddle, tft, color)  tft.fillRect(SCREEN_DEV(paddle), SCREEN_DEV(PADDLE_TOP), SCREEN_DEV(PADDLE_WIDTH), SCREEN_DEV(PADDLE_HEIGHT), color)

typedef enum {
  OPENING,
  START,
  PLAYING,
  CLEAR,
  GAMEOVER,
} Status_t;

typedef struct {
  uint16_t  level;
  uint16_t  score;
  uint16_t  balls;
  uint16_t  ball_cycle;
  uint16_t  block_top;
  uint16_t  block_end;
  uint16_t  paddle_width;
} Game_t;

typedef struct {
  int16_t   x, y;
  int16_t   dx, dy;
} Ball_t;

// Global variables
Game_t game;
Ball_t ball;
int16_t paddle;
bool blocks[N_BLOCKS];

// Game related method
void GameInit(Game_t &game) {
  game.level = 1;
  game.score = 0;
  game.balls = 5;
  game.ball_cycle = BALL_CYCLE;
  game.block_top = BLOCK_TOP;
  game.block_end = BLOCK_END(BLOCK_TOP);
  game.paddle_width = PADDLE_WIDTH;
}

void GameShow(int refresh = 0) {
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(5, 0);
  tft.print("Lv:");

  // Level (3 digits)
  if (refresh == REFRESH_ALL) {
    tft.fillRect(40, 0, FONT_SIZE_X * 3, FONT_SIZE_Y, BLACK);
  }
  if (refresh != REFRESH_SCORE) {
    tft.setCursor(40, 0);
    tft.print(game.level);
  }

  // Score (5 digits)
  if (refresh & REFRESH_SCORE) {
    tft.fillRect(96, 0, FONT_SIZE_X * 5, FONT_SIZE_Y, BLACK);
  }
  char buf[8];
  sprintf(buf, "%05d", game.score);
  tft.setCursor(96, 0);
  tft.print(buf);

  // Balls (5 digits)
  if (refresh == REFRESH_ALL) {
    tft.fillRect(175, 0, DEVICE_WIDTH - 175, SCREEN_DEV(game.block_top - 1), BLACK);
  }
  if (refresh != REFRESH_SCORE) {
    for (int i = 0; i < game.balls; i++) {
      tft.fillCircle(230 - (i * BALL_SIZE * 3 / 2), BALL_SIZE >> 1, BALL_SIZE >> 1, YELLOW);
    }
  }
}

// Ball related methods
void BallInit(Ball_t &ball) {
  DrawBall(ball, tft, BLACK);

  int16_t x = random(0, SCREEN_WIDTH); // 30, 19, 56, 36, 3
  ball = {
    .x  = (int16_t)x,
    .y  = (int16_t)(game.block_end + BLOCK_HEIGHT),
    .dx = (int16_t)(x > (SCREEN_WIDTH >> 1) ? -BALL_MOVE_X : BALL_MOVE_X),
    .dy = (int16_t)BALL_MOVE_Y
  };
}

bool BallLost(void) {
  if (ball.y >= PADDLE_TOP) {
    return true;
  } else {
    return false;
  }
}

// Block related methods
void BlocksInit() {
  for(int16_t i = 0; i < N_BLOCKS; i++) {
    blocks[i] = true;
  }
}

int16_t BlocksCount() {
  int16_t n = 0;
  
  for (int16_t i = 0; i < N_BLOCKS; i++) {
    if (blocks[i])
      n++;
  }

  return n;
}

void BlocksDrawAll() {
  static const uint16_t colors[BLOCK_ROWS] PROGMEM = {CYAN, MAGENTA, YELLOW, RED, GREEN};

  int16_t x, y;
  int16_t i = 0;
  int16_t c = 0;

  for(y = game.block_top; y <= game.block_end; y += BLOCK_HEIGHT, c++) {
    for(x = 0; x < SCREEN_WIDTH; x += BLOCK_WIDTH) {
      if (blocks[i++]) {
        tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), pgm_read_word(&colors[c]));
        tft.drawRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
      }
    }
  }
}

void BlocksEraseOne(int16_t block) {
  int16_t x = (block % BLOCK_COLS) * BLOCK_WIDTH;
  int16_t y = (block / BLOCK_COLS) * BLOCK_HEIGHT + game.block_top;
  tft.fillRect(SCREEN_DEV(x), SCREEN_DEV(y), SCREEN_DEV(BLOCK_WIDTH), SCREEN_DEV(BLOCK_HEIGHT), BLACK);
}

int16_t BlocksFind(int16_t x, int16_t y) {
  if ((x < WALL_LEFT) || (WALL_RIGHT < x)) {
    return -1;
  }

  if ((y < game.block_top) || (y > game.block_end)) {
    return -1;
  }

  int16_t block = (y - game.block_top) / BLOCK_HEIGHT * BLOCK_COLS + x / BLOCK_WIDTH;
  if (0 <= block && block < N_BLOCKS) {
    return block;
  } else {
    return -1;
  }
}

int16_t BlocksHit(int16_t x, int16_t y, int16_t &dx, int16_t &dy) {
  int16_t block;

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
  int16_t block;

  do {
    block = BlocksHit(ball.x, ball.y, ball.dx, ball.dy);
    if (block >= 0) {
      tone(PIN_SOUND, HIT_BLOCK, 20);
      blocks[block] = false;
      BlocksEraseOne(block);
      game.score++;
      GameShow(REFRESH_SCORE);
    }
  } while (block != -1);
}

// Move Ball
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
      if (ball.y == PADDLE_TOP - PADDLE_HEIGHT) {
        if (ball.x >= paddle && ball.x < paddle + PADDLE_WIDTH) {
          ball.dy = -ball.dy;
          dy = -dy;
          tone(PIN_SOUND, HIT_RACKET, 20);
        }
      }
    }

    if (ball.y == WALL_TOP) {
      ball.dy = -ball.dy;
      dy = -dy;
    }

    if (ball.y <= BLOCK_TOP + 1) {
      GameShow();
    }
  
    BlocksCheckHit();
    DrawBall(ball, tft, YELLOW);
  } while (nx > 0 || ny > 0);
}

void MovePaddle(void) {
  int16_t before = paddle;

#if DEMO_MODE
  paddle = ball.x - (PADDLE_WIDTH >> 1);
  paddle = MIN(MAX(paddle, WALL_LEFT), WALL_RIGHT - PADDLE_WIDTH + 1);
#else
  paddle = map(analogRead(PIN_RACKET), 0, 1023, -5, SCREEN_WIDTH - PADDLE_WIDTH + 5);
  paddle = constrain(paddle, WALL_LEFT, WALL_RIGHT - PADDLE_WIDTH + 1);
#endif

  if (before != paddle) {
    DrawPaddle(before, tft, BLACK);
  }

  DrawPaddle(paddle, tft, WHITE);  
}

void StartPlaying(void) {
  BallInit(ball);
  BlocksInit();
  BlocksDrawAll();
  GameShow(REFRESH_ALL);
}

void UpdateStatus(void) {
  static Status_t status = OPENING;
  switch (status) {
    case OPENING:
      ClearScreen();
      GameInit(game);
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
        game.balls--;
        GameShow(REFRESH_ALL);
        if (game.balls == 0) {
          status = GAMEOVER;
        } else {
          BallInit(ball);
        }
      }
      break;
    case CLEAR:
      game.level++;
      game.ball_cycle = MAX(game.ball_cycle - 1, (DEMO_MODE ? BALL_CYCLE : BALL_CYCLE / 2));
      game.block_top  = MIN(game.block_top  + 1, BLOCK_TOP + 10);
      game.block_end  = BLOCK_END(game.block_top);
      DrawBall(ball, tft, BLACK);
      status = START;
      break;
    case GAMEOVER:
      while (1);
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
  tft.init(DEVICE_WIDTH, DEVICE_HEIGHT, SPI_MODE2); // or SPI_MODE3
  tft.setRotation(2);
}

// Non-preemptive multitasking
#define DO_EVERY(period, prev)  static uint32_t prev = 0; for (uint32_t now = millis(); now - prev >= period; prev = now)

void loop() {
  UpdateStatus();

  DO_EVERY(game.ball_cycle, TimeBall) {
    MoveBall();
  }

  DO_EVERY(PADDLE_CYCLE, TimePaddle) {
    MovePaddle();
  }
}
