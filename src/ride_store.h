#pragma once

#include <time.h>
#include "chart_page.h"
#include "trip_stats.h"

// Everything the history pages show for one ride.
struct RideRecord {
    int number = -1;     // matches can_NNNN.log
    time_t startUtc = 0; // 0 if the GPS never set the clock
    TripStats trip;
    Series elevation, speed, gpsSpeed, coolant;
};

// Saves to /rides/ride_NNNN.bin, replacing any earlier save of that ride.
bool rideSave(const RideRecord& r);

// Loads a saved ride; false if it's missing or from an older firmware.
bool rideLoad(int number, RideRecord& r);

// Numbers of the saved rides on the card; returns how many.
int rideList(int* numbers, int cap);
