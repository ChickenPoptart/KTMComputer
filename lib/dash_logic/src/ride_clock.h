#pragma once

#include <time.h>

// Utah: Mountain time with US daylight saving (2nd Sunday of March to the
// 1st Sunday of November), as a POSIX TZ string for setenv("TZ", ...).
const char* const MOUNTAIN_TZ = "MST7MDT,M3.2.0,M11.1.0";

// Seconds since 1970-01-01 UTC for a GPS date and time (which are UTC).
time_t utcEpoch(int year, int month, int day, int hour, int minute, int second);

// Local time and date in the TZ set with setenv/tzset (MOUNTAIN_TZ on the
// bike): "3:05 PM" and "Sat Oct 3".
void formatClock(char* out, int cap, time_t utc);
void formatDate(char* out, int cap, time_t utc);
