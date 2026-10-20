#pragma once

#include <stdint.h>

// CH422G IO expander on the shared I2C bus. It has no register addresses:
// each command is a one-byte write to its own I2C address.
//   EXIO1  touch reset (active low)
//   EXIO2  backlight on
//   EXIO3  LCD reset (active low)
//   EXIO4  SD card chip select (active low)
enum Exio : uint8_t {
    EXIO_TOUCH_RST = 1 << 1,
    EXIO_BACKLIGHT = 1 << 2,
    EXIO_LCD_RST = 1 << 3,
    EXIO_SD_CS = 1 << 4,
};

// Starts I2C (Wire) and sets EXIO0-7 as outputs, all low.
bool ch422gBegin();

// Sets or clears one output, keeping the others.
void ch422gWrite(Exio pin, bool high);
