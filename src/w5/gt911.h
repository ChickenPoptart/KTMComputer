#pragma once

// GT911 capacitive touch controller on the shared I2C bus. Reports screen
// pixels directly (800 x 480), so no calibration is needed.

// Resets the controller (through the CH422G) so it answers at 0x5D.
// Call after ch422gBegin().
bool gt911Begin();

// The first touch point, if the screen is being touched.
bool gt911Read(int& x, int& y);
