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
#include <SPI.h>
#include <SD.h>

WROVER_KIT_LCD tft;

#define SD_CS 4

void setup(void) {
  Serial.begin(9600);

  tft.begin();
  tft.fillScreen(WROVER_BLUE);
  
  yield();

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  }
  Serial.println("OK!");

  tft.drawBmpFile(SD, "purple.bmp", 0, 0);
}

void loop() {
}
