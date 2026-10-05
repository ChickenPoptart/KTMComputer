#pragma once

// CYD resistive touch (XPT2046), bit-banged.
void touchBegin();
bool touchIsPressed();

// Raw 12-bit panel readings (0..4095) of where the screen is pressed, or
// false if it isn't. Mapping to screen pixels needs calibration.
bool touchReadRaw(int& rawX, int& rawY);
