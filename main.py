import machine
import time
import uos # For checking if lib exists, though not strictly needed now

# Attempt to import library files from /lib directory
try:
    from lib import ds3231
    from lib import ili9341
    from lib import writer
    from lib import font6 # Assuming font6.py contains `_font` bytearray
except ImportError:
    print("Error: Ensure ds3231.py, ili9341.py, writer.py, and font6.py are in the /lib folder on your Pico.")
    # Optional: halt execution if libs are missing
    # while True:
    #     pass

# Pin definitions (as per plan)
# I2C for DS3231
I2C_SDA_PIN = 0
I2C_SCL_PIN = 1

# SPI for ILI9341 (using SPI0)
SPI_SCK_PIN = 18
SPI_MOSI_PIN = 19
SPI_MISO_PIN = 16 # Set to -1 or None if not used by your specific ILI9341 board/lib config
ILI9341_CS_PIN = 17
ILI9341_DC_PIN = 20
ILI9341_RST_PIN = 21

# Display dimensions (default for ILI9341)
DISPLAY_WIDTH = 240
DISPLAY_HEIGHT = 320

# Global variable for temperature unit preference
METRIC_UNITS = True # True for Celsius, False for Fahrenheit

# --- Initialize Peripherals ---

# I2C for DS3231
i2c = None
rtc = None
try:
    scl_pin_obj = machine.Pin(I2C_SCL_PIN)
    sda_pin_obj = machine.Pin(I2C_SDA_PIN)
    i2c = machine.I2C(0, scl=scl_pin_obj, sda=sda_pin_obj, freq=100000) # Use I2C bus 0

    # Scan I2C bus to check for DS3231 (optional debug)
    # print("Scanning I2C bus...")
    # devices = i2c.scan()
    # if ds3231.DS3231_I2C_ADDR in devices:
    #     print(f"DS3231 found at 0x{ds3231.DS3231_I2C_ADDR:02X}")
    # else:
    #     print(f"DS3231 not found. Check wiring. Expected at 0x{ds3231.DS3231_I2C_ADDR:02X}")
        # Consider halting or error display

    rtc = ds3231.DS3231(i2c)
except Exception as e:
    print(f"Error initializing DS3231: {e}")
    # Consider visual error on display if available, or LED blink

# SPI for ILI9341
spi = None
display = None
screen_writer = None

try:
    spi = machine.SPI(0, baudrate=40000000, # Standard SPI0 pins
                      sck=machine.Pin(SPI_SCK_PIN),
                      mosi=machine.Pin(SPI_MOSI_PIN),
                      miso=machine.Pin(SPI_MISO_PIN) if SPI_MISO_PIN != -1 else None)

    display = ili9341.ILI9341(spi,
                              cs=machine.Pin(ILI9341_CS_PIN),
                              dc=machine.Pin(ILI9341_DC_PIN),
                              rst=machine.Pin(ILI9341_RST_PIN),
                              width=DISPLAY_WIDTH, # Standard for most ILI9341 in portrait
                              height=DISPLAY_HEIGHT,
                              rotation=0) # Portrait mode (0 or 2). 1 or 3 for landscape.
                                        # The example uses rotation 0 (width=128, height=160 for ST7735)
                                        # For ILI9341 usually 240x320.
                                        # If text appears sideways, change rotation (e.g. to 1 for landscape if preferred)

    display.fill(ili9341.BLACK) # Clear display

    # Initialize Writer for text
    # Ensure font6._font is the actual bytearray data from your font6.py
    screen_writer = writer.Writer(display, font6._font)
    writer.Writer.set_clip(True, True, False) # Clip text, no verbose messages
    screen_writer.text_color = ili9341.WHITE # Default text color

except Exception as e:
    print(f"Error initializing ILI9341 display: {e}")
    # Consider visual error (e.g. onboard LED blink) if display fails

# --- RTC Time Setting (run once) ---
def set_initial_rtc_time():
    if rtc:
        # Format: (year, month, day, weekday, hour, minute, second, subseconds)
        # Weekday: For this DS3231 library, let's assume 1=Monday, ..., 7=Sunday
        # (This might need adjustment based on the specific ds3231.py lib's convention)
        # Example: June 21, 2024 (Friday), 10:30:00
        # To get weekday: time.gmtime(time.mktime((2024,6,21,10,30,0,0,0)))[6] -> 4 (Friday, Mon=0)
        # So, for Mon=1..Sun=7, Friday is 5.

        # Check if time seems valid (e.g. year > 2023) to avoid resetting if already set
        current_time_check = rtc.datetime()
        if current_time_check[0] < 2024: # If year is before 2024, assume it needs setting
            print("Setting RTC time...")
            #                         YYYY, MM, DD, WDAY, HH, MM, SS, SUBSEC (ignored by DS3231 lib)
            # WDAY: 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat, 7=Sun
            # This needs to match the ds3231.py library's expectation for weekday.
            # The example site's code uses Python's time.struct_time convention (Mon=0, Sun=6)
            # for `t.tm_wday` and then maps it to `days` array.
            # Our ds3231.py `datetime` setter expects WDAY 1-7.
            # Let's set to a known date/time. User should change this.
            # Example: 2024-07-04 12:00:00 Thursday (Weekday = 4 for Mon=1..Sun=7)
            # To make it easier, we can use Python's time module to calculate weekday if needed,
            # but for initial set, hardcoding is fine.
            # time_to_set = (2024, 7, 4, 4, 12, 0, 0, 0) # Year, Mon, Day, Wkday, Hr, Min, Sec

            # Get current time from system if possible (e.g. if Pico has NTP sync, not by default)
            # Or hardcode a compile time / recent time:
            # For example, let's set it to a fixed date for now.
            # User MUST update this or uncomment and modify before running for the first time.
            # time_to_set = (2024, 1, 1, 1, 0, 0, 0, 0) # Jan 1, 2024, Monday 00:00:00
            # print(f"Setting time to: {time_to_set}")
            # rtc.datetime(time_to_set)
            # print("RTC time set. Please comment out set_initial_rtc_time() call for subsequent runs.")

            # Instruct user to uncomment and set manually:
            if screen_writer:
                screen_writer.set_textpos(display, 0, 0) # display required by writer
                screen_writer.text_color = ili9341.YELLOW
                screen_writer.printstring("RTC TIME NOT SET!\nEdit main.py,\nset_initial_rtc_time()\n then re-run.")
                display.show() # If display has show() method, like Adafruit's. Our lib doesn't.
                               # Updates are usually immediate with fill_rect or pixel.
            print("RTC time likely not set. Please edit set_initial_rtc_time() in main.py,")
            print("uncomment the rtc.datetime(...) line with the correct current time,")
            print("run the script once, then comment out the rtc.datetime(...) line again.")

        else:
            print(f"RTC time seems to be set: {current_time_check}")


# Call this ONCE to set the time, then comment it out.
# Ensure you have the correct time and weekday format for your ds3231 library.
# set_initial_rtc_time()


# --- Helper Functions ---
def c_to_f(celsius):
    return (celsius * 1.8) + 32

# Weekday names (align with Python's time.struct_time tm_wday: Mon=0, Sun=6)
# The DS3231 library might return weekday 1-7. We need to map.
# Our DS3231 lib: (..., wday, ...) where wday is 1-7 (Mon=1..Sun=7)
# Python struct_time: (..., ..., ..., ..., ..., ..., tm_wday, ...) where tm_wday is 0-6 (Mon=0..Sun=6)
DAYS_OF_WEEK = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]

# Global vars for min/max temp (simple daily reset logic not implemented here)
min_temperature_c = 100.0
max_temperature_c = -100.0
current_temperature_c = 0.0


def update_display_info(dt_tuple, temp_c):
    global min_temperature_c, max_temperature_c, current_temperature_c

    if not screen_writer or not display:
        print("Display or writer not initialized.")
        return

    current_temperature_c = temp_c
    if temp_c > max_temperature_c:
        max_temperature_c = temp_c
    if temp_c < min_temperature_c:
        min_temperature_c = temp_c

    year, month, mday, wday_rtc, hour, minute, sec, _ = dt_tuple

    # Map RTC weekday (1-7, Mon=1) to Python weekday (0-6, Mon=0) for DAYS_OF_WEEK lookup
    # If RTC wday is 1 (Mon) -> DAYS_OF_WEEK index 0
    # If RTC wday is 7 (Sun) -> DAYS_OF_WEEK index 6
    py_wday = wday_rtc - 1
    if py_wday < 0: py_wday = 6 # Should not happen if RTC returns 1-7 for Mon-Sun

    date_str = f"{DAYS_OF_WEEK[py_wday]}, {mday:02}/{month:02}/{year}"
    time_str = f"{hour:02}:{minute:02}:{sec:02}"

    if METRIC_UNITS:
        temp_display_str = f"{temp_c:.1f} C"
        min_temp_str = f"Min:{min_temperature_c:.1f}C"
        max_temp_str = f"Max:{max_temperature_c:.1f}C"
    else:
        temp_f = c_to_f(temp_c)
        min_temp_f = c_to_f(min_temperature_c)
        max_temp_f = c_to_f(max_temperature_c)
        temp_display_str = f"{temp_f:.1f} F"
        min_temp_str = f"Min:{min_temp_f:.1f}F"
        max_temp_str = f"Max:{max_temp_f:.1f}F"

    # Clear display before drawing new info
    display.fill(ili9341.BLACK)

    # --- Layout using Writer ---
    # Title
    screen_writer.text_color = ili9341.CYAN
    screen_writer.set_textpos(display, 10, (DISPLAY_WIDTH - screen_writer.stringlen("PICO CLOCK")) // 2) # Centered
    screen_writer.printstring("PICO CLOCK")

    # Date
    screen_writer.text_color = ili9341.GREEN
    # Approx center: (DISPLAY_WIDTH - screen_writer.stringlen(date_str)) // 2
    screen_writer.set_textpos(display, 40, 10)
    screen_writer.printstring(date_str)

    # Time (larger) - Writer doesn't directly support scaling.
    # For scaling, you'd need a larger font or draw char by char with scaling.
    # For now, use same font size.
    screen_writer.text_color = ili9341.WHITE
    # Approx center: (DISPLAY_WIDTH - screen_writer.stringlen(time_str)) // 2
    screen_writer.set_textpos(display, 70, (DISPLAY_WIDTH - screen_writer.stringlen(time_str)*1)//2 ) # Assuming font6 is small
    # If we want "larger" time, we'd need to use a larger font file and switch Writer's font
    # or implement a custom scaling draw routine.
    # For simplicity, we use one font.
    screen_writer.printstring(time_str)


    # Temperature Label
    screen_writer.text_color = ili9341.YELLOW
    screen_writer.set_textpos(display, 110, (DISPLAY_WIDTH - screen_writer.stringlen("Temperature")) // 2)
    screen_writer.printstring("Temperature")

    # Temperature Value (larger)
    screen_writer.text_color = ili9341.WHITE
    screen_writer.set_textpos(display, 140, (DISPLAY_WIDTH - screen_writer.stringlen(temp_display_str)) // 2)
    screen_writer.printstring(temp_display_str)

    # Min/Max Temperature
    screen_writer.text_color = ili9341.RED
    screen_writer.set_textpos(display, 180, 10)
    screen_writer.printstring(max_temp_str)

    screen_writer.text_color = ili9341.BLUE # Changed from RED for min to distinguish
    screen_writer.set_textpos(display, 180, DISPLAY_WIDTH - screen_writer.stringlen(min_temp_str) - 10) # Right align approx
    screen_writer.printstring(min_temp_str)

    # The writer updates pixels directly, no separate display.show() is usually needed for Hinch's writer
    # unless the underlying display driver requires it (our ili9341.py does not).

    # Print to console for debugging
    # print(f"Date: {date_str}, Time: {time_str}, Temp: {temp_display_str}")
    # print(f"{min_temp_str}, {max_temp_str}")


# --- Main Loop ---
def main():
    # Call set_initial_rtc_time() ONCE when you first run the code
    # then comment it out for subsequent runs.
    # Make sure the time set in the function is correct.
    # For example, if you just flashed and the RTC is not set:
    if rtc and rtc.datetime()[0] < 2024 : # year < 2024
         set_initial_rtc_time() # This will print instructions
         # Halting here until user sets time might be good.
         # while True: time.sleep(1)


    if not rtc or not display or not screen_writer:
        print("Critical component (RTC or Display) failed to initialize. Halting.")
        # Optionally blink Pico's onboard LED
        led = machine.Pin("LED", machine.Pin.OUT)
        while True:
            led.toggle()
            time.sleep(0.5)
        # return # Stop further execution

    print("Starting main loop...")
    while True:
        try:
            if rtc:
                current_dt = rtc.datetime()
                # The DS3231 library might need a temperature conversion trigger
                # or it might update periodically. The example used `force_temperature_conversion()`.
                # Our DS3231 lib's `temperature()` just reads. It might be stale by up to 64s.
                # If `start_temperature_conversion()` exists and is needed:
                # rtc.start_temperature_conversion()
                # time.sleep_ms(150) # Wait for conversion (typical DS3231 conversion time)
                current_temp_c = rtc.temperature()

                update_display_info(current_dt, current_temp_c)
            else:
                # Handle case where RTC is not available but display might be
                if screen_writer:
                    screen_writer.set_textpos(display, 0,0)
                    screen_writer.text_color = ili9341.RED
                    screen_writer.printstring("RTC Error!")
                print("RTC not available.")

            time.sleep(1) # Update every second

        except Exception as e:
            print(f"Error in main loop: {e}")
            # Optionally try to re-initialize display or show error message on it
            if screen_writer:
                try:
                    display.fill(ili9341.BLACK)
                    screen_writer.set_textpos(display, 0, 0)
                    screen_writer.text_color = ili9341.RED
                    screen_writer.printstring("Main Loop Error")
                    screen_writer.set_textpos(display, 20, 0)
                    # screen_writer.printstring(str(e)) # May be too long
                except Exception as e2:
                    print(f"Error trying to display main loop error: {e2}")
            time.sleep(5) # Pause before retrying or exiting


if __name__ == "__main__":
    main()
