#include "gt911.h"

#include <Arduino.h>
#include <Wire.h>
#include "board_w5.h"
#include "ch422g.h"

static const uint8_t ADDR = 0x5D;
static const uint16_t REG_PRODUCT_ID = 0x8140;
static const uint16_t REG_STATUS = 0x814E;  // bit 7: data ready, low nibble: touches
static const uint16_t REG_POINT1 = 0x8150;  // x lo, x hi, y lo, y hi, ...

static bool readRegs(uint16_t reg, uint8_t* out, int n) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)ADDR, n) != n) return false;
    for (int i = 0; i < n; i++) out[i] = Wire.read();
    return true;
}

static void writeReg(uint16_t reg, uint8_t value) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(value);
    Wire.endTransmission();
}

bool gt911Begin() {
    // Waveshare's sequence: INT held low through the reset selects 0x5D
    pinMode(W5_TOUCH_INT, OUTPUT);
    digitalWrite(W5_TOUCH_INT, LOW);
    delay(10);
    ch422gWrite(EXIO_TOUCH_RST, false);
    delay(100);
    ch422gWrite(EXIO_TOUCH_RST, true);
    delay(200);
    pinMode(W5_TOUCH_INT, INPUT);

    uint8_t id[4];
    if (!readRegs(REG_PRODUCT_ID, id, 4)) return false;
    return id[0] == '9';  // product id is ASCII "911"
}

bool gt911Read(int& x, int& y) {
    uint8_t status;
    if (!readRegs(REG_STATUS, &status, 1)) return false;
    if (!(status & 0x80)) return false;  // no new data yet
    int touches = status & 0x0F;
    bool touching = false;
    if (touches > 0) {
        uint8_t p[4];
        if (readRegs(REG_POINT1, p, 4)) {
            x = p[0] | (p[1] << 8);
            y = p[2] | (p[3] << 8);
            touching = true;
        }
    }
    writeReg(REG_STATUS, 0);  // ready for the next report
    return touching;
}
