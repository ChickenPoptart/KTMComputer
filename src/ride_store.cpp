#include "ride_store.h"

#include <SD.h>
#include "ride_history.h"
#include "sd_log.h"

// Files start with this header. The record is written as raw memory, so
// any change to RideRecord (or what it contains) must bump RIDE_VERSION;
// the size check catches most forgotten bumps.
static const uint32_t RIDE_MAGIC = 0x4B544D52;  // "KTMR"
static const uint16_t RIDE_VERSION = 1;

struct RideFileHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t recordSize;
};

static void rideFileName(char* out, size_t cap, int number) {
    snprintf(out, cap, "/rides/ride_%04d.bin", number);
}

bool rideSave(const RideRecord& r) {
    if (sdLogNumber() < 0) return false;  // no card
    char name[32];
    rideFileName(name, sizeof(name), r.number);
    RideFileHeader h = {RIDE_MAGIC, RIDE_VERSION, 0, sizeof(RideRecord)};

    sdLock();
    File f = SD.open(name, FILE_WRITE);
    bool ok = f && f.write((const uint8_t*)&h, sizeof(h)) == sizeof(h) &&
              f.write((const uint8_t*)&r, sizeof(r)) == sizeof(r);
    if (f) f.close();
    sdUnlock();
    return ok;
}

bool rideLoad(int number, RideRecord& r) {
    if (sdLogNumber() < 0) return false;
    char name[32];
    rideFileName(name, sizeof(name), number);
    RideFileHeader h;

    sdLock();
    File f = SD.open(name, FILE_READ);
    bool ok = f && f.read((uint8_t*)&h, sizeof(h)) == sizeof(h) && h.magic == RIDE_MAGIC &&
              h.version == RIDE_VERSION && h.recordSize == sizeof(RideRecord) &&
              f.read((uint8_t*)&r, sizeof(r)) == sizeof(r);
    if (f) f.close();
    sdUnlock();
    return ok;
}

int rideList(int* numbers, int cap) {
    if (sdLogNumber() < 0) return 0;
    int count = 0;
    sdLock();
    File dir = SD.open("/rides");
    for (File f = dir.openNextFile(); f && count < cap; f = dir.openNextFile()) {
        int n = f.isDirectory() ? -1 : rideNumberFromName(f.name());
        if (n >= 0) numbers[count++] = n;
        f.close();
    }
    if (dir) dir.close();
    sdUnlock();
    return count;
}
