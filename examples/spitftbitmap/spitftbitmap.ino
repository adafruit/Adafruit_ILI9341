/***************************************************
  This is our Bitmap drawing example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include "WROVER_KIT_LCD.h" // Hardware-specific library
#include <SD_MMC.h>

WROVER_KIT_LCD tft;

void setup(void) {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(WROVER_BLUE);
  
  Serial.print("Initializing SD card...");
  if (!SD_MMC.begin()) {
    Serial.println("failed!");
    return;
  }
  Serial.println("OK!");

  tft.drawBmpFile(SD_MMC, "/purple.bmp", 0, 0);
}

void loop() {}
