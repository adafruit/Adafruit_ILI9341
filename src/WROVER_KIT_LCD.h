/***************************************************
  This library is written to be compatible with Adafruit's ILI9341
  library and automatically detects the display type on ESP_WROVER_KITs
  Earlier WROVERs had ILI9341, while newer releases have ST7789V

  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifndef _WROVER_KIT_LCD_H_
#define _WROVER_KIT_LCD_H_

#include "Arduino.h"
#include "FS.h"
#include "SPI.h"
#include "Adafruit_GFX.h"

#define WROVER_WIDTH       240
#define WROVER_HEIGHT      320

#define WROVER_NOP         0x00
#define WROVER_SWRESET     0x01 // Software Reset
#define WROVER_RDDID       0x04 // Read Display ID
#define WROVER_RDDST       0x09 // Read Display Status
#define WROVER_RDDPM       0x0A // Read Display Power
#define WROVER_RDDMADCTL   0x0B // Read Display Memory Data Access Mode
#define WROVER_RDDCOLMOD   0x0C // Read Display Pixel
#define WROVER_RDDIM       0x0D // Read Display Image
#define WROVER_RDDSM       0x0E // Read Display Signal
#define WROVER_RDDSDR      0x0F // Read Display Self Diagnostics
#define WROVER_SLPIN       0x10 // Sleep In
#define WROVER_SLPOUT      0x11 // Sleep Out
#define WROVER_PTLON       0x12 // Partial Mode On
#define WROVER_NORON       0x13 // Partial Mode Off
#define WROVER_INVOFF      0x20 // Display Invert Off
#define WROVER_INVON       0x21 // Display Invert On
#define WROVER_GAMSET      0x26 // Display Invert On Gamma
#define WROVER_DISPOFF     0x28 // Display Off
#define WROVER_DISPON      0x29 // Display On
#define WROVER_CASET       0x2A // Column Address Set
#define WROVER_RASET       0x2B // Row Address Set
#define WROVER_RAMWR       0x2C // Memory Write
#define WROVER_RAMRD       0x2E // Memory Read
#define WROVER_PTLAR       0x30 // Partial Start/End Address Set
#define WROVER_VSCRDEF     0x33 // Vertical Scrolling Definition
#define WROVER_TEOFF       0x34 // Tearing Effect Line Off
#define WROVER_TEON        0x35 // Tearing Effect Line On
#define WROVER_MADCTL      0x36 // Memory Data Access Control
#define WROVER_VSCRSADD    0x37 // Vertical Scrolling Start Address
#define WROVER_IDMOFF      0x38 // Idle Mode Off
#define WROVER_IDMON       0x39 // Idle Mode On
#define WROVER_COLMOD      0x3A // Interface Pixel Format
#define WROVER_RAMWRC      0x3C // Memory Write Continue
#define WROVER_RAMRDC      0x3E // Memory Read Continue
#define WROVER_TESCAN      0x44 // Set Tear Scan Line
#define WROVER_RDTESCAN    0x45 // Get Tear Scan Line
#define WROVER_WRDISBV     0x51 // Set Display Brightness
#define WROVER_RDDISBV     0x52 // Get Display Brightness
#define WROVER_WRCTRLD     0x53 // Set Display Control
#define WROVER_RDCTRLD     0x54 // Get Display Control
#define WROVER_WRCACE      0x55 // Write content adaptive brightness control and Color enhancement
#define WROVER_RDCABC      0x56 // Read content adaptive brightness control and Color enhancement
#define WROVER_WRCABCMB    0x5E // Write CABC minimum brightness
#define WROVER_RDCABCMB    0x5F // Read CABC minimum brightness
#define WROVER_RDABCSDR    0x68 // Read Automatic Brightness Control Self-Diagnostic Result
#define WROVER_PORCTRK     0xB2 // Porch setting
#define WROVER_GCTRL       0xB7 // Gate Control
#define WROVER_VCOMS       0xBB // VCOM setting
#define WROVER_LCMCTRL     0xC0 // LCM Control
#define WROVER_VDVVRHEN    0xC2 // VDV and VRH Command Enable
#define WROVER_VRHS        0xC3 // VRH Set
#define WROVER_VDVS        0xC4 // VDV Set
#define WROVER_FRCTRL2     0xC6 // Frame Rate control in normal mode
#define WROVER_PWCTRL1     0xD0 // Power Control 1
#define WROVER_RDID1       0xDA // Read ID1
#define WROVER_RDID2       0xDB // Read ID2
#define WROVER_RDID3       0xDC // Read ID3
#define WROVER_PVGAMCTRL   0xE0 // Positive Voltage Gamma control
#define WROVER_NVGAMCTRL   0xE1 // Negative Voltage Gamma control

// Color definitions
#define WROVER_BLACK       0x0000 /*   0,   0,   0 */
#define WROVER_NAVY        0x000F /*   0,   0, 128 */
#define WROVER_DARKGREEN   0x03E0 /*   0, 128,   0 */
#define WROVER_DARKCYAN    0x03EF /*   0, 128, 128 */
#define WROVER_MAROON      0x7800 /* 128,   0,   0 */
#define WROVER_PURPLE      0x780F /* 128,   0, 128 */
#define WROVER_OLIVE       0x7BE0 /* 128, 128,   0 */
#define WROVER_LIGHTGREY   0xC618 /* 192, 192, 192 */
#define WROVER_DARKGREY    0x7BEF /* 128, 128, 128 */
#define WROVER_BLUE        0x001F /*   0,   0, 255 */
#define WROVER_GREEN       0x07E0 /*   0, 255,   0 */
#define WROVER_CYAN        0x07FF /*   0, 255, 255 */
#define WROVER_RED         0xF800 /* 255,   0,   0 */
#define WROVER_MAGENTA     0xF81F /* 255,   0, 255 */
#define WROVER_YELLOW      0xFFE0 /* 255, 255,   0 */
#define WROVER_WHITE       0xFFFF /* 255, 255, 255 */
#define WROVER_ORANGE      0xFD20 /* 255, 165,   0 */
#define WROVER_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define WROVER_PINK        0xF81F


// Compatibility with ILI code.
#define ILI9341_WIDTH       WROVER_WIDTH       
#define ILI9341_HEIGHT      WROVER_HEIGHT      

#define ILI9341_RDMODE      WROVER_RDDPM       
#define ILI9341_RDMADCTL    WROVER_RDDMADCTL   
#define ILI9341_RDPIXFMT    WROVER_RDDCOLMOD   
#define ILI9341_RDIMGFMT    WROVER_RDDIM       
#define ILI9341_RDSELFDIAG  WROVER_RDDSDR      

#define ILI9341_BLACK       WROVER_BLACK       
#define ILI9341_NAVY        WROVER_NAVY        
#define ILI9341_DARKGREEN   WROVER_DARKGREEN   
#define ILI9341_DARKCYAN    WROVER_DARKCYAN    
#define ILI9341_MAROON      WROVER_MAROON      
#define ILI9341_PURPLE      WROVER_PURPLE      
#define ILI9341_OLIVE       WROVER_OLIVE       
#define ILI9341_LIGHTGREY   WROVER_LIGHTGREY   
#define ILI9341_DARKGREY    WROVER_DARKGREY    
#define ILI9341_BLUE        WROVER_BLUE        
#define ILI9341_GREEN       WROVER_GREEN       
#define ILI9341_CYAN        WROVER_CYAN        
#define ILI9341_RED         WROVER_RED         
#define ILI9341_MAGENTA     WROVER_MAGENTA     
#define ILI9341_YELLOW      WROVER_YELLOW      
#define ILI9341_WHITE       WROVER_WHITE       
#define ILI9341_ORANGE      WROVER_ORANGE      
#define ILI9341_GREENYELLOW WROVER_GREENYELLOW 
#define ILI9341_PINK        WROVER_PINK        

typedef enum {
    JPEG_DIV_NONE,
    JPEG_DIV_2,
    JPEG_DIV_4,
    JPEG_DIV_8,
    JPEG_DIV_MAX
} jpeg_div_t;

class WROVER_KIT_LCD : public Adafruit_GFX {
    protected:

    public:
        WROVER_KIT_LCD();
        virtual ~WROVER_KIT_LCD(){}

        void      begin();

        void      setRotation(uint8_t r);
        void      invertDisplay(boolean i);
        void      scrollTo(uint16_t y);
        // setupScrollArea: optionnaly setup a portion of the screen for vertical hardware scrolling
        // tfa = top fixed area, bfa = bottom fixed area
        void      setupScrollArea(uint16_t tfa, uint16_t bfa);

        // Required Non-Transaction
        void      drawPixel(int16_t x, int16_t y, uint16_t color);

        // Transaction API
        void      startWrite(void);
        void      endWrite(void);
        void      writePixel(int16_t x, int16_t y, uint16_t color);
        void      writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void      writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void      writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

        // Transaction API not used by GFX
        void      setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void      writePixel(uint16_t color);
        void      writePixels(uint16_t * colors, uint32_t len);
        void      writeColor(uint16_t color, uint32_t len);
        void      pushColor(uint16_t color);

        // Recommended Non-Transaction
        void      drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void      drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        void      fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void      drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);

        uint8_t   readcommand8(uint8_t reg, uint8_t index = 0);
        uint32_t  readId();

        uint16_t  color565(uint8_t r, uint8_t g, uint8_t b);
        void      startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void      endBitmap();

        void      drawJpg(const uint8_t * jpg_data, size_t jpg_len, uint16_t x=0, uint16_t y=0, uint16_t maxWidth=0, uint16_t maxHeight=0, uint16_t offX=0, uint16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE);
        void      drawJpgFile(fs::FS &fs, const char * path, uint16_t x=0, uint16_t y=0, uint16_t maxWidth=0, uint16_t maxHeight=0, uint16_t offX=0, uint16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE);
        void      drawBmpFile(fs::FS &fs, const char * path, uint16_t x=0, uint16_t y=0, uint16_t maxWidth=0, uint16_t maxHeight=0, uint16_t offX=0, uint16_t offY=0);

    private:
        uint32_t    _id;
        uint32_t    _freq;

        void        writeInitData(const uint8_t * data);
        void        writeCommand(uint8_t cmd);

        void        bmpSkipPixels(fs::File &file, uint8_t bitsPerPixel, size_t len);
        void        bmpAddPixels(fs::File &file, uint8_t bitsPerPixel, size_t len);


};

#endif
