#pragma once

#include <TFT_eSPI.h>
#include "telemetry.h"

// Draws the F1-style layout on a 320x240 landscape screen. Only regions
// whose values changed since the last call are redrawn.
class DashUI {
public:
    explicit DashUI(TFT_eSPI& tft) : tft(tft) {}

    void begin();
    void update(const Telemetry& t, unsigned long nowMs);

private:
    void drawStatic();
    void drawGear(int gear);
    void drawTach(int rpm, bool blinkOn);
    void drawRpm(const Telemetry& t);
    void drawSpeed(const Telemetry& t);
    void drawCoolant(const Telemetry& t);
    void drawHeading(const Telemetry& t);
    void drawElevation(const Telemetry& t);
    void drawGpsStatus(const Telemetry& t);
    void drawClock(const Telemetry& t);

    TFT_eSPI& tft;
    bool firstFrame = true;
    Telemetry last;
    long lastClockMinute = -1;
    bool lastSegLit[8] = {};
    uint16_t lastShiftColor = 0;
};
