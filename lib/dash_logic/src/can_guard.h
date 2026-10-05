#pragma once

#include <stdint.h>

// If the CAN receive line carries garbage (transceiver unpowered, H/L
// swapped, no ground), the ESP32 raises an error interrupt for nearly every
// bit, and its driver's chip-bug workarounds reset the peripheral inside
// that interrupt. That flood starves the rest of the firmware: the screen
// freezes and touch stops working. The guard switches CAN off for a while
// when errors swamp frames, then tries again.

// Errors per second that count as a flood (when they outnumber frames)
const uint32_t CAN_GUARD_MIN_ERRORS = 200;
// How long CAN stays off before the next attempt
const unsigned long CAN_GUARD_PAUSE_MS = 10000;
const unsigned long CAN_GUARD_WINDOW_MS = 1000;

enum GuardAction { GUARD_NONE, GUARD_STOP, GUARD_RESTART };

class CanGuard {
public:
    // Call often with running totals of frames received and bus errors.
    GuardAction check(unsigned long nowMs, uint32_t frames, uint32_t errors) {
        if (stopped) {
            if (nowMs - stoppedMs < CAN_GUARD_PAUSE_MS) return GUARD_NONE;
            stopped = false;
            startWindow(nowMs, frames, errors);
            return GUARD_RESTART;
        }
        if (!started) {
            started = true;
            startWindow(nowMs, frames, errors);
            return GUARD_NONE;
        }
        if (nowMs - windowMs < CAN_GUARD_WINDOW_MS) return GUARD_NONE;

        uint32_t newErrors = errors - windowErrors;
        uint32_t newFrames = frames - windowFrames;
        startWindow(nowMs, frames, errors);
        if (newErrors >= CAN_GUARD_MIN_ERRORS && newErrors > newFrames) {
            stopped = true;
            stoppedMs = nowMs;
            fault = true;
            return GUARD_STOP;
        }
        if (newFrames > 0 && newErrors <= newFrames) fault = false;
        return GUARD_NONE;
    }

    // True from a flood until frames flow cleanly again.
    bool faulted() const { return fault; }

private:
    void startWindow(unsigned long nowMs, uint32_t frames, uint32_t errors) {
        windowMs = nowMs;
        windowFrames = frames;
        windowErrors = errors;
    }

    bool started = false, stopped = false, fault = false;
    unsigned long windowMs = 0, stoppedMs = 0;
    uint32_t windowFrames = 0, windowErrors = 0;
};
