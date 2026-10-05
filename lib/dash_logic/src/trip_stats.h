#pragma once

#include <stdint.h>
#include "dash_logic.h"

// What the bike and GPS reported at one moment.
struct TripSample {
    bool speedLive = false;
    int speedMph = 0;
    bool rpmLive = false;
    int rpm = 0;
    bool coolantLive = false;
    int coolantF = 0;
    bool elevationValid = false;  // GPS fix with altitude
    int elevationFt = 0;
};

// Below this the bike counts as stopped for moving time.
const int TRIP_MOVING_MPH = 2;

// GPS altitude wanders by tens of feet even when parked, so a climb or
// descent only counts once it moves this far from the last level counted.
const int TRIP_ELEVATION_STEP_FT = 30;

// Running totals for the Trip page, since power-up or the last reset.
class TripStats {
public:
    // Adds dtMs of riding at the given sample.
    void update(uint32_t dtMs, const TripSample& s) {
        elapsed += dtMs;
        if (s.speedLive) {
            totalMiles += (double)s.speedMph * dtMs / 3600000.0;
            if (s.speedMph >= TRIP_MOVING_MPH) moving += dtMs;
            if (s.speedMph > topSpeed) topSpeed = s.speedMph;
        }
        if (s.rpmLive) {
            if (s.rpm > topRpm) topRpm = s.rpm;
            zone[tachZone(s.rpm)] += dtMs;
        }
        if (s.coolantLive && s.coolantF > topCoolant) topCoolant = s.coolantF;
        if (s.elevationValid) trackElevation(s.elevationFt);
    }

    uint32_t elapsedMs() const { return elapsed; }
    uint32_t movingMs() const { return moving; }
    double miles() const { return totalMiles; }
    int maxSpeedMph() const { return topSpeed; }
    int maxRpm() const { return topRpm; }
    int maxCoolantF() const { return topCoolant; }
    int gainFt() const { return gain; }
    int lossFt() const { return loss; }
    // Time spent in each tach colour zone while RPM was being received.
    uint32_t zoneMs(TachZone z) const { return zone[z]; }
    void clearZones() {
        for (uint32_t& z : zone) z = 0;
    }

private:
    void trackElevation(int ft) {
        if (!haveLevel) {
            level = ft;
            haveLevel = true;
        } else if (ft - level >= TRIP_ELEVATION_STEP_FT) {
            gain += ft - level;
            level = ft;
        } else if (level - ft >= TRIP_ELEVATION_STEP_FT) {
            loss += level - ft;
            level = ft;
        }
    }

    uint32_t elapsed = 0;
    uint32_t moving = 0;
    double totalMiles = 0;
    int topSpeed = 0;
    int topRpm = 0;
    int topCoolant = 0;
    bool haveLevel = false;
    int level = 0;
    int gain = 0;
    int loss = 0;
    uint32_t zone[ZONE_COUNT] = {};
};
