# MicroPython ILI9341 SPI display driver
# Adapted from rdagger/micropython-ili9341 and other sources

import machine
import time
import ustruct

# Commands
ILI9341_NOP = 0x00
ILI9341_SWRESET = 0x01
ILI9341_RDDID = 0x04
ILI9341_RDDST = 0x09

ILI9341_SLPIN = 0x10
ILI9341_SLPOUT = 0x11
ILI9341_PTLON = 0x12
ILI9341_NORON = 0x13

ILI9341_RDMODE = 0x0A
ILI9341_RDMADCTL = 0x0B
ILI9341_RDPIXFMT = 0x0C
ILI9341_RDIMGFMT = 0x0D
ILI9341_RDSELFDIAG = 0x0F

ILI9341_INVOFF = 0x20
ILI9341_INVON = 0x21
ILI9341_GAMMASET = 0x26
ILI9341_DISPOFF = 0x28
ILI9341_DISPON = 0x29

ILI9341_CASET = 0x2A
ILI9341_PASET = 0x2B
ILI9341_RAMWR = 0x2C
ILI9341_RAMRD = 0x2E

ILI9341_PTLAR = 0x30
ILI9341_MADCTL = 0x36
ILI9341_PIXFMT = 0x3A

ILI9341_FRMCTR1 = 0xB1
ILI9341_FRMCTR2 = 0xB2
ILI9341_FRMCTR3 = 0xB3
ILI9341_INVCTR = 0xB4
ILI9341_DFUNCTR = 0xB6

ILI9341_PWCTR1 = 0xC0
ILI9341_PWCTR2 = 0xC1
ILI9341_PWCTR3 = 0xC2
ILI9341_PWCTR4 = 0xC3
ILI9341_PWCTR5 = 0xC4
ILI9341_VMCTR1 = 0xC5
ILI9341_VMCTR2 = 0xC7

ILI9341_RDID1 = 0xDA
ILI9341_RDID2 = 0xDB
ILI9341_RDID3 = 0xDC
ILI9341_RDID4 = 0xDD

ILI9341_GMCTRP1 = 0xE0
ILI9341_GMCTRN1 = 0xE1

# MADCTL Bits
ILI9341_MADCTL_MY = 0x80  # Row address order
ILI9341_MADCTL_MX = 0x40  # Column address order
ILI9341_MADCTL_MV = 0x20  # Row/Column exchange
ILI9341_MADCTL_ML = 0x10  # Vertical refresh order
ILI9341_MADCTL_BGR = 0x08 # BGR-RGB order
ILI9341_MADCTL_MH = 0x04  # Horizontal refresh order

# Screen dimensions (update if your display is different)
ILI9341_TFTWIDTH = 240
ILI9341_TFTHEIGHT = 320

# Colors (RGB565)
BLACK = 0x0000
BLUE = 0x001F
RED = 0xF800
GREEN = 0x07E0
CYAN = 0x07FF
MAGENTA = 0xF81F
YELLOW = 0xFFE0
WHITE = 0xFFFF


def color565(r, g, b):
    """Convert RGB888 to RGB565"""
    return (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3


class ILI9341:
    def __init__(self, spi, cs, dc, rst, width=ILI9341_TFTWIDTH, height=ILI9341_TFTHEIGHT, rotation=0):
        self.spi = spi
        self.cs = cs
        self.dc = dc
        self.rst = rst
        self.width = width
        self.height = height

        self.cs.init(self.cs.OUT, value=1)
        self.dc.init(self.dc.OUT, value=0)
        if self.rst:
            self.rst.init(self.rst.OUT, value=0)

        self.reset()
        self._init_commands()
        self.set_rotation(rotation) # Default rotation
        self.fill(BLACK) # Clear screen

    def _write_cmd(self, cmd):
        self.cs.value(0)
        self.dc.value(0)
        self.spi.write(bytearray([cmd]))
        self.cs.value(1)

    def _write_data(self, data):
        self.cs.value(0)
        self.dc.value(1)
        self.spi.write(data)
        self.cs.value(1)

    def reset(self):
        if self.rst:
            self.rst.value(1)
            time.sleep_ms(5)
            self.rst.value(0)
            time.sleep_ms(20)
            self.rst.value(1)
            time.sleep_ms(150)
        else: # Software reset if no RST pin
            self._write_cmd(ILI9341_SWRESET)
            time.sleep_ms(150)


    def _init_commands(self):
        self._write_cmd(ILI9341_DISPOFF) # Display OFF

        self._write_cmd(ILI9341_PWCTR1) # Power Control 1
        self._write_data(b'\x23') # VRH[5:0] = 4.6V

        self._write_cmd(ILI9341_PWCTR2) # Power Control 2
        self._write_data(b'\x10') # SAP[2:0];BT[3:0] = VCIX2; -5.7V; +1.8V

        self._write_cmd(ILI9341_VMCTR1) # VCOM Control 1
        self._write_data(b'\x3e\x28') # VMH[6:0] = 4.25V, VML[6:0] = -1.5V

        self._write_cmd(ILI9341_VMCTR2) # VCOM Control 2
        self._write_data(b'\x86') # --

        self._write_cmd(ILI9341_MADCTL) # Memory Access Control
        self._write_data(b'\x48') # MX, BGR

        self._write_cmd(ILI9341_PIXFMT) # Pixel Format Set
        self._write_data(b'\x55') # 16 bits/pixel

        self._write_cmd(ILI9341_FRMCTR1) # Frame Rate Control (In Normal Mode/Full Colors)
        self._write_data(b'\x00\x18') # 0x00, 0x1B = 70Hz

        self._write_cmd(ILI9341_DFUNCTR) # Display Function Control
        self._write_data(b'\x08\x82\x27') # AGND, GS, SS, SM, ISC[3:0]

        # Gamma settings
        self._write_cmd(ILI9341_GAMMASET) # Gamma curve selected
        self._write_data(b'\x01')

        self._write_cmd(ILI9341_GMCTRP1) # Set Gamma
        self._write_data(b'\x0F\x31\x2B\x0C\x0E\x08\x4E\xF1\x37\x07\x10\x03\x0E\x09\x00')

        self._write_cmd(ILI9341_GMCTRN1) # Set Gamma
        self._write_data(b'\x00\x0E\x14\x03\x11\x07\x31\xC1\x48\x08\x0F\x0C\x31\x36\x0F')

        self._write_cmd(ILI9341_SLPOUT) # Sleep Out
        time.sleep_ms(120)
        self._write_cmd(ILI9341_DISPON) # Display ON
        time.sleep_ms(50)


    def set_rotation(self, r):
        self.rotation = r % 4
        self._write_cmd(ILI9341_MADCTL)
        if self.rotation == 0: # Portrait
            self._write_data(bytes([ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR]))
            self.width = ILI9341_TFTWIDTH
            self.height = ILI9341_TFTHEIGHT
        elif self.rotation == 1: # Landscape
            self._write_data(bytes([ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR]))
            self.width = ILI9341_TFTHEIGHT
            self.height = ILI9341_TFTWIDTH
        elif self.rotation == 2: # Portrait Inverted
            self._write_data(bytes([ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR]))
            self.width = ILI9341_TFTWIDTH
            self.height = ILI9341_TFTHEIGHT
        elif self.rotation == 3: # Landscape Inverted
            self._write_data(bytes([ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR]))
            self.width = ILI9341_TFTHEIGHT
            self.height = ILI9341_TFTWIDTH
        self._COLUMN_SET = ILI9341_CASET
        self._PAGE_SET = ILI9341_PASET
        self._RAM_WRITE = ILI9341_RAMWR
        self._RAM_READ = ILI9341_RAMRD

    def _set_window(self, x0, y0, x1, y1):
        self._write_cmd(self._COLUMN_SET)
        self._write_data(ustruct.pack(">HH", x0, x1))
        self._write_cmd(self._PAGE_SET)
        self._write_data(ustruct.pack(">HH", y0, y1))
        self._write_cmd(self._RAM_WRITE)

    def pixel(self, x, y, color):
        if not (0 <= x < self.width and 0 <= y < self.height):
            return
        self._set_window(x, y, x, y)
        self._write_data(ustruct.pack(">H", color))

    def fill_rectangle(self, x, y, w, h, color):
        x = min(self.width - 1, max(0, x))
        y = min(self.height - 1, max(0, y))
        w = min(self.width - x, max(1, w))
        h = min(self.height - y, max(1, h))

        self._set_window(x, y, x + w - 1, y + h - 1)

        # Prepare buffer with color
        chunk_size = 512 # Max SPI buffer size or reasonable chunk
        pixels_per_chunk = chunk_size // 2
        num_pixels = w * h

        buf = ustruct.pack(">H", color) * pixels_per_chunk

        self.cs.value(0)
        self.dc.value(1) # Data mode

        for _ in range(num_pixels // pixels_per_chunk):
            self.spi.write(buf)

        remaining_pixels = num_pixels % pixels_per_chunk
        if remaining_pixels > 0:
            self.spi.write(ustruct.pack(">H", color) * remaining_pixels)

        self.cs.value(1)


    def fill(self, color):
        self.fill_rectangle(0, 0, self.width, self.height, color)

    def hline(self, x, y, w, color):
        self.fill_rectangle(x, y, w, 1, color)

    def vline(self, x, y, h, color):
        self.fill_rectangle(x, y, 1, h, color)

    # Basic text drawing (using a simple built-in font or placeholder)
    # For more advanced text, a separate font library (like writer.py) is recommended.
    # This is a very rudimentary text function.
    def text(self, x, y, s, color, font=None, size=1, background=None):
        """
        Draws text. This is a placeholder for more sophisticated text rendering.
        It's recommended to use a proper font rendering library (e.g. writer.py).
        """
        # This is where you'd integrate with a font library.
        # For now, we'll just print a message indicating it's a placeholder
        # or draw very simple pixels if no advanced font lib is used.

        # If a background color is provided, fill the text area
        if background is not None:
            # Crude estimation of text bounds, assumes 8x8 font
            text_width = len(s) * 8 * size
            text_height = 8 * size
            self.fill_rectangle(x, y, text_width, text_height, background)

        # Actual text drawing would go here.
        # Example with a hypothetical 5x7 font if one was embedded:
        # for char_idx, char_code in enumerate(s):
        #   draw_char(x + char_idx * (5*size + spacing), y, char_code, color, size)

        # As a simple placeholder, draw a small rectangle where text would be
        # self.fill_rectangle(x, y, len(s) * 5, 7, color)
        print(f"ILI9341.text placeholder: '{s}' at ({x},{y}) color {color}, size {size}")
        # If using with writer.py, the writer object would handle this.


# Example of how to use with a font writer (e.g. Peter Hinch's writer.py)
# You would typically pass the ILI9341 instance to the Writer class.
# from writer import Writer
# import my_font  # Assuming my_font.py is generated by font-to-py
#
# class TFT_Screen(Writer):
#     def __init__(self, display, font):
#         super().__init__(display, font)
#         self.display = display # Keep a reference if needed for other drawing
#
# Then, in your main code:
# display = ILI9341(spi, cs, dc, rst)
# screen_writer = TFT_Screen(display, my_font)
# screen_writer.set_textpos(display, row, col) # display is passed for pixel method
# screen_writer.printstring("Hello World!")

# For direct use without Writer, one might implement a simple char drawer here
# based on a fixed font if available.
# The example project used adafruit_display_text.label which is more advanced.
# We will use Peter Hinch's Writer class for text.
