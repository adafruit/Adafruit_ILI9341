/*!
* @file Adafruit_ILI9341.cpp
*
* @mainpage Adafruit ILI9341 TFT Displays
*
* @section intro_sec Introduction
*
* This is the documentation for Adafruit's ILI9341 driver for the
* Arduino platform. 
*
* This library works with the Adafruit 2.8" Touch Shield V2 (SPI)
*    http://www.adafruit.com/products/1651
*
* Adafruit 2.4" TFT LCD with Touchscreen Breakout w/MicroSD Socket - ILI9341
*    https://www.adafruit.com/product/2478
*
* 2.8" TFT LCD with Touchscreen Breakout Board w/MicroSD Socket - ILI9341
*    https://www.adafruit.com/product/1770
*
* 2.2" 18-bit color TFT LCD display with microSD card breakout - ILI9340
*    https://www.adafruit.com/product/1770
*
* TFT FeatherWing - 2.4" 320x240 Touchscreen For All Feathers 
*    https://www.adafruit.com/product/3315
*
* These displays use SPI to communicate, 4 or 5 pins are required
* to interface (RST is optional).
*
* Adafruit invests time and resources providing this open source code,
* please support Adafruit and open-source hardware by purchasing
* products from Adafruit!
*
* @section dependencies Dependencies
*
* This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
* Adafruit_GFX</a> being present on your system. Please make sure you have
* installed the latest version before using this library.
*
* @section author Author
*
* Written by Limor "ladyada" Fried for Adafruit Industries.
*
* @section license License
*
* BSD license, all text here must be included in any redistribution.
*
*/

#include "Adafruit_ILI9341.h"
#ifndef ARDUINO_STM32_FEATHER
  #include "pins_arduino.h"
#ifndef RASPI
    #include "wiring_private.h"
#endif
#endif
#include <limits.h>


/*
 * Final SPI Macros
 * */
#if defined (ARDUINO_ARCH_ARC32) || defined (ARDUINO_MAXIM)
#define SPI_DEFAULT_FREQ         16000000
#elif defined (__AVR__) || defined(TEENSYDUINO)
#define SPI_DEFAULT_FREQ         8000000
#elif defined(ESP8266) || defined(ESP32)
#define SPI_DEFAULT_FREQ         40000000
#elif defined(RASPI)
#define SPI_DEFAULT_FREQ         80000000
#elif defined(ARDUINO_ARCH_STM32F1)
#define SPI_DEFAULT_FREQ         36000000
#else
#define SPI_DEFAULT_FREQ         24000000      ///< Default SPI data clock frequency
#endif


/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ILI9341 driver with software SPI
    @param    cs    Chip select pin #
    @param    dc    Data/Command pin #
    @param    mosi  SPI MOSI pin #
    @param    sclk  SPI Clock pin #
    @param    rst   Reset pin # (optional, pass -1 if unused)
    @param    miso  SPI MISO pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t mosi,
        int8_t sclk, int8_t rst, int8_t miso)
  : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, cs, dc, mosi, sclk, rst, miso) {

}

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ILI9341 driver with hardware SPI
    @param    cs    Chip select pin #
    @param    dc    Data/Command pin #
    @param    rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst) 
  : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, cs, dc, rst) {

}

/**************************************************************************/
/*!
    @brief   Initialize ILI9341 chip
    Connects to the ILI9341 over SPI and sends initialization procedure commands
    @param    freq  Desired SPI clock frequency
*/
/**************************************************************************/
void Adafruit_ILI9341::begin(uint32_t freq)
{
    if(!freq){
        freq = SPI_DEFAULT_FREQ;
    }
    _freq = freq;

    initSPI(freq);

    startWrite();

    writeCommand(0xEF);
    spiWrite(0x03);
    spiWrite(0x80);
    spiWrite(0x02);

    writeCommand(0xCF);
    spiWrite(0x00);
    spiWrite(0XC1);
    spiWrite(0X30);

    writeCommand(0xED);
    spiWrite(0x64);
    spiWrite(0x03);
    spiWrite(0X12);
    spiWrite(0X81);

    writeCommand(0xE8);
    spiWrite(0x85);
    spiWrite(0x00);
    spiWrite(0x78);

    writeCommand(0xCB);
    spiWrite(0x39);
    spiWrite(0x2C);
    spiWrite(0x00);
    spiWrite(0x34);
    spiWrite(0x02);

    writeCommand(0xF7);
    spiWrite(0x20);

    writeCommand(0xEA);
    spiWrite(0x00);
    spiWrite(0x00);

    writeCommand(ILI9341_PWCTR1);    //Power control
    spiWrite(0x23);   //VRH[5:0]

    writeCommand(ILI9341_PWCTR2);    //Power control
    spiWrite(0x10);   //SAP[2:0];BT[3:0]

    writeCommand(ILI9341_VMCTR1);    //VCM control
    spiWrite(0x3e);
    spiWrite(0x28);

    writeCommand(ILI9341_VMCTR2);    //VCM control2
    spiWrite(0x86);  //--

    writeCommand(ILI9341_MADCTL);    // Memory Access Control
    spiWrite(0x48);

    writeCommand(ILI9341_VSCRSADD); // Vertical scroll
    SPI_WRITE16(0);                 // Zero

    writeCommand(ILI9341_PIXFMT);
    spiWrite(0x55);

    writeCommand(ILI9341_FRMCTR1);
    spiWrite(0x00);
    spiWrite(0x18);

    writeCommand(ILI9341_DFUNCTR);    // Display Function Control
    spiWrite(0x08);
    spiWrite(0x82);
    spiWrite(0x27);

    writeCommand(0xF2);    // 3Gamma Function Disable
    spiWrite(0x00);

    writeCommand(ILI9341_GAMMASET);    //Gamma curve selected
    spiWrite(0x01);

    writeCommand(ILI9341_GMCTRP1);    //Set Gamma
    spiWrite(0x0F);
    spiWrite(0x31);
    spiWrite(0x2B);
    spiWrite(0x0C);
    spiWrite(0x0E);
    spiWrite(0x08);
    spiWrite(0x4E);
    spiWrite(0xF1);
    spiWrite(0x37);
    spiWrite(0x07);
    spiWrite(0x10);
    spiWrite(0x03);
    spiWrite(0x0E);
    spiWrite(0x09);
    spiWrite(0x00);

    writeCommand(ILI9341_GMCTRN1);    //Set Gamma
    spiWrite(0x00);
    spiWrite(0x0E);
    spiWrite(0x14);
    spiWrite(0x03);
    spiWrite(0x11);
    spiWrite(0x07);
    spiWrite(0x31);
    spiWrite(0xC1);
    spiWrite(0x48);
    spiWrite(0x08);
    spiWrite(0x0F);
    spiWrite(0x0C);
    spiWrite(0x31);
    spiWrite(0x36);
    spiWrite(0x0F);

    writeCommand(ILI9341_SLPOUT);    //Exit Sleep
    delay(120);
    writeCommand(ILI9341_DISPON);    //Display on
    delay(120);
    endWrite();

    _width  = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;
}


/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ILI9341::setRotation(uint8_t m) {
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            m = (MADCTL_MX | MADCTL_BGR);
            _width  = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 1:
            m = (MADCTL_MV | MADCTL_BGR);
            _width  = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
        case 2:
            m = (MADCTL_MY | MADCTL_BGR);
            _width  = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 3:
            m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width  = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
    }

    startWrite();
    writeCommand(ILI9341_MADCTL);
    spiWrite(m);
    endWrite();
}

/**************************************************************************/
/*!
    @brief   Enable/Disable display color inversion
    @param   invert True to invert, False to have normal color
*/
/**************************************************************************/
void Adafruit_ILI9341::invertDisplay(boolean invert) {
    startWrite();
    writeCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
    endWrite();
}

/**************************************************************************/
/*!
    @brief   Scroll display memory
    @param   y How many pixels to scroll display by
*/
/**************************************************************************/
void Adafruit_ILI9341::scrollTo(uint16_t y) {
    startWrite();
    writeCommand(ILI9341_VSCRSADD);
    SPI_WRITE16(y);
    endWrite();
}

/**************************************************************************/
/*!
    @brief   Set the "address window" - the rectangle we will write to RAM with the next chunk of SPI data writes. The ILI9341 will automatically wrap the data as each row is filled
    @param   x  TFT memory 'x' origin
    @param   y  TFT memory 'y' origin
    @param   w  Width of rectangle
    @param   h  Height of rectangle
*/
/**************************************************************************/
void Adafruit_ILI9341::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
    uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
    writeCommand(ILI9341_CASET); // Column addr set
    SPI_WRITE32(xa);
    writeCommand(ILI9341_PASET); // Row addr set
    SPI_WRITE32(ya);
    writeCommand(ILI9341_RAMWR); // write to RAM
}


/**************************************************************************/
/*!
   @brief  Read 8 bits of data from ILI9341 configuration memory. NOT from RAM!
           This is highly undocumented/supported, it's really a hack but kinda works?
    @param    command  The command register to read data from
    @param    index  The byte index into the command to read from
    @return   Unsigned 8-bit data read from ILI9341 register
*/
/**************************************************************************/
uint8_t Adafruit_ILI9341::readcommand8(uint8_t command, uint8_t index) {
    uint32_t freq = _freq;
    if(_freq > 24000000){
        _freq = 24000000;
    }
    startWrite();
    writeCommand(0xD9);  // woo sekret command?
    spiWrite(0x10 + index);
    writeCommand(command);
    uint8_t r = spiRead();
    endWrite();
    _freq = freq;
    return r;
}
