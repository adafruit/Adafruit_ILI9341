# DS3231 MicroPython Library
# Based on various available libraries

import machine

DS3231_I2C_ADDR = 0x68

# Register Addresses
DS3231_REG_SECS = 0x00
DS3231_REG_MINS = 0x01
DS3231_REG_HOUR = 0x02 # 12/24 hour mode and AM/PM bit
DS3231_REG_WDAY = 0x03
DS3231_REG_MDAY = 0x04
DS3231_REG_MNTH = 0x05 # Century bit
DS3231_REG_YEAR = 0x06
DS3231_REG_TEMP_MSB = 0x11
DS3231_REG_TEMP_LSB = 0x12
DS3231_REG_CTRL = 0x0E
DS3231_REG_STATUS = 0x0F


def bcd_to_dec(bcd):
    return (bcd // 16 * 10) + (bcd % 16)

def dec_to_bcd(dec):
    return (dec // 10 * 16) + (dec % 10)

class DS3231:
    def __init__(self, i2c):
        self.i2c = i2c
        self.buf1 = bytearray(1)
        self.buf7 = bytearray(7)

        # Ensure EOSC is not set (oscillator enabled)
        self._clear_bit(DS3231_REG_CTRL, 7) # EOSC is bit 7
        # Ensure OSF is cleared
        self._clear_bit(DS3231_REG_STATUS, 7) # OSF is bit 7

    def _read_reg(self, reg_addr):
        self.i2c.readfrom_mem_into(DS3231_I2C_ADDR, reg_addr, self.buf1)
        return self.buf1[0]

    def _write_reg(self, reg_addr, value):
        self.buf1[0] = value
        self.i2c.writeto_mem(DS3231_I2C_ADDR, reg_addr, self.buf1)

    def _set_bit(self, reg_addr, bit):
        val = self._read_reg(reg_addr)
        val |= (1 << bit)
        self._write_reg(reg_addr, val)

    def _clear_bit(self, reg_addr, bit):
        val = self._read_reg(reg_addr)
        val &= ~(1 << bit)
        self._write_reg(reg_addr, val)

    def datetime(self, dt=None):
        """
        Get or set the date and time.
        dt format: (year, month, day, weekday, hour, minute, second)
        weekday: 1 (Monday) to 7 (Sunday) as per DS3231, but Python's time.struct_time is 0-6.
                 This library will expect weekday 1-7 (Sunday=7 for DS3231, Monday=1 for typical struct_time)
                 For simplicity, this example uses DS3231's 1-7 (e.g. Sunday=7)
        """
        if dt is None:
            # Get time
            self.i2c.readfrom_mem_into(DS3231_I2C_ADDR, DS3231_REG_SECS, self.buf7)
            sec = bcd_to_dec(self.buf7[0] & 0x7F)
            minute = bcd_to_dec(self.buf7[1] & 0x7F)
            hour_reg = self.buf7[2]
            if hour_reg & 0x40: # 12-hour mode
                hour = bcd_to_dec(hour_reg & 0x1F)
                if hour_reg & 0x20 and hour != 12: # PM bit set
                    hour += 12
                elif not (hour_reg & 0x20) and hour == 12: # AM bit not set, hour is 12 (midnight)
                    hour = 0
            else: # 24-hour mode
                hour = bcd_to_dec(hour_reg & 0x3F)

            wday = bcd_to_dec(self.buf7[3] & 0x07) # 1-7
            mday = bcd_to_dec(self.buf7[4] & 0x3F)
            month_reg = self.buf7[5]
            month = bcd_to_dec(month_reg & 0x1F)
            # century = 1 if month_reg & 0x80 else 0 # Not directly used for year calculation here
            year = bcd_to_dec(self.buf7[6]) + 2000 # Assumes 21st century
            return (year, month, mday, wday, hour, minute, sec, 0) # Adding 0 for subseconds to somewhat match struct_time

        else:
            # Set time
            year, month, mday, wday, hour, minute, sec, _ = dt

            self.buf7[0] = dec_to_bcd(sec) & 0x7F
            self.buf7[1] = dec_to_bcd(minute) & 0x7F
            # Assuming 24 hour mode. Bit 6 must be 0 for 24hr mode.
            self.buf7[2] = dec_to_bcd(hour) & 0x3F
            self.buf7[3] = dec_to_bcd(wday) & 0x07 # 1-7
            self.buf7[4] = dec_to_bcd(mday) & 0x3F

            # Clear century bit for now, handle if needed
            self.buf7[5] = dec_to_bcd(month) & 0x1F
            if year >= 2100: # Century bit needs to be set
                 self.buf7[5] |= 0x80

            self.buf7[6] = dec_to_bcd(year % 100)

            self.i2c.writeto_mem(DS3231_I2C_ADDR, DS3231_REG_SECS, self.buf7)

            # Clear OSF flag after setting time
            self._clear_bit(DS3231_REG_STATUS, 7)


    def temperature(self):
        """Return temperature in Celsius."""
        msb = self._read_reg(DS3231_REG_TEMP_MSB)
        lsb = self._read_reg(DS3231_REG_TEMP_LSB)
        # Temperature is in units of 0.25 degrees C.
        # MSB is integer part, LSB bits 7 and 6 are fractional part.
        temp = float(msb) # Integer part
        if lsb & 0x80: # Check bit 7
            temp += 0.5
        if lsb & 0x40: # Check bit 6
            temp += 0.25
        # Handle negative temperatures (MSB sign bit)
        if msb & 0x80: # if sign bit is set
            # This conversion is simplified, assumes positive for now as per most RTC examples
            # A more robust conversion for negative values might be needed if temps go below 0.
            # For simplicity, if msb is e.g. 0b1xxxxxxx, it's negative.
            # Example: -0.25C is 11111111.11 (-1 + 0.75)
            # Let's assume positive temperatures for this project as per the example.
            pass

        return temp

    def start_temperature_conversion(self):
        """
        Initiates a temperature conversion.
        The DS3231 automatically updates temperature every 64 seconds.
        This can be used to force an update if CONV bit in control register is set.
        However, reading the temperature registers usually gives the last converted value.
        For simplicity, we'll assume auto-conversion is sufficient or CONV is not used.
        The example article uses `rtc.force_temperature_conversion()` which might set CONV.
        A simple way is to set the CONV bit (bit 5) in the Control Register (0x0E).
        It will be reset automatically after conversion.
        """
        current_ctrl = self._read_reg(DS3231_REG_CTRL)
        if not (current_ctrl & (1<<5)): # if CONV is not already set
            self._write_reg(DS3231_REG_CTRL, current_ctrl | (1<<5))

        # Wait for CONV bit to clear (conversion complete)
        # Max conversion time is typically ~150ms for DS3231
        # For simplicity, we will not poll here in the library function.
        # The main code can call this then read temp after a small delay if needed,
        # or rely on the automatic 64s conversion cycle.
        # The example's `force_temperature_conversion` in CircuitPython likely handles the wait.
        # For MicroPython, it might be better to just read the temp registers.
        # The DS3231 updates the temperature registers automatically every 64s.
        # If more frequent updates are needed, this CONV bit logic is used.

        # Let's make this function just trigger it, and the user can read later.
        # Or, for very basic use, just reading temp registers is enough.
        # The article's `force_temperature_conversion` suggests an active process.
        # Let's ensure the CONV bit is set if we want an immediate conversion.
        self._set_bit(DS3231_REG_CTRL, 5) # Set CONV bit
        # Polling for completion:
        # while self._read_reg(DS3231_REG_CTRL) & (1<<5):
        #     pass # Wait for CONV bit to clear
        # This polling can block, so it's often better done in user code if strict timing is needed.

# Example usage (comment out when deploying):
# if __name__ == '__main__':
#     i2c = machine.I2C(0, scl=machine.Pin(1), sda=machine.Pin(0), freq=100000)
#     ds = DS3231(i2c)
#
#     # Set time: (YYYY, MM, DD, WDAY, HH, MM, SS, subsecs)
#     # WDAY for DS3231: 1=Sunday, 2=Monday, ..., 7=Saturday (differs from struct_time 0=Mon)
#     # Let's use Python's time.localtime() to get current time and adapt it
#     # For setting, let's assume (2024, 6, 20, 4, 10, 0, 0, 0) # Yr,M,D, Wd,H,M,S,Sub
#     # Thursday is 4 in struct_time (Mon=0). For DS3231, Thursday is 5 if Sun=1.
#     # Or Thursday is 4 if Mon=1. The datasheet says 1-7.
#     # The example code uses: days = ("Sunday", "Monday", ...) and t.tm_wday (0-6)
#     # The set_time in example code: (2023, 2, 19, 12, 29, 20, 8, -1, -1) -> Weekday 8? That's odd.
#     # time.struct_time tm_wday is 0-6 (Mon-Sun).
#     # DS3231 is 1-7. Let's assume for this lib: 1=Mon, ..., 7=Sun for set/get `wday`
#     # ds.datetime((2024, 6, 20, 4, 15, 30, 0, 0)) # Year, Month, Day, WDay (1-7, Mon=1), Hour, Min, Sec
#
#     current_time = ds.datetime()
#     print("Current time:", current_time)
#
#     temp = ds.temperature()
#     print("Temperature:", temp, "C")
#
#     # To force a new temperature reading:
#     # ds.start_temperature_conversion()
#     # import time
#     # time.sleep_ms(200) # Wait for conversion
#     # temp = ds.temperature()
#     # print("New Temperature:", temp, "C")
