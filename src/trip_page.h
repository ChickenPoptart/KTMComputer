#pragma once

#include <TFT_eSPI.h>
#include "trip_stats.h"

// Totals for one ride, in a 2 x 4 grid under the history header.
class TripPage {
public:
    explicit TripPage(TFT_eSPI& tft) : tft(tft) {}

    void begin(const char* rideLabel, bool canOlder, bool canNewer);
    void update(const TripStats& trip);

private:
    void drawCell(int col, int row, const char* value);

    TFT_eSPI& tft;
};
