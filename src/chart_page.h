#pragma once

#include <TFT_eSPI.h>
#include "chart_series.h"

// One point per plot pixel column; older data merges so the whole ride fits.
const int CHART_POINTS = 280;
using Series = ChartSeries<CHART_POINTS>;

struct ChartStyle {
    const char* title;
    const char* unit;
    uint16_t color;
    int minSpan;     // smallest y range, so noise isn't blown up full-height
    bool fromZero;   // y axis starts at 0 (speed)
    // Optional second line, drawn dimmer. Left out of a {...} initializer,
    // these are zero: no overlay.
    const Series* overlay;
    uint16_t overlayColor;
    const char* overlayLabel;
};

// Line chart of one series over the whole ride (or since its last reset).
class ChartPage {
public:
    explicit ChartPage(TFT_eSPI& tft) : tft(tft) {}

    void begin(const Series& series, const ChartStyle& style, unsigned long sampleMs,
               const char* rideLabel, bool canOlder, bool canNewer);
    // Redraws when a new point arrived; cheap to call every frame.
    void update();

private:
    void drawPlot();
    void drawLine(const Series& s, uint16_t color, int16_t lo, int16_t hi);

    TFT_eSPI& tft;
    const Series* series = nullptr;
    ChartStyle style{};
    unsigned long sampleMs = 0;
    int drawnSize = -1, drawnStride = -1;
};
