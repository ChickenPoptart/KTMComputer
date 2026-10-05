#pragma once

#include <TFT_eSPI.h>
#include "can_bus.h"
#include "can_tracker.h"
#include "sd_log.h"

// Live table of every CAN ID seen: rate plus the 8 data bytes. Bytes that
// changed in the last second are yellow, bytes that have ever changed are
// white, bytes that never changed are grey.
class CanPage {
public:
    static const int ROWS = 12;

    CanPage(TFT_eSPI& tft, const CanTracker& tracker) : tft(tft), tracker(tracker) {}

    // Shows IDs starting at firstRow; pageNum/pageCount label the header.
    void begin(int firstRow, int pageNum, int pageCount);
    void update(const CanBusStatus& status, const SdLogStatus& sd, unsigned long nowMs);

private:
    void drawHeader(const CanBusStatus& status);
    void drawSdFooter(const SdLogStatus& sd);
    void drawRow(int screenRow, const CanEntry& e, unsigned long nowMs);

    TFT_eSPI& tft;
    const CanTracker& tracker;
    int firstRow = 0;
    int pageNum = 1, pageCount = 1;
    bool showingWaiting = false;
};
