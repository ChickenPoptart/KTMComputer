#pragma once

enum PressEvent { PRESS_NONE, PRESS_TAP, PRESS_LONG };

// Turns a noisy "is the screen being touched" signal into taps and long
// presses.
class PressDetector {
public:
    // How long the screen must read untouched before a press counts as
    // released. Resistive panels drop out briefly under a steady finger.
    static const unsigned long RELEASE_MS = 60;
    // Holding this long is a long press, reported while still held
    static const unsigned long LONG_MS = 1000;

    PressEvent update(bool touching, unsigned long nowMs) {
        if (touching) {
            if (!pressed) {
                pressStartMs = nowMs;
                longReported = false;
            }
            pressed = true;
            lastTouchMs = nowMs;
            if (!longReported && nowMs - pressStartMs >= LONG_MS) {
                longReported = true;
                return PRESS_LONG;
            }
            return PRESS_NONE;
        }
        if (pressed && nowMs - lastTouchMs >= RELEASE_MS) {
            pressed = false;
            return longReported ? PRESS_NONE : PRESS_TAP;
        }
        return PRESS_NONE;
    }

private:
    bool pressed = false;
    bool longReported = false;
    unsigned long pressStartMs = 0;
    unsigned long lastTouchMs = 0;
};
