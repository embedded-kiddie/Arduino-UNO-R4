// Adafruit_GFX compatible example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example renders a Jpeg file that is stored in an array within Flash (program) memory
// see ferarri.h tab.  The ferarri image file being ~11Kbytes.

// Include the array
#include "ferrari.h"
#include "ford.h"
#include "audi.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

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

#define TFT_BLACK       0x0000
#define TFT_RED         0xF800

// Include the TFT library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // In ILI9341 library this function clips the image block at TFT boundaries
  tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n Testing TJpg_Decoder library");

  // Initialise the TFT
  tft.init(DEVICE_WIDTH, DEVICE_HEIGHT, SPI_MODE2);
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
}

void drawImage(const uint8_t img[], uint16_t size) {
//tft.fillScreen(TFT_RED);

  // Time recorded for test purposes
  uint32_t t = millis();

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, img, size);
  Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);

  // Scaling
  if (w == 480 || h == 480) {
    TJpgDec.setJpgScale(2);
  } else {
    TJpgDec.setJpgScale(1);
  }

  // Draw the image, top left at 0,0
  TJpgDec.drawJpg(0, 0, img, size);

  // How much time did rendering take (ESP8266 80MHz 473ms, 160MHz 266ms, ESP32 SPI 116ms)
  t = millis() - t;
  Serial.print(t); Serial.println(" ms");
}

void loop() {
  drawImage(ferrari, sizeof(ferrari));
  delay(3000);

  drawImage(ford, sizeof(ford));
  delay(3000);

  drawImage(audi, sizeof(audi));
  delay(3000);
}
