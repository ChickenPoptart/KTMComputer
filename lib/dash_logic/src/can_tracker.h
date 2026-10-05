#pragma once

#include <stdint.h>

// Latest state of one CAN ID, for reverse engineering which bytes carry what.
struct CanEntry {
    uint32_t id = 0;
    bool extended = false;
    uint8_t len = 0;
    uint8_t data[8] = {};
    uint32_t count = 0;
    uint16_t hz = 0;
    uint8_t everChangedMask = 0;          // bit n set once byte n has changed
    unsigned long byteChangedMs[8] = {};  // when each byte last changed
    uint32_t countAtLastRate = 0;

    bool byteRecentlyChanged(int i, unsigned long nowMs, unsigned long windowMs) const {
        return (everChangedMask & (1 << i)) && nowMs - byteChangedMs[i] < windowMs;
    }
};

// Keeps one entry per CAN ID, sorted by ID, with change and rate tracking.
class CanTracker {
public:
    static const int CAPACITY = 64;

    // Returns true if this is a new ID or its data differs from last time.
    bool record(uint32_t id, bool extended, uint8_t len, const uint8_t* data, unsigned long nowMs);

    // Recomputes per-ID frame rates once at least a second has passed.
    void updateRates(unsigned long nowMs);

    int size() const { return count; }
    const CanEntry& at(int i) const { return entries[i]; }
    uint32_t droppedIds() const { return dropped; }
    uint32_t totalHz() const { return totalRate; }

private:
    CanEntry entries[CAPACITY];
    int count = 0;
    uint32_t dropped = 0;
    uint32_t totalRate = 0;
    unsigned long lastRateMs = 0;
};
