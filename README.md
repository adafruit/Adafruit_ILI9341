# Adafruit ILI9341 Arduino Library [![Build Status](https://github.com/adafruit/Adafruit_ILI9341/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_ILI9341/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_ILI9341/html/index.html)

## Description
This is a library to interface with `Adafruit ILI9341` display products via SPI.

## Supported Devices
This library works with
 - the [Adafruit 2.8" Touch Shield V2 (SPI)](http://www.adafruit.com/products/1651)
 - the [Adafruit 2.4" TFT LCD with Touchscreen Breakout w/MicroSD Socket - ILI9341](https://www.adafruit.com/product/2478)
 - the [2.8" TFT LCD with Touchscreen Breakout Board w/MicroSD Socket - ILI9341](https://www.adafruit.com/product/1770)
 - the [2.2" 18-bit color TFT LCD display with microSD card breakout - ILI9340](https://www.adafruit.com/product/1480)
 - the [2.4" TFT FeatherWing 320x240 Touchscreen](https://www.adafruit.com/product/3315) (for all feathers)

Check out the links above for our tutorials and wiring diagrams.
These displays use SPI to communicate, therefore at least 4 pins are required to interface with them (the 5th pin named `RST` is optional).

---

## Install
1. Click the DOWNLOADS button in the top right corner
2. Rename the uncompressed folder `Adafruit_ILI9341`
3. Check that the `Adafruit_ILI9341` folder contains `Adafruit_ILI9341.cpp` and `Adafruit_ILI9341`
4. Place the `Adafruit_ILI9341` library folder in your `arduino_sketch_folder/libraries/` folder.<br>Note: You may need to create the libraries subfolder if its your first library.
5. Restart the Arduino IDE.

This library **depends on** the [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) library for Arduino and requires it to be installed as well.


## Contribute
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

## License
Written by Limor Fried / Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution.

## Related Adafruit Libraries
- [Adafruit_ImageReader library](https://github.com/adafruit/Adafruit_ImageReader) for image-loading
