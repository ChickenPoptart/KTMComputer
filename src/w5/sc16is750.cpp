#include "sc16is750.h"

#include <Arduino.h>
#include <Wire.h>

static const uint8_t ADDR = 0x4D;  // A0 = A1 = GND
// The MCU-750 board's crystal
static const uint32_t XTAL_HZ = 14745600;

// Registers; on the bus each is sent as (reg << 3)
static const uint8_t REG_RHR = 0x00;  // receive (read) / transmit (write)
static const uint8_t REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03;
static const uint8_t REG_SPR = 0x07;  // scratchpad, used to detect the chip
static const uint8_t REG_TXLVL = 0x08;
static const uint8_t REG_RXLVL = 0x09;
static const uint8_t REG_DLL = 0x00;  // while LCR bit 7 is set
static const uint8_t REG_DLH = 0x01;

static void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg << 3);
    Wire.write(value);
    Wire.endTransmission();
}

static bool readReg(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg << 3);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)ADDR, 1) != 1) return false;
    value = Wire.read();
    return true;
}

bool sc16Begin(uint32_t baud) {
    uint8_t check;
    writeReg(REG_SPR, 0x5A);
    if (!readReg(REG_SPR, check) || check != 0x5A) return false;

    uint16_t divisor = XTAL_HZ / (16 * baud);
    writeReg(REG_LCR, 0x80);  // open the divisor latch
    writeReg(REG_DLL, divisor & 0xFF);
    writeReg(REG_DLH, divisor >> 8);
    writeReg(REG_LCR, 0x03);  // 8 data bits, no parity, 1 stop bit
    writeReg(REG_FCR, 0x07);  // enable and clear both FIFOs
    return true;
}

size_t sc16Read(uint8_t* out, size_t cap) {
    uint8_t waiting;
    if (!readReg(REG_RXLVL, waiting) || waiting == 0) return 0;
    size_t n = waiting < cap ? waiting : cap;

    // Repeated reads of RHR stream out of the FIFO (no auto-increment)
    Wire.beginTransmission(ADDR);
    Wire.write(REG_RHR << 3);
    if (Wire.endTransmission(false) != 0) return 0;
    size_t got = Wire.requestFrom((int)ADDR, (int)n);
    for (size_t i = 0; i < got; i++) out[i] = Wire.read();
    return got;
}

void sc16Write(const uint8_t* data, size_t len) {
    while (len > 0) {
        uint8_t space;
        if (!readReg(REG_TXLVL, space) || space == 0) return;
        size_t n = len < space ? len : space;
        if (n > 32) n = 32;  // stay within the I2C library's buffer
        Wire.beginTransmission(ADDR);
        Wire.write(REG_RHR << 3);
        Wire.write(data, n);
        Wire.endTransmission();
        data += n;
        len -= n;
    }
}
