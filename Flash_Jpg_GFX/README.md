# Jpeg images in Flash memory

This sketch renders jpeg images on Flash memory onto the LCD.

## Dependencies
- [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder "Bodmer/TJpg_Decoder: Jpeg decoder library based on Tiny JPEG Decompressor")
- [adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library "adafruit/Adafruit-GFX-Library: Adafruit GFX graphics core Arduino library, this is the &#39;core&#39; class that all our other graphics libraries derive from")
- [Adafruit ST7735 and ST7789 Library](https://www.arduino.cc/reference/en/libraries/adafruit-st7735-and-st7789-library/ "Adafruit ST7735 and ST7789 Library - Arduino Reference")

## Wiring diagram

![Wiring](LCD240x240.png "DiyStudio 1.3”color IPS LCD 240x240 with ST7789VW")

### Pin Assignments

SPI pin definition for Arduino UNO R3 and R4

  | ST7798 | PIN  |  R3  |   R4   |     Description      |
  |--------|------|------|--------|----------------------|
  | SCL    |  D13 | SCK  | RSPCKA | Serial clock         |
  | SDA    | ~D11 | COPI | COPIA  | Serial data input    |
  | RES    | ~D9  | PB1  | P303   | Reset signal         |
  | DC     |  D8  | PB0  | P304   | Display data/command |

## How to store jpeg data into flash memory

1. Install the dependencies into the Arduino IDE library.

2. Prepare a JPEG image that matche the resolution of your LCD.

3. Visit the following sites to convert jpeg image into hexadecimal text data.

  - [File to hex converter](http://tomeko.net/online_tools/file_to_hex.php?lang=en "File to hex converter")
  - [image2cpp](https://javl.github.io/image2cpp/ "image2cpp")

4. Create a new header file and paste hexadecimal text data as follows:

```c++
const uint8_t name_of_image[] PROGMEM = {
  0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x48, 
  ...
};
```

5. Include the created header file in [`Flash_Jpg_GFX.ino`](Flash_Jpg_GFX.ino).

```c++
#include "name_of_image.h"
...
```

6. Add code to render to the LCD.

```c++
void loop() {
  drawImage(name_of_image, sizeof(name_of_image));
  delay(3000);
}
```
