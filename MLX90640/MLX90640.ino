#include <Adafruit_MLX90640.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
/* SPI pin definition for Arduino UNO R3 and R4
  | ST7789 | PIN  |  R3  |   R4   |     Description      |
  |--------|------|------|--------|----------------------|
  | SCL    |  D13 | SCK  | RSPCKA | Serial clock         |
  | SDA    | ~D11 | COPI | COPIA  | Serial data input    |
  | RES    | ~D9  | PB1  | P303   | Reset signal         |
  | DC     |  D8  | PB0  | P304   | Display data/command |
*/
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#define SPI_MODE      SPI_MODE3 // SPI_MODE2 or SPI_MODE3

#elif defined(ARDUINO_XIAO_ESP32S3)
// Seeed Studio XIAO ESP32-S3
#define TFT_MISO      D9
#define TFT_MOSI      D10
#define TFT_SCLK      D8
#define TFT_DC        D1
#define TFT_CS        (-1) // dummy
#define TFT_RST       (-1) // Or set to -1 and connect to Arduino RESET pin
#define SPI_MODE      SPI_MODE3 // SPI_MODE3

#else
#warning "must specify board type"
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define DEVICE_WIDTH  240
#define DEVICE_HEIGHT 240
#define DEVICE_ORIGIN 2

// Font size for setTextSize(2)
#define FONT_WIDTH    12 // [px] (Device coordinate system)
#define FONT_HEIGHT   16 // [px] (Device coordinate system)

#define ClearScreen() tft.fillScreen(ST77XX_BLACK)

Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

//low range of the sensor (this will be blue on the screen)
#define MINTEMP 20

//high range of the sensor (this will be red on the screen)
#define MAXTEMP 35

//the colors we will be using
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

uint16_t displayPixelWidth, displayPixelHeight;

void TFT_Printf(uint8_t x, uint8_t y, const char* fmt, ...) {
  int len = 0;
  char buf[16];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, arg_ptr);
  va_end(arg_ptr);

  tft.fillRect(x, y, len * FONT_WIDTH, FONT_HEIGHT, ST77XX_BLACK);
  tft.setCursor(x, y);
  tft.print(buf);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif

  // Initialize ST7789
  tft.init(DEVICE_WIDTH, DEVICE_HEIGHT, SPI_MODE);
  tft.setRotation(DEVICE_ORIGIN);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
#if defined (ARDUINO_XIAO_ESP32S3)
  tft.setSPISpeed(80000000);
#endif
  ClearScreen();

  displayPixelWidth = DEVICE_WIDTH / 32;
  displayPixelHeight = DEVICE_HEIGHT / 32; //Keep pixels square 
 
  // Draw color bar
  const long n = sizeof(camColors) / sizeof(camColors[0]) - 1;
  const long w = displayPixelWidth * 32;
  for (int i = 0; i <= n; i++) {
    int16_t j = map(i, 0, n, 0, w);
    tft.fillRect(j, DEVICE_HEIGHT - FONT_HEIGHT * 4, 1, FONT_HEIGHT - 4, camColors[i]);
  }

  int y = DEVICE_HEIGHT - FONT_HEIGHT * 3;
  TFT_Printf(FONT_WIDTH *  0, y, "%d", MINTEMP);
  TFT_Printf(FONT_WIDTH *  8, y, "%3.1f", (float)(MINTEMP + MAXTEMP) / 2.0f);
  TFT_Printf(FONT_WIDTH * 17, y, "%d", MAXTEMP);

  delay(100);

  Serial.println("Adafruit MLX90640 Camera");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);
  
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);

#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)

  mlx.setRefreshRate(MLX90640_4_HZ); // 4 FPS
  Wire.setClock(400000); // 400 KHz

#elif defined(ARDUINO_XIAO_ESP32S3)

//mlx.setRefreshRate(MLX90640_16_HZ); // unstable
  mlx.setRefreshRate(MLX90640_8_HZ);  // 8 FPS
  Wire.setClock(400000); // 400 KHz

#endif
}

void loop() {
  uint32_t timestamp = millis();
  if (mlx.getFrame(frame) != 0) {
    TFT_Printf(DEVICE_WIDTH / 2 - FONT_WIDTH * 3, DEVICE_WIDTH / 2 - FONT_HEIGHT * 3, "Failed");
    Serial.println("Failed");
    delay(1000); // false = no new frame capture
    return;
  }

  // Ambient temperature
  float v = mlx.getTa(false);
  if (v > 0) {
    TFT_Printf(FONT_WIDTH * 10, DEVICE_HEIGHT - FONT_HEIGHT, "%4.1f'C", v);  // false = no new frame capture
  }

  int colorTemp;
  for (uint8_t h = 0; h < 24; h++) {
    for (uint8_t w = 0; w < 32; w++) {
      float t = frame[h * 32 + w];
      //Serial.print(t, 1); Serial.print(", ");

      t = min((int)t, MAXTEMP);
      t = max((int)t, MINTEMP); 
           
      uint8_t colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);

      //draw the pixels!
      tft.fillRect(displayPixelWidth * w, displayPixelHeight * h,
                               displayPixelHeight, displayPixelWidth, 
                               camColors[colorIndex]);
    }
  }

  // FPS
  v = 2000.0f / (float)(millis() - timestamp); // 2 frames per display
  TFT_Printf(0, DEVICE_HEIGHT - FONT_HEIGHT, "%3.2f FPS", v);
}