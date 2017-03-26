// Simple example to embed pictures in your sketch
// and draw on the ILI9341 display with writeRect()
//
// By Frank Bösing
//
// https://forum.pjrc.com/threads/32601-SPI-Library-Issue-w-ILI9341-TFT-amp-PN532-NFC-Module-on-Teensy-3-2?p=94534&viewfull=1#post94534

#include "SPI.h"
#include <Adafruit_ILI9341.h>

// New picture format, as converted by 
// http://www.rinkydinkelectronics.com/t_imageconverter565.php
#include "picture1.c"

// Original picture convertion, with a header to skip (from Frank Bösing)
#include "picture2.c"

// My own logo for the IoTuz board (rolled out in Tasmania)
#include "tasmanian-devil.c"

/* GIMP (https://www.gimp.org/) can also be used to export the image using the following steps:

    1. File -> Export As
    2. In the Export Image dialog, use 'C source code (*.c)' as filetype.
    3. Press export to get the export options dialog.
    4. Type the desired variable name into the 'prefixed name' box.
    5. Uncheck 'GLIB types (guint8*)'
    6. Check 'Save as RGB565 (16-bit)'
    7. Press export to save your image.

  Assuming 'image_name' was typed in the 'prefixed name' box of step 4, you can have to include the c file as above,
  using the image can be done with:

    tft.drawBitmap(0, 0, image_name.width, image_name.height, (uint16_t*)(image_name.pixel_data));

  See also https://forum.pjrc.com/threads/35575-Export-for-ILI9341_t3-with-GIMP 
*/

// Normal Connections
#define TFT_DC       9
#define TFT_CS      10
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12

// Alternate Connections with Teensy Audio Shield
//#define TFT_DC      20
//#define TFT_CS      21
//#define TFT_RST    255  // 255 = unused, connect to 3.3V
//#define TFT_MOSI     7
//#define TFT_SCLK    14
//#define TFT_MISO    12

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.drawBitmap(33, 32, 256, 174, (uint16_t*)picture1);
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  tft.drawBitmap(1, 1, 256, 174, (uint16_t*)picture2+35);
  delay(2000);
  tft.drawBitmap(0, 0, 320, 240, (uint16_t*)picture3);
  delay(2000);
  for (int16_t addr = 320; addr -=2; addr >= 0)
  {
    tft.scrollTo(addr);
    delay(10);
  }
}

void loop(void) {
}
