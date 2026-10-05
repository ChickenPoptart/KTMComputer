#include "can_tracker.h"

// Sort order: standard IDs first, then extended, each ascending
static bool keyLess(uint32_t idA, bool extA, uint32_t idB, bool extB) {
    if (extA != extB) return !extA;
    return idA < idB;
}

bool CanTracker::record(uint32_t id, bool extended, uint8_t len, const uint8_t* data, unsigned long nowMs) {
    if (len > 8) len = 8;

    int pos = 0;
    while (pos < count && keyLess(entries[pos].id, entries[pos].extended, id, extended)) pos++;

    bool exists = pos < count && entries[pos].id == id && entries[pos].extended == extended;
    if (!exists) {
        if (count == CAPACITY) {
            dropped++;
            return false;
        }
        for (int i = count; i > pos; i--) entries[i] = entries[i - 1];
        count++;
        CanEntry fresh;
        fresh.id = id;
        fresh.extended = extended;
        fresh.len = len;
        for (int i = 0; i < len; i++) fresh.data[i] = data[i];
        fresh.count = 1;
        entries[pos] = fresh;
        return true;
    }

    CanEntry& e = entries[pos];
    e.count++;
    bool changed = e.len != len;
    for (int i = 0; i < len; i++) {
        if (e.data[i] != data[i]) {
            e.data[i] = data[i];
            e.everChangedMask |= 1 << i;
            e.byteChangedMs[i] = nowMs;
            changed = true;
        }
    }
    e.len = len;
    return changed;
}

void CanTracker::updateRates(unsigned long nowMs) {
    unsigned long elapsed = nowMs - lastRateMs;
    if (elapsed < 1000) return;

    totalRate = 0;
    for (int i = 0; i < count; i++) {
        CanEntry& e = entries[i];
        e.hz = (e.count - e.countAtLastRate) * 1000UL / elapsed;
        e.countAtLastRate = e.count;
        totalRate += e.hz;
    }
    lastRateMs = nowMs;
}
