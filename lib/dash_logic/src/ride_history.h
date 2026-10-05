#pragma once

#include "log_rotation.h"
#include "trip_stats.h"

// Each ride (one key-on) saves its trip stats and charts to
// /rides/ride_NNNN.bin, numbered like its can_NNNN.log.
inline int rideNumberFromName(const char* name) {
    return numberedFileName(name, "ride_", ".bin");
}

// A key-on only becomes a saved ride once the engine has run this long, so
// bench power-ups and quick on/offs don't clutter the history.
const int RIDE_MIN_SECONDS = 60;

inline bool worthSaving(const TripStats& trip) {
    uint32_t runningMs = 0;
    for (int z = ZONE_IDLE; z < ZONE_COUNT; z++) runningMs += trip.zoneMs((TachZone)z);
    return runningMs >= RIDE_MIN_SECONDS * 1000UL;
}

// Which ride the history pages show: this ride (live) or a saved past one.
// < steps to older rides, > back towards this ride.
class RideBrowser {
public:
    static const int CAPACITY = 512;

    void setRides(const int* numbers, int n, int thisRide) {
        current = thisRide;
        count = 0;
        viewed = LIVE;
        for (int i = 0; i < n; i++) addRide(numbers[i]);
    }

    // Records a newly saved ride (ignored if it's this ride or already known).
    void addRide(int number) {
        if (number == current || count == CAPACITY) return;
        int pos = 0;
        while (pos < count && rides[pos] < number) pos++;
        if (pos < count && rides[pos] == number) return;
        for (int i = count; i > pos; i--) rides[i] = rides[i - 1];
        rides[pos] = number;
        count++;
    }

    bool viewingLive() const { return viewed == LIVE; }
    int viewing() const { return viewed; }  // ride number, or -1 for this ride

    bool older() {
        int i = indexOfViewed();
        if (viewed == LIVE) i = count;
        if (i <= 0) return false;
        viewed = rides[i - 1];
        return true;
    }

    bool newer() {
        if (viewed == LIVE) return false;
        int i = indexOfViewed();
        viewed = i + 1 < count ? rides[i + 1] : LIVE;
        return true;
    }

    void showLive() { viewed = LIVE; }

    bool hasNewer() const { return viewed != LIVE; }
    bool hasOlder() const {
        if (viewed == LIVE) return count > 0;
        return indexOfViewed() > 0;
    }

private:
    static const int LIVE = -1;

    int indexOfViewed() const {
        for (int i = 0; i < count; i++) if (rides[i] == viewed) return i;
        return -1;
    }

    int rides[CAPACITY];  // ascending, excluding this ride
    int count = 0;
    int current = -1;
    int viewed = LIVE;
};
