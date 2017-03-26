// ILI9341 example with embedded color bitmaps in sketch.
// WILL NOT FIT ON ARDUINO UNO OR OTHER AVR BOARDS;
// uses large bitmap image stored in array!

#include "SPI.h"
#include <Adafruit_ILI9341.h>
#include "dragon.h"

// For the Adafruit shield, these are the default.
//#define TFT_DC 9
//#define TFT_CS 10

// Feather 32u4 or M0 with TFT FeatherWing:
#define TFT_DC 10
#define TFT_CS  9

// Other boards (including Feather boards) may have other pinouts;
// see learn.adafruit.com/adafruit-2-4-tft-touch-screen-featherwing/pinouts

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  tft.begin();
}

void loop(void) {
  for(uint8_t r=0; r<4; r++) {
    tft.setRotation(r);
    tft.fillScreen(ILI9341_BLACK);
    for(uint8_t j=0; j<20; j++) {
      tft.drawBitmap(
        random(-DRAGON_WIDTH , tft.width()),
        random(-DRAGON_HEIGHT, tft.height()),
        DRAGON_WIDTH, DRAGON_HEIGHT, (uint16_t*)dragonBitmap);
    }
    delay(3000);
  }
}
