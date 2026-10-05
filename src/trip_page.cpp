#include "trip_page.h"
#include "dash_logic.h"
#include "history_header.h"

static const uint16_t CL_BG = 0x0000;
static const uint16_t CL_TEXT = 0xFFFF;
static const uint16_t CL_LABEL = 0x8410;
static const uint16_t CL_DIVIDER = 0x2104;

static const int GRID_Y = HEADER_H + 6, ROW_H = 52;
static const int COL_X[2] = {8, 168};

static const char* const LABELS[4][2] = {
    {"DISTANCE MI", "MAX SPEED MPH"},
    {"RIDE TIME", "MOVING TIME"},
    {"MAX RPM", "MAX COOLANT F"},
    {"CLIMB FT", "DESCENT FT"},
};

void TripPage::begin(const char* rideLabel, bool canOlder, bool canNewer) {
    tft.fillScreen(CL_BG);
    drawHistoryHeader(tft, "TRIP", CL_TEXT, rideLabel, canOlder, canNewer);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CL_LABEL, CL_BG);
    for (int r = 0; r < 4; r++) {
        int y = GRID_Y + r * ROW_H;
        tft.drawFastHLine(0, y - 4, 320, CL_DIVIDER);
        for (int c = 0; c < 2; c++) tft.drawString(LABELS[r][c], COL_X[c], y, 2);
    }
    tft.drawFastVLine(160, GRID_Y - 4, 4 * ROW_H, CL_DIVIDER);
}

void TripPage::drawCell(int col, int row, const char* value) {
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CL_TEXT, CL_BG);
    tft.setTextPadding(148);
    tft.drawString(value, COL_X[col], GRID_Y + row * ROW_H + 18, 4);
    tft.setTextPadding(0);
}

void TripPage::update(const TripStats& trip) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", trip.miles());
    drawCell(0, 0, buf);
    snprintf(buf, sizeof(buf), "%d", trip.maxSpeedMph());
    drawCell(1, 0, buf);
    formatDuration(buf, sizeof(buf), trip.elapsedMs());
    drawCell(0, 1, buf);
    formatDuration(buf, sizeof(buf), trip.movingMs());
    drawCell(1, 1, buf);
    snprintf(buf, sizeof(buf), "%d", trip.maxRpm());
    drawCell(0, 2, buf);
    if (trip.maxCoolantF() > 0) snprintf(buf, sizeof(buf), "%d", trip.maxCoolantF());
    else snprintf(buf, sizeof(buf), "--");
    drawCell(1, 2, buf);
    snprintf(buf, sizeof(buf), "%d", trip.gainFt());
    drawCell(0, 3, buf);
    snprintf(buf, sizeof(buf), "%d", trip.lossFt());
    drawCell(1, 3, buf);
}
