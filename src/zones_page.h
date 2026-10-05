#pragma once

#include <TFT_eSPI.h>
#include "trip_stats.h"

// Time spent in each tach colour zone during one ride, as bars.
class ZonesPage {
public:
    explicit ZonesPage(TFT_eSPI& tft) : tft(tft) {}

    void begin(const char* rideLabel, bool canOlder, bool canNewer);
    void update(const TripStats& trip);

private:
    TFT_eSPI& tft;
};
