# writer.py Implements the Writer class.
# V0.3.0 Peter Hinch 2017-2021
# Released under the MIT license.

# A Writer is a class for rendering text to a display. The characters are
# rendered as a series of pixel values (integers). The Writer instance provides
# a virtual display which maintains the current screen state. This is written
# to the actual display when the refresh() method is called.

# A font is a Python bytearray encoding the character pixels.
# Fonts are created by the font_to_py.py utility.

# Methods:
# clear_screen()
# printstring(s)
# height() Font height in pixels
# stringlen(s) Width of a string in pixels.

# For a fixed pitch font:
# chars_per_row()
# charpos(row, col) Returns x,y co-ords for a given row and column.

# For a variable pitch font:
# set_textpos(x, y) Sets the x,y position for the next char.

# Demo of variable pitch font:
# import font6
# from writer import Writer
# from ssd1306_setup import WIDTH, HEIGHT, setup
# ssd = setup()
# Writer.set_clip(True, True, False)  # Clip verbose
# wri = Writer(ssd, font6)
# wri.set_textpos(ssd, 0, 0)  # In case previous tests have constrained it
# wri.printstring('Sunday\n')
# wri.printstring('12/2/17\n')
# wri.printstring('12.30pm')
# ssd.show()

# TODO Add sensible defaults to set_textpos like CWriter does.

_TWO_BYTE_INDEX = const(0)  # Font has two byte index for char codes
_ONE_BYTE_INDEX = const(1)
_MAX_CHARS = const(2)  # Maximum number of chars in font
_HEIGHT = const(3)
_WIDTH = const(4)  # Maximum character width
_MISSING = const(5)  # Code of character to substitute for missing chars.
# _FIRST_CHAR = const(6) Unused
_LAST_CHAR = const(7)

# Verbose messages when fonts are clipped
_VERBOSE = True
_NO_VERBOSE = False


def _get_char_addr(font, letter):
    index = font[_MAX_CHARS]
    if font[index] == _TWO_BYTE_INDEX:
        index += 1
        if letter >= font[_LAST_CHAR] or letter < font[_MISSING + 2]: # font[_FIRST_CHAR]:
            letter = font[_MISSING]  # Character is missing
        letter -= font[_MISSING + 2] # font[_FIRST_CHAR]
        idx = index + letter * 2
        offset = font[idx] << 8 | font[idx + 1]
    else: # Single byte index (ASCII only)
        index += 1
        if letter >= font[_LAST_CHAR] or letter < font[_MISSING + 2]: # font[_FIRST_CHAR]:
            letter = font[_MISSING]  # Character is missing
        letter -= font[_MISSING + 2] # font[_FIRST_CHAR]
        idx = index + letter
        offset = font[idx]
    # Skip the width byte
    return offset + index + 1 + (font[_LAST_CHAR] - font[_MISSING + 2]) * (font[font[_MAX_CHARS]] == _TWO_BYTE_INDEX)


class Writer():
    # Default scroll delay (ms)
    # dscroll = 100  # Not currently implemented

    # Print a single character at the current position and wrap.
    # Does not update display.
    # This is the essential drawing routine which needs to be overridden for
    # particular display types e.g. by Writer.draw_char_points for line drawing
    # displays such as the PyPortal. It is also overridden by the CWriter
    # and FastWriter classes.
    @staticmethod
    def draw_char(char_code, x, y, _display, _font, _color, _bgcolor, _landscape, _reverse):
        fnt = _font
        height = fnt[_HEIGHT]
        # offset is address of char definition in font array
        offset = _get_char_addr(fnt, char_code)
        # If it's an empty definition, bail
        if offset == 0:
            return True # Success
        width = fnt[offset]  # Width of this char
        offset += 1  # Address of data for this char
        buf = memoryview(fnt)
        if _landscape:
            for row in range(height):
                for col in range(width):
                    # Read the font data
                    if fnt[offset + (row * width + col) // 8] & (1 << (col % 8)):
                        _display.pixel(x + row, y + width - col -1, _color)
                    elif _bgcolor is not None:
                        _display.pixel(x + row, y + width - col -1, _bgcolor)
        else:
            for row in range(height):
                for col in range(width):
                    if fnt[offset + (col * height + row) // 8] & (1 << (row % 8)):
                        _display.pixel(x + col, y + row, _color)
                    elif _bgcolor is not None:
                        _display.pixel(x + col, y + row, _bgcolor)
        return width

    # Optional arguments color and bgcolor. Note that these are numbers not objects.
    # They are an optimisation for the normal case where the Writer has an SSD instance
    # and these colors are fixed. In the case of a display which supports true color
    # these values are passed to the font object.
    def __init__(self, device, font, color=1, bgcolor=0, verbose=True):
        super().__init__()
        self.device = device
        self.font = font
        # Current text color
        self.text_color = color
        # Current background color (None == transparent)
        self.bgcolor = bgcolor
        #ঞ্চলের x, y position. This is maintained by hardware or by this class.
        # In the latter case it facilitates setting the text position.
        self.x = 0
        self.y = 0
        # Text rotation. 0 = normal, 1 = landscape (90 deg clockwise)
        self.usd = False  # Upside down (not supported by this class)
        self.landscape = False  # Rotated 90 degrees clockwise (HA!)
        # Text reversal. Text rendered with foreground and background colors swapped.
        self.reverse = False
        # Clipping flags: True if it is required to clip characters when they
        # would extend beyond the edge of the physical display.
        self._clip = verbose  # Default to verbose messages
        self._ha = True  # Default to horizontal alignment (landscape mode)
        self._va = False  # Default to vertical alignment
        # Current font dimensions
        self.height = font[_HEIGHT]  # Font height
        self.max_width = font[_WIDTH]  # Max character width
        self.baseline = self.height -1  # Baseline is at bottom of char row
        # Default mapping of space character to its font width
        self.map_space = True

        # Allow to miss argument color for circle compatibility
        if isinstance(device, int):
            self.text_color = device
        if isinstance(font, int):
            self.bgcolor = font

    # Reset the text anچhor to values provided by the display's driver
    # This provides a way of overcoming the fact that the anچhor may be
    # constrained by a previous test.
    def set_textpos(self, device, x=None, y=None):
        if x is None:
            if hasattr(device, 'width'):
                x = device.width // 2 # Default to centre
            else:
                x = 0
        if y is None:
            if hasattr(device, 'height'):
                y = device.height // 2
            else:
                y = 0
        self.x = x
        self.y = y

    # Return the screen width of a string in pixels
    def stringlen(self, s):
        l = 0
        for char_code in s:
            l += self._char_width(ord(char_code))
        return l

    # Return the font height in pixels
    def height(self):  # Property
        return self.height

    # Return the width of a single char
    def _char_width(self, char_code):
        # If it's an empty definition, width is 0
        if _get_char_addr(self.font, char_code) == 0:
            return 0
        # Width of space char is a special case to facilitate variable pitch fonts
        # The space char in the font has zero width. If self.map_space is True
        # the width of a space is the width of the '0' character. Otherwise it
        # is zero. This is achieved by substituting '0' for space in this method.
        if self.map_space and char_code == ord(' '):
            char_code = ord('0')
        # If char is not in font, use the "missing" character (usually '?')
        if char_code >= self.font[_LAST_CHAR] or char_code < self.font[_MISSING +2]: # self.font[_FIRST_CHAR]:
            char_code = self.font[_MISSING]
        # Address of char definition in font array.
        # Offset is the address of the char width byte.
        offset = _get_char_addr(self.font, char_code) -1
        return self.font[offset] # Width of this char

    # Erase the screen. Does not update display.
    def clear_screen(self):
        if hasattr(self.device, 'fill'):
            self.device.fill(self.bgcolor if self.bgcolor is not None else 0)
        self.x = 0
        self.y = 0

    # Print a string at the current position. Text may be wrapped.
    # Does not update display.
    def printstring(self, s):
        for char_code in s:
            self._printchar(char_code)

    # Method using Writer.draw_char which needs to be overridden for displays
    # which require points to be computed.
    def _printchar(self, char_code):  # Print one character
        char_code = ord(char_code)
        # Determine width of current char
        width = self._char_width(char_code)
        # If it's an empty definition, width is 0: effectively a NOP.
        if width == 0:
            return
        # Wrap text if it extends beyond the screen edge
        if self.x + width > self.device.width:
            self.x = 0
            self.y += self.height
            # If screen has scrolled off top, do a clear screen
            if self.y >= self.device.height:
                self.clear_screen() # x and y are reset by this method
        # Clip if it goes off bottom of screen
        if self.y + self.height < 0:
            if self._clip and _VERBOSE:
                print('Font too tall for screen')
            return # Don't render: off bottom of screen

        # Off top of screen
        if self.y >= self.device.height:
            if self._clip and _VERBOSE:
                print('Font off bottom of screen')
            return

        # All clear to draw the character
        Writer.draw_char(char_code, self.x, self.y, self.device, self.font,
                         self.text_color, self.bgcolor, self.landscape, self.reverse)
        self.x += width

    # This method is out of date. The code has been incorporated into
    # Writer.draw_char().
    @classmethod
    def set_clip(cls, horiz, vert, verbose):
        cls._clip = horiz or vert
        cls._ha = horiz
        cls._va = vert
        if verbose:
            cls._vba = _VERBOSE
        else:
            cls._vba = _NO_VERBOSE
        return # cls._clip # TEST

    # For fixed pitch fonts only:
    # Return the number of characters which will fit on a row
    def chars_per_row(self):
        return self.device.width // self.max_width

    # For fixed pitch fonts only:
    # Return the x,y coordinates of a given row and column
    # If landscape=True y is the physical x dimension.
    def charpos(self, row, col):
        if self.landscape:
            return row * self.height, col * self.max_width
        return col * self.max_width, row * self.height
