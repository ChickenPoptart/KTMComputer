#pragma once

// Bench calibration of this CYD's touch panel, 2026-10-03, screen rotation 1
// (320 x 240 landscape). The panel is turned relative to the display: raw Y
// runs left to right across the screen, raw X runs top to bottom. Values
// are the averaged corner taps. Redo if SCREEN_ROTATION changes.
const int TOUCH_RAW_LEFT = 446, TOUCH_RAW_RIGHT = 3617;   // raw Y
const int TOUCH_RAW_TOP = 540, TOUCH_RAW_BOTTOM = 3498;   // raw X
const int SCREEN_W = 320, SCREEN_H = 240;

inline int touchScale(int raw, int rawLo, int rawHi, int pixels) {
    int p = (int)((long)(raw - rawLo) * (pixels - 1) / (rawHi - rawLo));
    if (p < 0) return 0;
    if (p > pixels - 1) return pixels - 1;
    return p;
}

// Screen pixel for a raw touch reading, clamped to the screen.
inline void touchToScreen(int rawX, int rawY, int& x, int& y) {
    x = touchScale(rawY, TOUCH_RAW_LEFT, TOUCH_RAW_RIGHT, SCREEN_W);
    y = touchScale(rawX, TOUCH_RAW_TOP, TOUCH_RAW_BOTTOM, SCREEN_H);
}
