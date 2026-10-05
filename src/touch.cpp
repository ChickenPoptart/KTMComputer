#include "touch.h"

#include <Arduino.h>

// CYD touch controller (XPT2046) pins. Both hardware SPI buses are taken
// (display on HSPI, MicroSD on VSPI), so the controller is bit-banged.
// "Is it pressed" comes from its pen IRQ; positions from 12-bit reads.
static const int TOUCH_CLK = 25;
static const int TOUCH_MISO = 39;
static const int TOUCH_MOSI = 32;
static const int TOUCH_CS = 33;
static const int TOUCH_IRQ = 36;  // low while the panel is pressed

// Start bit | channel | 12-bit | differential | power-down with pen IRQ on
static const uint8_t CMD_READ_X = 0xD0;
static const uint8_t CMD_READ_Y = 0x90;

// Averaged per reading; resistive panels are noisy
static const int SAMPLES = 4;

// Sends a command and returns the 12-bit conversion result.
static uint16_t transfer(uint8_t cmd) {
    digitalWrite(TOUCH_CS, LOW);
    for (int i = 7; i >= 0; i--) {
        digitalWrite(TOUCH_MOSI, (cmd >> i) & 1);
        digitalWrite(TOUCH_CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(TOUCH_CLK, LOW);
        delayMicroseconds(1);
    }
    digitalWrite(TOUCH_MOSI, LOW);
    // The controller shifts out a busy bit, then 12 data bits, MSB first
    uint16_t raw = 0;
    for (int i = 0; i < 16; i++) {
        digitalWrite(TOUCH_CLK, HIGH);
        delayMicroseconds(1);
        raw = (raw << 1) | digitalRead(TOUCH_MISO);
        digitalWrite(TOUCH_CLK, LOW);
        delayMicroseconds(1);
    }
    digitalWrite(TOUCH_CS, HIGH);
    return (raw >> 3) & 0x0FFF;
}

void touchBegin() {
    pinMode(TOUCH_CLK, OUTPUT);
    pinMode(TOUCH_MOSI, OUTPUT);
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(TOUCH_MISO, INPUT);
    pinMode(TOUCH_IRQ, INPUT);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(TOUCH_CLK, LOW);

    // Leaves the controller idle with its pen-down interrupt enabled
    transfer(CMD_READ_X);
}

bool touchIsPressed() {
    return digitalRead(TOUCH_IRQ) == LOW;
}

bool touchReadRaw(int& rawX, int& rawY) {
    if (!touchIsPressed()) return false;
    long sx = 0, sy = 0;
    for (int i = 0; i < SAMPLES; i++) {
        sx += transfer(CMD_READ_X);
        sy += transfer(CMD_READ_Y);
    }
    // Lifted mid-read: the samples are garbage
    if (!touchIsPressed()) return false;
    rawX = sx / SAMPLES;
    rawY = sy / SAMPLES;
    return true;
}
