#pragma once

#include <stdint.h>

// Decodes the 2020 KTM 690 Enduro R's CAN broadcast. IDs and byte layouts
// from github.com/blalor/ktm-can, captured on the same model:
//   0x120 (20 ms)  D0-D1 engine rpm, big-endian
//   0x129 (20 ms)  D0 high nibble gear, 0 = neutral
//   0x12B (10 ms)  D0-D1 front wheel speed, D2-D3 rear
//   0x540 (100 ms) D6-D7 coolant, tenths of a degree C

// A value counts as live only if its frame arrived this recently; the
// slowest one used (0x540) repeats every 100 ms.
const unsigned long KTM_STALE_MS = 1000;

struct KtmState {
    int rpm = 0;
    int gear = 0;  // 0 = neutral, 1..6
    int coolantF = 0;
    int speedMph = 0;

    bool haveRpm = false, haveGear = false, haveCoolant = false, haveSpeed = false;
    unsigned long rpmMs = 0, gearMs = 0, coolantMs = 0, speedMs = 0;
};

// Updates the state from one received frame; other IDs are ignored.
void ktmDecodeFrame(KtmState& s, unsigned long nowMs, uint32_t id, bool extended,
                    uint8_t len, const uint8_t* data);

// True while that value's frames are still arriving.
bool ktmRpmLive(const KtmState& s, unsigned long nowMs);
bool ktmGearLive(const KtmState& s, unsigned long nowMs);
bool ktmCoolantLive(const KtmState& s, unsigned long nowMs);
bool ktmSpeedLive(const KtmState& s, unsigned long nowMs);
