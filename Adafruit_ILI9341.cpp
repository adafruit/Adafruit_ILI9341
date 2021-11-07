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

#if defined(ARDUINO_ARCH_ARC32) || defined(ARDUINO_MAXIM)
#define SPI_DEFAULT_FREQ 16000000
// Teensy 3.0, 3.1/3.2, 3.5, 3.6
#elif defined(__MK20DX128__) || defined(__MK20DX256__) ||                      \
    defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define SPI_DEFAULT_FREQ 40000000
#elif defined(__AVR__) || defined(TEENSYDUINO)
#define SPI_DEFAULT_FREQ 8000000
#elif defined(ESP8266) || defined(ESP32)
#define SPI_DEFAULT_FREQ 40000000
#elif defined(RASPI)
#define SPI_DEFAULT_FREQ 80000000
#elif defined(ARDUINO_ARCH_STM32F1)
#define SPI_DEFAULT_FREQ 36000000
#else
#define SPI_DEFAULT_FREQ 24000000 ///< Default SPI data clock frequency
#endif

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

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
    : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, cs, dc, mosi, sclk,
                      rst, miso) {}

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ILI9341 driver with hardware SPI using the
            default SPI peripheral.
    @param  cs   Chip select pin # (OK to pass -1 if CS tied to GND).
    @param  dc   Data/Command pin # (required).
    @param  rst  Reset pin # (optional, pass -1 if unused).
*/
/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, cs, dc, rst) {}

#if !defined(ESP8266)
/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ILI9341 driver with hardware SPI using
            a specific SPI peripheral (not necessarily default).
    @param  spiClass  Pointer to SPI peripheral (e.g. &SPI or &SPI1).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and
                      CS is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
*/
/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(SPIClass *spiClass, int8_t dc, int8_t cs,
                                   int8_t rst)
    : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, spiClass, cs, dc,
                      rst) {}
#endif // end !ESP8266

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ILI9341 driver using parallel interface.
    @param  busWidth  If tft16 (enumeration in Adafruit_SPITFT.h), is a
                      16-bit interface, else 8-bit.
    @param  d0        Data pin 0 (MUST be a byte- or word-aligned LSB of a
                      PORT register -- pins 1-n are extrapolated from this).
    @param  wr        Write strobe pin # (required).
    @param  dc        Data/Command pin # (required).
    @param  cs        Chip select pin # (optional, pass -1 if unused and CS
                      is tied to GND).
    @param  rst       Reset pin # (optional, pass -1 if unused).
    @param  rd        Read strobe pin # (optional, pass -1 if unused).
*/
/**************************************************************************/
Adafruit_ILI9341::Adafruit_ILI9341(tftBusWidth busWidth, int8_t d0, int8_t wr,
                                   int8_t dc, int8_t cs, int8_t rst, int8_t rd)
    : Adafruit_SPITFT(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, busWidth, d0, wr, dc,
                      cs, rst, rd) {}

// clang-format off
static const uint8_t PROGMEM initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};
// clang-format on

/**************************************************************************/
/*!
    @brief   Initialize ILI9341 chip
    Connects to the ILI9341 over SPI and sends initialization procedure commands
    @param    freq  Desired SPI clock frequency
*/
/**************************************************************************/
void Adafruit_ILI9341::begin(uint32_t freq) {

  if (!freq)
    freq = SPI_DEFAULT_FREQ;
  initSPI(freq);

  if (_rst < 0) {                 // If no hardware reset pin...
    sendCommand(ILI9341_SWRESET); // Engage software reset
    delay(150);
  }

  uint8_t cmd, x, numArgs;
  const uint8_t *addr = initcmd;
  while ((cmd = pgm_read_byte(addr++)) > 0) {
    x = pgm_read_byte(addr++);
    numArgs = x & 0x7F;
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;
    if (x & 0x80)
      delay(150);
  }

  _width = ILI9341_TFTWIDTH;
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
    _width = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;
    break;
  case 1:
    m = (MADCTL_MV | MADCTL_BGR);
    _width = ILI9341_TFTHEIGHT;
    _height = ILI9341_TFTWIDTH;
    break;
  case 2:
    m = (MADCTL_MY | MADCTL_BGR);
    _width = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;
    break;
  case 3:
    m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    _width = ILI9341_TFTHEIGHT;
    _height = ILI9341_TFTWIDTH;
    break;
  }

  sendCommand(ILI9341_MADCTL, &m, 1);
}

/**************************************************************************/
/*!
    @brief   Enable/Disable display color inversion
    @param   invert True to invert, False to have normal color
*/
/**************************************************************************/
void Adafruit_ILI9341::invertDisplay(bool invert) {
  sendCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
}

/**************************************************************************/
/*!
    @brief   Scroll display memory
    @param   y How many pixels to scroll display by
*/
/**************************************************************************/
void Adafruit_ILI9341::scrollTo(uint16_t y) {
  uint8_t data[2];
  data[0] = y >> 8;
  data[1] = y & 0xff;
  sendCommand(ILI9341_VSCRSADD, (uint8_t *)data, 2);
}

/**************************************************************************/
/*!
    @brief   Set the height of the Top and Bottom Scroll Margins
    @param   top The height of the Top scroll margin
    @param   bottom The height of the Bottom scroll margin
 */
/**************************************************************************/
void Adafruit_ILI9341::setScrollMargins(uint16_t top, uint16_t bottom) {
  // TFA+VSA+BFA must equal 320
  if (top + bottom <= ILI9341_TFTHEIGHT) {
    uint16_t middle = ILI9341_TFTHEIGHT - (top + bottom);
    uint8_t data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = middle >> 8;
    data[3] = middle & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;
    sendCommand(ILI9341_VSCRDEF, (uint8_t *)data, 6);
  }
}

/**************************************************************************/
/*!
    @brief   Set the "address window" - the rectangle we will write to RAM with
   the next chunk of      SPI data writes. The ILI9341 will automatically wrap
   the data as each row is filled
    @param   x1  TFT memory 'x' origin
    @param   y1  TFT memory 'y' origin
    @param   w   Width of rectangle
    @param   h   Height of rectangle
*/
/**************************************************************************/
void Adafruit_ILI9341::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w,
                                     uint16_t h) {
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
#if defined(ARDUINO_ARCH_RP2040)
  // Use optimised code if it is HARDWARE SPI
  if (connection == 0) {
    // Since the SPI functions do not terminate until transmission is complete
    // a busy check is not needed.
    // while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    SPI_DC_LOW();          // Command mode
    spi_set_format(spi0,  8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST);
    spi_get_hw(spi0)->dr = (uint32_t)ILI9341_CASET;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    SPI_DC_HIGH();         // Data mode
    spi_get_hw(spi0)->dr = (uint32_t)x1>>8;
    spi_get_hw(spi0)->dr = (uint32_t)x1;
    spi_get_hw(spi0)->dr = (uint32_t)x2>>8;
    spi_get_hw(spi0)->dr = (uint32_t)x2;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    SPI_DC_LOW();          // Command mode
    spi_get_hw(spi0)->dr = (uint32_t)ILI9341_PASET;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    SPI_DC_HIGH();         // Data mode
    spi_get_hw(spi0)->dr = (uint32_t)y1>>8;
    spi_get_hw(spi0)->dr = (uint32_t)y1;
    spi_get_hw(spi0)->dr = (uint32_t)y2>>8;
    spi_get_hw(spi0)->dr = (uint32_t)y2;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    SPI_DC_LOW();          // Command mode
    spi_get_hw(spi0)->dr = (uint32_t)ILI9341_RAMWR;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    spi_set_format(spi0, 16, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST);
    SPI_DC_HIGH();         // Data mode
  }
  else
#endif
  {
    writeCommand(ILI9341_CASET); // Column address set
    SPI_WRITE16(x1);
    SPI_WRITE16(x2);
    writeCommand(ILI9341_PASET); // Row address set
    SPI_WRITE16(y1);
    SPI_WRITE16(y2);
    writeCommand(ILI9341_RAMWR); // Write to RAM
  }
}

/**************************************************************************/
/*!
    @brief  Read 8 bits of data from ILI9341 configuration memory. NOT from RAM!
            This is highly undocumented/supported, it's really a hack but kinda
   works?
    @param    commandByte  The command register to read data from
    @param    index  The byte index into the command to read from
    @return   Unsigned 8-bit data read from ILI9341 register
 */
/**************************************************************************/
uint8_t Adafruit_ILI9341::readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t data = 0x10 + index;
  sendCommand(0xD9, &data, 1); // Set Index Register
  return Adafruit_SPITFT::readcommand8(commandByte);
}


// Override Adafruit_GFX writePixel() for RP2040
/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Not self-contained; should follow a startWrite() call.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
#if defined(ARDUINO_ARCH_RP2040)
void Adafruit_ILI9341::writePixel(int16_t x, int16_t y, uint16_t color) {

  // If it is a TFT_HARD_SPI interface then use RP2040 optimised code
  // otherwise use original code
  if (connection == 0) {
    if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
      // Since the SPI functions do not terminate until transmission is complete
      // a busy check is not needed.
      // while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_LOW();          // Command mode
      spi_set_format(spi0,  8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST);
      spi_get_hw(spi0)->dr = (uint32_t)ILI9341_CASET;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_HIGH();
      spi_get_hw(spi0)->dr = (uint32_t)x>>8;
      spi_get_hw(spi0)->dr = (uint32_t)x;
      spi_get_hw(spi0)->dr = (uint32_t)x>>8;
      spi_get_hw(spi0)->dr = (uint32_t)x;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_LOW();          // Command mode
      spi_get_hw(spi0)->dr = (uint32_t)ILI9341_PASET;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_HIGH();
      spi_get_hw(spi0)->dr = (uint32_t)y>>8;
      spi_get_hw(spi0)->dr = (uint32_t)y;
      spi_get_hw(spi0)->dr = (uint32_t)y>>8;
      spi_get_hw(spi0)->dr = (uint32_t)y;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_LOW();          // Command mode
      spi_get_hw(spi0)->dr = (uint32_t)ILI9341_RAMWR;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
      SPI_DC_HIGH();
      spi_get_hw(spi0)->dr = (uint32_t)color>>8;
      spi_get_hw(spi0)->dr = (uint32_t)color;

    /*
      // Subsequent pixel reads work OK without draining the FIFO...
      // Drain RX FIFO, then wait for shifting to finish (which may be *after*
      // TX FIFO drains), then drain RX FIFO again
      while (spi_is_readable(spi0))
          (void)spi_get_hw(spi0)->dr;
      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS)
          tight_loop_contents();
      while (spi_is_readable(spi0))
          (void)spi_get_hw(spi0)->dr;
    //*/

    //  Subsequent pixel reads work without this
    //  spi_get_hw(spi0)->icr = SPI_SSPICR_RORIC_BITS;

      while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};

      // Next call will start with 8 bit command so changing to 16 bit not needed here
      //spi_set_format(spi0,  16, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST);
    }
  }
  else {
    Adafruit_SPITFT::writePixel(x, y, color);
    return;
  }
}
#endif