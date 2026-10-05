#include "ktm_decode.h"

// Front wheel counts per 0.1 mph, from the 2026-10-01 ride against the
// bike's speedometer (22 and 40 mph runs); about +/-5%. See docs/can-signals.md
static const int FRONT_COUNTS_PER_10_MPH = 274;

static uint16_t be16(const uint8_t* p) { return (p[0] << 8) | p[1]; }

static void markSeen(bool& have, unsigned long& atMs, unsigned long nowMs) {
    have = true;
    atMs = nowMs;
}

static bool live(bool have, unsigned long atMs, unsigned long nowMs) {
    return have && nowMs - atMs <= KTM_STALE_MS;
}

void ktmDecodeFrame(KtmState& s, unsigned long nowMs, uint32_t id, bool extended,
                    uint8_t len, const uint8_t* data) {
    if (extended || len < 2) return;

    if (id == 0x120) {
        s.rpm = be16(data);
        markSeen(s.haveRpm, s.rpmMs, nowMs);
    } else if (id == 0x129) {
        s.gear = data[0] >> 4;
        markSeen(s.haveGear, s.gearMs, nowMs);
    } else if (id == 0x12B && len >= 4) {
        s.speedMph = (be16(data) * 10 + FRONT_COUNTS_PER_10_MPH / 2) / FRONT_COUNTS_PER_10_MPH;
        markSeen(s.haveSpeed, s.speedMs, nowMs);
    } else if (id == 0x540 && len >= 8) {
        // Tenths of a degree C to whole degrees F, rounded
        s.coolantF = (be16(data + 6) * 9 + 25) / 50 + 32;
        markSeen(s.haveCoolant, s.coolantMs, nowMs);
    }
}

bool ktmRpmLive(const KtmState& s, unsigned long nowMs) { return live(s.haveRpm, s.rpmMs, nowMs); }
bool ktmGearLive(const KtmState& s, unsigned long nowMs) { return live(s.haveGear, s.gearMs, nowMs); }
bool ktmCoolantLive(const KtmState& s, unsigned long nowMs) { return live(s.haveCoolant, s.coolantMs, nowMs); }
bool ktmSpeedLive(const KtmState& s, unsigned long nowMs) { return live(s.haveSpeed, s.speedMs, nowMs); }
