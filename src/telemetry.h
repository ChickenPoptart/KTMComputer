#pragma once

#include <time.h>

// Latest known vehicle + GPS state. Engine values and speed come from the
// CAN decoder, heading and elevation from the GPS.
struct Telemetry {
    int rpm = 0;
    int gear = -1;         // 0 = neutral, 1..6, -1 = no data
    int speedMph = 0;
    int coolantF = 0;
    bool rpmLive = false;
    bool speedLive = false;
    bool coolantLive = false;

    bool gpsFix = false;
    int satellites = 0;
    int headingDeg = 0;
    int elevationFt = 0;

    bool canFault = false;   // CAN error flood: wiring problem
    bool timeValid = false;  // clock set from GPS
    time_t utc = 0;
};
