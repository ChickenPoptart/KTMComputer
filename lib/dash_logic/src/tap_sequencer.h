#pragma once

// A second tap within this long of the first makes a double tap. Single
// taps are therefore reported this much after the finger lifts.
const unsigned long DOUBLE_TAP_MS = 350;

enum TapKind { TAP_NONE, TAP_SINGLE, TAP_DOUBLE };

struct TapResult {
    TapKind kind = TAP_NONE;
    int x = 0, y = 0;  // where a single tap landed
};

// Tells single taps (with their position) from double taps.
class TapSequencer {
public:
    // A tap ended at nowMs at screen position (x, y).
    void tap(unsigned long nowMs, int x, int y) {
        if (waiting && nowMs - firstMs < DOUBLE_TAP_MS) {
            waiting = false;
            doublePending = true;
            return;
        }
        waiting = true;
        firstMs = nowMs;
        firstX = x;
        firstY = y;
    }

    // Call every loop; returns each tap or double tap once.
    TapResult poll(unsigned long nowMs) {
        TapResult r;
        if (doublePending) {
            doublePending = false;
            r.kind = TAP_DOUBLE;
        } else if (waiting && nowMs - firstMs >= DOUBLE_TAP_MS) {
            waiting = false;
            r.kind = TAP_SINGLE;
            r.x = firstX;
            r.y = firstY;
        }
        return r;
    }

private:
    bool waiting = false, doublePending = false;
    unsigned long firstMs = 0;
    int firstX = 0, firstY = 0;
};
