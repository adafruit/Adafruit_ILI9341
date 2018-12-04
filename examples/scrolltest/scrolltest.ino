/*
 *  This sketch demonstrates how to use the hardware scrolling on a portion of the screen.
 *  It performs a basic WiFi scan to generate some terminal-like animation.
 *  Courtesy of tobozo (Dec.2018)
 * 
 */


#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include "WROVER_KIT_LCD.h"
WROVER_KIT_LCD tft;

// this only works in portrait mode (orientation=0 or 3)
uint16_t height = tft.height();// (=320)
uint16_t width  = tft.width();// (=240)

// scroll control variables
uint16_t scrollTopFixedArea = 40; // top fixed area
uint16_t scrollBottomFixedArea = 40; // bottom fixed area
uint16_t yStart = scrollTopFixedArea;
uint16_t yArea = height - scrollTopFixedArea - scrollBottomFixedArea;
uint16_t w_tmp, h_tmp;
int16_t  x1_tmp, y1_tmp;
int scrollPosY = scrollTopFixedArea; // keeps track of the cursor position
int scrollPosX = -1;

String output;


int scrollText(const char* str) {
  if (scrollPosY == -1) {
    scrollPosY = tft.getCursorY();
  }
  scrollPosX = tft.getCursorX();
  if (scrollPosY >= (height - scrollBottomFixedArea)) {
    scrollPosY = (scrollPosY % (height - scrollBottomFixedArea)) + scrollTopFixedArea;
  }
  tft.getTextBounds(str, scrollPosX, scrollPosY, &x1_tmp, &y1_tmp, &w_tmp, &h_tmp);
  if (scrollPosX == 0) {
    tft.fillRect(0, scrollPosY, width, h_tmp, WROVER_BLACK);
  } else { // fill the horizontal gap
    tft.fillRect(0, scrollPosY, w_tmp, h_tmp, WROVER_BLACK);
  }
  tft.setCursor(scrollPosX, scrollPosY);
  doScroll(h_tmp, 5); // Scroll lines, 5ms per line
  tft.print(str);
  scrollPosY = tft.getCursorY();
  return h_tmp;
}


int doScroll(int lines, int wait) {
  int yTemp = yStart;
  scrollPosY = -1;
  for (int i = 0; i < lines; i++) {
    yStart++;
    if (yStart == height - scrollBottomFixedArea) yStart = scrollTopFixedArea;
    tft.scrollTo(yStart);
    delay(wait);
  }
  return  yTemp;
}


void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation( 0 ); // portrait mode is required
  tft.setTextColor(WROVER_YELLOW);
  tft.fillScreen(WROVER_DARKGREY);
  tft.setCursor(100, 10);
  tft.print("HEADER");
  tft.setCursor(100, 300);
  tft.print("FOOTER");
  tft.setCursor(0, scrollTopFixedArea);
  // setup the scrollable area  
  tft.setupScrollArea(scrollTopFixedArea, scrollBottomFixedArea); 
  // clear it
  tft.fillRect(0, scrollTopFixedArea, width, yArea, WROVER_BLACK);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");
}

void loop() {

  scrollText("WiFi Scan Start\n");
  int n = WiFi.scanNetworks();
  scrollText("WiFi Scan done\n");
  
  if (n == 0) {
    scrollText("no networks found\n");
  } else {
    output = String(n) + " networks found\n";
    scrollText(output.c_str());
    for (int i = 0; i < n; ++i) {
      output = String(i+1) + ":" + WiFi.SSID(i) + " (" +WiFi.RSSI(i)+") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*") + "\n";
      scrollText(output.c_str());
      Serial.println(output);
      delay(10);
    }
  }
  
  delay(5000);
}
