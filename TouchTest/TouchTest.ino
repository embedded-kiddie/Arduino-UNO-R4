#include <XPT2046_Touchscreen.h>
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
//#define TFT_SCLK      13
//#define TFT_MISO      12
//#define TFT_MOSI      11
#define TFT_CS        10
#define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        8
#define TOUCH_CS      7
#define TOUCH_IRQ     2
#define SPI_MODE      SPI_MODE3 // SPI_MODE2 or SPI_MODE3

#elif defined(ARDUINO_XIAO_ESP32S3)
// Seeed Studio XIAO ESP32-S3
#define TFT_SCLK      D8
#define TFT_MISO      D9
#define TFT_MOSI      D10
#define TFT_CS        D2    // (-1)  // dummy
#define TFT_RST       D1    // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        D0
#define TOUCH_CS      D3
#define TOUCH_IRQ     D7
#define SPI_MODE      SPI_MODE3 // SPI_MODE3

#else
#warning "must specify board type"
#endif

//XPT2046_Touchscreen ts(TOUCH_CS);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(TOUCH_CS, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void setup() {
  Serial.begin(115200);//38400);
  ts.begin();
  ts.setRotation(3);
  while (!Serial && (millis() <= 1000));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    delay(30);
    Serial.println();
  }
}

