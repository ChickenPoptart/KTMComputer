#pragma once

// A position older than this (the GPS sends one per second by default)
// means the fix was lost, e.g. in a canyon or under trees.
const unsigned long GPS_STALE_MS = 3000;

// Below this the GPS course is mostly noise, so the last heading is kept.
const double GPS_HEADING_MIN_MPH = 3.0;

// One snapshot of what the GPS parser currently knows.
struct GpsReading {
    bool locationValid = false;
    unsigned long ageMs = 0;  // since the last position update
    int satellites = 0;
    double courseDeg = 0;
    double speedMph = 0;
    bool altitudeValid = false;
    double altitudeFt = 0;
};

// What the dash shows.
struct GpsView {
    bool fix = false;
    int satellites = 0;
    int headingDeg = 0;
    int elevationFt = 0;
    int speedMph = 0;
};

// Updates the view from the latest reading.
inline void gpsUpdateView(GpsView& v, const GpsReading& r) {
    v.fix = r.locationValid && r.ageMs <= GPS_STALE_MS;
    v.satellites = r.satellites;
    if (!v.fix) return;
    v.speedMph = (int)(r.speedMph + 0.5);
    if (r.altitudeValid) v.elevationFt = (int)(r.altitudeFt + 0.5);
    if (r.speedMph >= GPS_HEADING_MIN_MPH) v.headingDeg = (int)(r.courseDeg + 0.5) % 360;
}
