#include "Adafruit_GFX.h"
#include "WROVER_KIT_LCD.h"
#include "esp_wp1.h"
#include "esp_wp2.h"

#define BUTTON 0

WROVER_KIT_LCD tft;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  tft.begin();
  tft.setRotation(0);
  tft.drawJpg(esp_wp2_jpg, esp_wp2_jpg_len);
}

void loop() {
  if(!digitalRead(BUTTON)){
    tft.drawJpg(esp_wp1_jpg, esp_wp1_jpg_len);
  } else {
    tft.drawJpg(esp_wp2_jpg, esp_wp2_jpg_len);
  }
  delay(1000);
}
