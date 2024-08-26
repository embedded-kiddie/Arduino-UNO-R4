
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
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
#define SD_CS         D6
#define SPI_MODE      SPI_MODE3 // SPI_MODE3

#else
#warning "must specify board type"
#endif
