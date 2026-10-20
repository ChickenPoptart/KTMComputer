#include "ch422g.h"

#include <Arduino.h>
#include <Wire.h>
#include "board_w5.h"

// Command "addresses" (from Waveshare's driver: 0x48 >> 1 etc.)
static const uint8_t CMD_SET = 0x24;  // system parameters
static const uint8_t CMD_IO = 0x38;   // EXIO0-7 output levels
static const uint8_t SET_IO_OUTPUT = 0x01;

static uint8_t outputs = 0;

static bool command(uint8_t cmd, uint8_t value) {
    Wire.beginTransmission(cmd);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool ch422gBegin() {
    Wire.begin(W5_I2C_SDA, W5_I2C_SCL, 400000);
    outputs = 0;
    return command(CMD_SET, SET_IO_OUTPUT) && command(CMD_IO, outputs);
}

void ch422gWrite(Exio pin, bool high) {
    outputs = high ? (outputs | pin) : (outputs & ~pin);
    command(CMD_IO, outputs);
}
