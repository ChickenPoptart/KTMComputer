#include "zones_page.h"
#include "dash_logic.h"
#include "history_header.h"

static const uint16_t CL_BG = 0x0000;
static const uint16_t CL_TEXT = 0xFFFF;
static const uint16_t CL_LABEL = 0x8410;
static const uint16_t CL_TRACK = 0x2104;

// Same colours as the tach bar
static const TachZone ZONES[4] = {ZONE_IDLE, ZONE_GREEN, ZONE_YELLOW, ZONE_RED};
static const char* const NAMES[4] = {"IDLE", "GREEN", "SHIFT", "TOP END"};
static const uint16_t COLORS[4] = {0x07FF, 0x07E0, 0xFFE0, 0xF800};

static const int ROW_Y0 = 36, ROW_H = 46;
static const int BAR_X = 8, BAR_W = 304, BAR_H = 14;

void ZonesPage::begin(const char* rideLabel, bool canOlder, bool canNewer) {
    tft.fillScreen(CL_BG);
    drawHistoryHeader(tft, "RPM ZONES", CL_TEXT, rideLabel, canOlder, canNewer);
    tft.setTextDatum(TL_DATUM);
    for (int i = 0; i < 4; i++) {
        tft.setTextColor(COLORS[i], CL_BG);
        tft.drawString(NAMES[i], BAR_X, ROW_Y0 + i * ROW_H, 2);
    }
}

void ZonesPage::update(const TripStats& trip) {
    uint32_t total = 0;
    for (TachZone z : ZONES) total += trip.zoneMs(z);

    char buf[24], dur[16];
    for (int i = 0; i < 4; i++) {
        int y = ROW_Y0 + i * ROW_H;
        uint32_t ms = trip.zoneMs(ZONES[i]);
        int pct = total ? (int)((uint64_t)ms * 100 / total) : 0;

        formatDuration(dur, sizeof(dur), ms);
        snprintf(buf, sizeof(buf), "%s  %d%%", dur, pct);
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(CL_TEXT, CL_BG);
        tft.setTextPadding(120);
        tft.drawString(buf, BAR_X + BAR_W, y, 2);
        tft.setTextPadding(0);

        int w = total ? (int)((uint64_t)ms * BAR_W / total) : 0;
        tft.fillRect(BAR_X, y + 20, w, BAR_H, COLORS[i]);
        tft.fillRect(BAR_X + w, y + 20, BAR_W - w, BAR_H, CL_TRACK);
    }
}
