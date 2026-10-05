#pragma once

#include <stdint.h>
#include "can_tracker.h"
#include "ktm_decode.h"

struct CanBusStatus {
    bool running = false;
    const char* state = "OFF";
    uint32_t rxMissed = 0;     // frames lost because the RX queue was full
    uint32_t busErrors = 0;
    bool faulted = false;      // error flood seen; CAN paused or being rechecked
};

// Starts the ESP32 TWAI controller at 500 kbps in LISTEN-ONLY mode: it never
// sends frames or ACKs. Caveat: on the ESP32 a listen-only controller can
// still drive error flags when it sees bus errors (chip errata; the
// workaround isn't enabled in this framework build). For a guaranteed
// read-only tap, the transceiver's TX input (CTX/D) should be tied to 3.3 V
// instead of IO22. If errors flood in, CAN is paused (see can_guard.h).
bool canBegin();

// Drains received frames into the tracker and the KTM decoder, and queues
// every frame for the SD card log in candump format:
// (seconds.micros) can0 ID#DATA
// Frames are not echoed to Serial: at 115200 baud it can't keep up with
// the bus, and writes then block the loop long enough to lose frames.
void canPoll(CanTracker& tracker, KtmState& ktm, unsigned long nowMs);

CanBusStatus canStatus();
