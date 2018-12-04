/***************************************************
  This is our library for the Adafruit ST7789V Breakout and Shield
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

#include "WROVER_KIT_LCD.h"
#include "pins_arduino.h"

#define WROVER_CS               22
#define WROVER_DC               21
#define WROVER_RST              18
#define WROVER_SCLK             19
#define WROVER_MOSI             23
#define WROVER_MISO             25
#define WROVER_BL               5

#define WROVER_BL_OFF           HIGH
#define WROVER_BL_ON            LOW

#define WROVER_DC_HIGH()           GPIO.out_w1ts = (1 << WROVER_DC)
#define WROVER_DC_LOW()            GPIO.out_w1tc = (1 << WROVER_DC)
#define WROVER_CS_HIGH()           GPIO.out_w1ts = (1 << WROVER_CS)
#define WROVER_CS_LOW()            GPIO.out_w1tc = (1 << WROVER_CS)

#define WROVER_DEFAULT_FREQ        40000000
#define WROVER_MAX_PIXELS_AT_ONCE  32

#define WROVER_MADCTL_MY  0x80
#define WROVER_MADCTL_MX  0x40
#define WROVER_MADCTL_MV  0x20
#define WROVER_MADCTL_ML  0x10
#define WROVER_MADCTL_BGR 0x08
#define WROVER_MADCTL_MH  0x04

const uint8_t ili9341_init_data[] = {
    0xEF, 3, 0x03,0x80,0x02,
    0xCF, 3, 0x00,0XC1,0X30,
    0xED, 4, 0x64,0x03,0X12,0X81,
    0xE8, 3, 0x85,0x00,0x78,
    0xCB, 5, 0x39,0x2C,0x00,0x34,0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00,0x00,
    0xC0, 1, 0x23,
    0xC1, 1, 0x10,
    0xC5, 2, 0x3e,0x28,
    0xC7, 1, 0x86,
    0x36, 1, 0x48,
    0x3A, 1, 0x55,
    0xB1, 2, 0x00,0x18,
    0xB6, 3, 0x08,0x82,0x27,
    0xF2, 1, 0x00,
    0x26, 1, 0x01,
    0xE0, 15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,
    0xE1, 15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,
    0x00
};

const uint8_t st7789v_init_data[] = {
    0x36, 1, 0,
    0x3A, 1, 0x55,
    0xB2, 5, 0x0C,0x0C,0x00,0x33,0x33,
    0xB7, 1, 0x35,
    0xBB, 1, 0x2B,
    0xC0, 1, 0x2C,
    0xC2, 2, 0x01,0xFF,
    0xC3, 1, 0x11,
    0xC4, 1, 0x20,
    0xC6, 1, 0x0F,
    0xD0, 2, 0xA4,0xA1,
    0xE0, 14, 0xD0,0x00,0x05,0x0E,0x15,0x0D,0x37,0x43,0x47,0x09,0x15,0x12,0x16,0x19,
    0xE1, 14, 0xD0,0x00,0x05,0x0D,0x0C,0x06,0x2D,0x44,0x40,0x0E,0x1C,0x18,0x16,0x19,
    0x00
};

void  WROVER_KIT_LCD::writeInitData(const uint8_t * data){
    uint8_t cmd, len, i;
    while(true){
        cmd = *data++;
        if(!cmd){ //END
            return;
        }
        len = *data++;
        writeCommand(cmd);
        for(i=0;i<len;i++){
            SPI.write(*data++);
        }
    }
}

uint32_t WROVER_KIT_LCD::readId() {
    uint32_t freq = _freq;
    if(_freq > 24000000){
        _freq = 24000000;
    }
    startWrite();
    writeCommand(0xD9);
    SPI.write(0x10);
    writeCommand(0x04);
    uint32_t r = SPI.transfer(0) << 16;
    r |= SPI.transfer(0) << 8;
    r |= SPI.transfer(0);
    endWrite();
    _freq = freq;
    return r;
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t WROVER_KIT_LCD::color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

WROVER_KIT_LCD::WROVER_KIT_LCD() : Adafruit_GFX(WROVER_WIDTH, WROVER_HEIGHT) {
    _id   = 0;
    _freq = WROVER_DEFAULT_FREQ;
}

void WROVER_KIT_LCD::startWrite(void){
    SPI.beginTransaction(SPISettings(_freq, MSBFIRST, SPI_MODE0));
    WROVER_CS_LOW();
}

void WROVER_KIT_LCD::endWrite(void){
    WROVER_CS_HIGH();
    SPI.endTransaction();
}

void WROVER_KIT_LCD::writeCommand(uint8_t cmd){
    WROVER_DC_LOW();
    SPI.write(cmd);
    WROVER_DC_HIGH();
}

void WROVER_KIT_LCD::begin(){
    _width  = WROVER_WIDTH;
    _height = WROVER_HEIGHT;

    pinMode(WROVER_DC, OUTPUT);
    digitalWrite(WROVER_DC, LOW);
    pinMode(WROVER_CS, OUTPUT);
    digitalWrite(WROVER_CS, HIGH);
    pinMode(WROVER_BL, OUTPUT);
    digitalWrite(WROVER_BL, WROVER_BL_OFF);

    SPI.begin(WROVER_SCLK, WROVER_MISO, WROVER_MOSI, -1);

    pinMode(WROVER_RST, OUTPUT);
    digitalWrite(WROVER_RST, HIGH);
    delay(100);
    digitalWrite(WROVER_RST, LOW);
    delay(100);
    digitalWrite(WROVER_RST, HIGH);
    delay(200);

    _id = readId();
    log_d("Display ID: 0x%06X", _id);

    startWrite();

    if(_id){
        writeInitData(st7789v_init_data);
    } else {
        writeInitData(ili9341_init_data);
    }

    writeCommand(WROVER_SLPOUT);
    delay(120);
    writeCommand(WROVER_DISPON);
    delay(120);

    endWrite();
    digitalWrite(WROVER_BL, WROVER_BL_ON);
}

typedef struct {
        uint8_t madctl;
        uint8_t bmpctl;
        uint16_t width;
        uint16_t height;
} rotation_data_t;

const rotation_data_t ili9341_rotations[4] = {
    {(WROVER_MADCTL_MX|WROVER_MADCTL_BGR),(WROVER_MADCTL_MX|WROVER_MADCTL_MY|WROVER_MADCTL_BGR),WROVER_WIDTH,WROVER_HEIGHT},
    {(WROVER_MADCTL_MV|WROVER_MADCTL_BGR),(WROVER_MADCTL_MV|WROVER_MADCTL_MX|WROVER_MADCTL_BGR),WROVER_HEIGHT,WROVER_WIDTH},
    {(WROVER_MADCTL_MY|WROVER_MADCTL_BGR),(WROVER_MADCTL_BGR),WROVER_WIDTH,WROVER_HEIGHT},
    {(WROVER_MADCTL_MX|WROVER_MADCTL_MY|WROVER_MADCTL_MV|WROVER_MADCTL_BGR),(WROVER_MADCTL_MY|WROVER_MADCTL_MV|WROVER_MADCTL_BGR),WROVER_HEIGHT,WROVER_WIDTH}
};

const rotation_data_t st7789v_rotations[4] = {
    {0,WROVER_MADCTL_MY,WROVER_WIDTH,WROVER_HEIGHT},
    {(WROVER_MADCTL_MV|WROVER_MADCTL_MX),WROVER_MADCTL_MV,WROVER_HEIGHT,WROVER_WIDTH},
    {(WROVER_MADCTL_MY|WROVER_MADCTL_MX),WROVER_MADCTL_MX,WROVER_WIDTH,WROVER_HEIGHT},
    {(WROVER_MADCTL_MY|WROVER_MADCTL_MV),(WROVER_MADCTL_MX|WROVER_MADCTL_MY|WROVER_MADCTL_MV),WROVER_HEIGHT,WROVER_WIDTH}
};

void WROVER_KIT_LCD::setRotation(uint8_t m) {
    rotation = m % 4; // can't be higher than 3

    if(_id){
        m = st7789v_rotations[rotation].madctl;
        _width  = st7789v_rotations[rotation].width;
        _height = st7789v_rotations[rotation].height;
    } else {
        m = ili9341_rotations[rotation].madctl;
        _width  = ili9341_rotations[rotation].width;
        _height = ili9341_rotations[rotation].height;
    }

    startWrite();
    writeCommand(WROVER_MADCTL);
    SPI.write(m);
    endWrite();
}

void WROVER_KIT_LCD::invertDisplay(boolean i) {
    startWrite();
    writeCommand(i ? WROVER_INVON : WROVER_INVOFF);
    endWrite();
}

void WROVER_KIT_LCD::scrollTo(uint16_t y) {
    startWrite();
    writeCommand(WROVER_VSCRSADD);
    SPI.write16(y);
    endWrite();
}

void WROVER_KIT_LCD::setupScrollArea(uint16_t tfa, uint16_t bfa) {
  startWrite();
  writeCommand(WROVER_VSCRDEF); // Vertical scroll definition
  SPI.write(tfa >> 8);
  SPI.write(tfa);
  SPI.write((WROVER_HEIGHT - tfa - bfa) >> 8);
  SPI.write(WROVER_HEIGHT - tfa - bfa);
  SPI.write(bfa >> 8);
  SPI.write(bfa);
  endWrite();
}

/*
 * Transaction API
 * */

void WROVER_KIT_LCD::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
    uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
    writeCommand(WROVER_CASET);
    SPI.write32(xa);
    writeCommand(WROVER_RASET);
    SPI.write32(ya);
    writeCommand(WROVER_RAMWR);
}

void WROVER_KIT_LCD::startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    startWrite();
    writeCommand(WROVER_MADCTL);
    if(_id){
        SPI.write(st7789v_rotations[rotation].bmpctl);
    } else {
        SPI.write(ili9341_rotations[rotation].bmpctl);
    }
    endWrite();
    startWrite();
    setAddrWindow(x, _height - y - h, w, h);
}

void WROVER_KIT_LCD::endBitmap() {
    endWrite();
    startWrite();
    writeCommand(WROVER_MADCTL);
    if(_id){
        SPI.write(st7789v_rotations[rotation].madctl);
    } else {
        SPI.write(ili9341_rotations[rotation].madctl);
    }
    endWrite();
}

void WROVER_KIT_LCD::pushColor(uint16_t color) {
    startWrite();
    SPI.write16(color);
    endWrite();
}

void WROVER_KIT_LCD::writePixel(uint16_t color){
    SPI.write16(color);
}

void WROVER_KIT_LCD::writePixels(uint16_t * colors, uint32_t len){
    SPI.writePixels((uint8_t*)colors , len * 2);
}

void WROVER_KIT_LCD::writeColor(uint16_t color, uint32_t len){
    static uint16_t temp[WROVER_MAX_PIXELS_AT_ONCE];
    size_t blen = (len > WROVER_MAX_PIXELS_AT_ONCE)?WROVER_MAX_PIXELS_AT_ONCE:len;
    uint16_t tlen = 0;

    for (uint32_t t=0; t<blen; t++){
        temp[t] = color;
    }

    while(len){
        tlen = (len>blen)?blen:len;
        writePixels(temp, tlen);
        len -= tlen;
    }
}

void WROVER_KIT_LCD::writePixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    setAddrWindow(x,y,1,1);
    writePixel(color);
}

void WROVER_KIT_LCD::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    if((x >= _width) || (y >= _height)) return;

    int16_t x2 = x + w - 1, y2 = y + h - 1;
    if((x2 < 0) || (y2 < 0)) return;

    // Clip left/top
    if(x < 0) {
        x = 0;
        w = x2 + 1;
    }
    if(y < 0) {
        y = 0;
        h = y2 + 1;
    }

    // Clip right/bottom
    if(x2 >= _width)  w = _width  - x;
    if(y2 >= _height) h = _height - y;

    int32_t len = (int32_t)w * h;
    setAddrWindow(x, y, w, h);
    writeColor(color, len);
}

void WROVER_KIT_LCD::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
    writeFillRect(x, y, 1, h, color);
}

void WROVER_KIT_LCD::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){
    writeFillRect(x, y, w, 1, color);
}

uint8_t WROVER_KIT_LCD::readcommand8(uint8_t c, uint8_t index) {
    uint32_t freq = _freq;
    if(_freq > 24000000){
        _freq = 24000000;
    }
    startWrite();
    writeCommand(0xD9);  // woo sekret command?
    SPI.write(0x10 + index);
    writeCommand(c);
    uint8_t r = SPI.transfer(0);
    endWrite();
    _freq = freq;
    return r;
}

void WROVER_KIT_LCD::drawPixel(int16_t x, int16_t y, uint16_t color){
    startWrite();
    writePixel(x, y, color);
    endWrite();
}

void WROVER_KIT_LCD::drawFastVLine(int16_t x, int16_t y,
        int16_t h, uint16_t color) {
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}

void WROVER_KIT_LCD::drawFastHLine(int16_t x, int16_t y,
        int16_t w, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}

void WROVER_KIT_LCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {
    startWrite();
    writeFillRect(x,y,w,h,color);
    endWrite();
}

// This code was ported/adapted from https://github.com/PaulStoffregen/ILI9341_t3
void WROVER_KIT_LCD::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors) {
    startWrite();
    setAddrWindow(x, y, w, h);
    writePixels((uint16_t*) pcolors, w*h);
    endWrite();
}


/*
 * Bitmaps and JPEGs
 * */


/*
 * BMP
 * */

#define bmpRead32(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8 | (uint32_t)(d[(o)+2]) << 16 | (uint32_t)(d[(o)+3]) << 24)
#define bmpRead16(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8)

#define bmpColor8(c)       (((uint16_t)(((uint8_t*)(c))[0] & 0xE0) << 8) | ((uint16_t)(((uint8_t*)(c))[0] & 0x1C) << 6) | ((((uint8_t*)(c))[0] & 0x3) << 3))
#define bmpColor16(c)      ((((uint8_t*)(c))[0] | ((uint16_t)((uint8_t*)(c))[1]) << 8))
#define bmpColor24(c)      (((uint16_t)(((uint8_t*)(c))[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[0] & 0xF8) >> 3))
#define bmpColor32(c)      (((uint16_t)(((uint8_t*)(c))[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[2] & 0xFC) << 3) | ((((uint8_t*)(c))[1] & 0xF8) >> 3))

void WROVER_KIT_LCD::bmpSkipPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
    size_t bytesToSkip = (len * bitsPerPixel) / 8;
    file.seek(bytesToSkip, SeekCur);
}

void WROVER_KIT_LCD::bmpAddPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
    size_t bytesPerTransaction = bitsPerPixel * 4;
    uint8_t transBuf[bytesPerTransaction];
    uint16_t pixBuf[32];

    uint8_t * tBuf;
    uint8_t pixIndex = 0;
    size_t wIndex = 0, pixNow, bytesNow;

    while(wIndex < len){
        pixNow = len - wIndex;
        if(pixNow > 32){
            pixNow = 32;
        }
        bytesNow = (pixNow * bitsPerPixel) / 8;
        file.read(transBuf, bytesNow);
        tBuf = transBuf;

        for(pixIndex=0; pixIndex < pixNow; pixIndex++){
            if(bitsPerPixel == 32){
                pixBuf[pixIndex] = (bmpColor32(tBuf));
                tBuf+=4;
            } else if(bitsPerPixel == 24){
                pixBuf[pixIndex] = (bmpColor24(tBuf));
                tBuf+=3;
            } else if(bitsPerPixel == 16){
                pixBuf[pixIndex] = (bmpColor16(tBuf));
                tBuf+=2;
            } else if(bitsPerPixel == 8){
                pixBuf[pixIndex] = (bmpColor8(tBuf));
                tBuf+=1;
            } else if(bitsPerPixel == 4){
                uint16_t g = tBuf[0] & 0xF;
                if(pixIndex & 1){
                    tBuf+=1;
                } else {
                    g = tBuf[0] >> 4;
                }
                pixBuf[pixIndex] = ((g << 12) | (g << 7) | (g << 1));
            }
        }
        writePixels(pixBuf, pixNow);
        wIndex += pixNow;
    }
}

void WROVER_KIT_LCD::drawBmpFile(fs::FS &fs, const char * path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY){
    if((x + maxWidth) > width() || (y + maxHeight) > height()){
        log_e("Bad dimensions given");
        return;
    }

    if(!maxWidth){
        maxWidth = width() - x;
    }
    if(!maxHeight){
        maxHeight = height() - y;
    }

    File file = fs.open(path);
    if(!file){
        log_e("Failed to open file for reading");
        return;
    }
    size_t headerLen = 0x22;
    size_t fileSize = file.size();
    uint8_t headerBuf[headerLen];
    if(fileSize < headerLen || file.read(headerBuf, headerLen) < headerLen){
        log_e("Failed to read the file's header");
        file.close();
        return;
    }

    if(headerBuf[0] != 'B' || headerBuf[1] != 'M'){
        log_e("Wrong file format");
        file.close();
        return;
    }

    //size_t bmpSize = bmpRead32(headerBuf, 0x02);
    uint32_t dataOffset = bmpRead32(headerBuf, 0x0A);
    int32_t bmpWidthI = bmpRead32(headerBuf, 0x12);
    int32_t bmpHeightI = bmpRead32(headerBuf, 0x16);
    uint16_t bitsPerPixel = bmpRead16(headerBuf, 0x1C);

    size_t bmpWidth = abs(bmpWidthI);
    size_t bmpHeight = abs(bmpHeightI);

    if(offX >= bmpWidth || offY >= bmpHeight){
        log_e("Offset Outside of bitmap size");
        file.close();
        return;
    }

    size_t bmpMaxWidth = bmpWidth - offX;
    size_t bmpMaxHeight = bmpHeight - offY;
    size_t outWidth = (bmpMaxWidth > maxWidth)?maxWidth:bmpMaxWidth;
    size_t outHeight = (bmpMaxHeight > maxHeight)?maxHeight:bmpMaxHeight;
    size_t ovfWidth = bmpMaxWidth - outWidth;
    size_t ovfHeight = bmpMaxHeight - outHeight;

    file.seek(dataOffset);
    startBitmap(x, y, outWidth, outHeight);

    if(ovfHeight){
        bmpSkipPixels(file, bitsPerPixel, ovfHeight * bmpWidth);
    }
    if(!offX && !ovfWidth){
        bmpAddPixels(file, bitsPerPixel, outWidth * outHeight);
    } else {
        size_t ih;
        for(ih=0;ih<outHeight;ih++){
            if(offX){
                bmpSkipPixels(file, bitsPerPixel, offX);
            }
            bmpAddPixels(file, bitsPerPixel, outWidth);
            if(ovfWidth){
                bmpSkipPixels(file, bitsPerPixel, ovfWidth);
            }
        }
    }

    endBitmap();
    file.close();
}

/*
 * JPEG
 * */

#include "rom/tjpgd.h"

#define jpgColor(c) (((uint16_t)(((uint8_t*)(c))[0] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[2] & 0xF8) >> 3))

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
const char * jd_errors[] = {
    "Succeeded",
    "Interrupted by output function",
    "Device error or wrong termination of input stream",
    "Insufficient memory pool for the image",
    "Insufficient stream input buffer",
    "Parameter error",
    "Data format error",
    "Right format but not supported",
    "Not supported JPEG standard"
};
#endif

typedef struct {
        uint16_t x;
        uint16_t y;
        uint16_t maxWidth;
        uint16_t maxHeight;
        uint16_t offX;
        uint16_t offY;
        jpeg_div_t scale;
        const void * src;
        size_t len;
        size_t index;
        WROVER_KIT_LCD * tft;
        uint16_t outWidth;
        uint16_t outHeight;
} jpg_file_decoder_t;

static uint32_t jpgReadFile(JDEC *decoder, uint8_t *buf, uint32_t len){
    jpg_file_decoder_t * jpeg = (jpg_file_decoder_t *)decoder->device;
    File * file = (File *)jpeg->src;
    if(buf){
        return file->read(buf, len);
    } else {
        file->seek(len, SeekCur);
    }
    return len;
}

static uint32_t jpgRead(JDEC *decoder, uint8_t *buf, uint32_t len){
    jpg_file_decoder_t * jpeg = (jpg_file_decoder_t *)decoder->device;
    if(buf){
        memcpy(buf, (const uint8_t *)jpeg->src + jpeg->index, len);
    }
    jpeg->index += len;
    return len;
}

static uint32_t jpgWrite(JDEC *decoder, void *bitmap, JRECT *rect){
    jpg_file_decoder_t * jpeg = (jpg_file_decoder_t *)decoder->device;
    uint16_t x = rect->left;
    uint16_t y = rect->top;
    uint16_t w = rect->right + 1 - x;
    uint16_t h = rect->bottom + 1 - y;
    uint16_t oL = 0, oR = 0;
    uint8_t *data = (uint8_t *)bitmap;

    if(rect->right < jpeg->offX){
        return 1;
    }
    if(rect->left >= (jpeg->offX + jpeg->outWidth)){
        return 1;
    }
    if(rect->bottom < jpeg->offY){
        return 1;
    }
    if(rect->top >= (jpeg->offY + jpeg->outHeight)){
        return 1;
    }
    if(rect->top < jpeg->offY){
        uint16_t linesToSkip = jpeg->offY - rect->top;
        data += linesToSkip * w * 3;
        h -= linesToSkip;
        y += linesToSkip;
    }
    if(rect->bottom >= (jpeg->offY + jpeg->outHeight)){
        uint16_t linesToSkip = (rect->bottom + 1) - (jpeg->offY + jpeg->outHeight);
        h -= linesToSkip;
    }
    if(rect->left < jpeg->offX){
        oL = jpeg->offX - rect->left;
    }
    if(rect->right >= (jpeg->offX + jpeg->outWidth)){
        oR = (rect->right + 1) - (jpeg->offX + jpeg->outWidth);
    }

    uint16_t pixBuf[32];
    uint8_t pixIndex = 0;
    uint16_t line;

    jpeg->tft->startWrite();
    jpeg->tft->setAddrWindow(x - jpeg->offX + jpeg->x + oL, y - jpeg->offY + jpeg->y, w - (oL + oR), h);

    while(h--){
        data += 3 * oL;
        line = w - (oL + oR);
        while(line--){
            pixBuf[pixIndex++] = jpgColor(data);
            data += 3;
            if(pixIndex == 32){
                jpeg->tft->writePixels(pixBuf, 32);
                pixIndex = 0;
            }
        }
        data += 3 * oR;
    }
    if(pixIndex){
        jpeg->tft->writePixels(pixBuf, pixIndex);
    }
    jpeg->tft->endWrite();
    return 1;
}

static bool jpgDecode(jpg_file_decoder_t * jpeg, uint32_t(* reader)(JDEC*,uint8_t *, uint32_t)){
    static uint8_t work[3100];
    JDEC decoder;

    JRESULT jres = jd_prepare(&decoder, reader, work, 3100, jpeg);
    if(jres != JDR_OK){
        log_e("jd_prepare failed! %s", jd_errors[jres]);
        return false;
    }

    uint16_t jpgWidth = decoder.width / (1 << (uint8_t)(jpeg->scale));
    uint16_t jpgHeight = decoder.height / (1 << (uint8_t)(jpeg->scale));

    if(jpeg->offX >= jpgWidth || jpeg->offY >= jpgHeight){
        log_e("Offset Outside of JPEG size");
        return false;
    }

    size_t jpgMaxWidth = jpgWidth - jpeg->offX;
    size_t jpgMaxHeight = jpgHeight - jpeg->offY;

    jpeg->outWidth = (jpgMaxWidth > jpeg->maxWidth)?jpeg->maxWidth:jpgMaxWidth;
    jpeg->outHeight = (jpgMaxHeight > jpeg->maxHeight)?jpeg->maxHeight:jpgMaxHeight;

    jres = jd_decomp(&decoder, jpgWrite, (uint8_t)jpeg->scale);
    if(jres != JDR_OK){
        log_e("jd_decomp failed! %s", jd_errors[jres]);
        return false;
    }

    return true;
}

void WROVER_KIT_LCD::drawJpg(const uint8_t * jpg_data, size_t jpg_len, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale){
    if((x + maxWidth) > width() || (y + maxHeight) > height()){
        log_e("Bad dimensions given");
        return;
    }

    jpg_file_decoder_t jpeg;

    if(!maxWidth){
        maxWidth = width() - x;
    }
    if(!maxHeight){
        maxHeight = height() - y;
    }

    jpeg.src = jpg_data;
    jpeg.len = jpg_len;
    jpeg.index = 0;
    jpeg.x = x;
    jpeg.y = y;
    jpeg.maxWidth = maxWidth;
    jpeg.maxHeight = maxHeight;
    jpeg.offX = offX;
    jpeg.offY = offY;
    jpeg.scale = scale;
    jpeg.tft = this;

    jpgDecode(&jpeg, jpgRead);
}

void WROVER_KIT_LCD::drawJpgFile(fs::FS &fs, const char * path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale){
    if((x + maxWidth) > width() || (y + maxHeight) > height()){
        log_e("Bad dimensions given");
        return;
    }

    File file = fs.open(path);
    if(!file){
        log_e("Failed to open file for reading");
        return;
    }

    jpg_file_decoder_t jpeg;

    if(!maxWidth){
        maxWidth = width() - x;
    }
    if(!maxHeight){
        maxHeight = height() - y;
    }

    jpeg.src = &file;
    jpeg.len = file.size();
    jpeg.index = 0;
    jpeg.x = x;
    jpeg.y = y;
    jpeg.maxWidth = maxWidth;
    jpeg.maxHeight = maxHeight;
    jpeg.offX = offX;
    jpeg.offY = offY;
    jpeg.scale = scale;
    jpeg.tft = this;

    jpgDecode(&jpeg, jpgReadFile);

    file.close();
}
