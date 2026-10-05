#include "can_page.h"

static const uint16_t CL_BG       = 0x0000;
static const uint16_t CL_TEXT     = 0xFFFF;
static const uint16_t CL_LABEL    = 0x8410;
static const uint16_t CL_CONSTANT = 0x4208;
static const uint16_t CL_CHANGED  = 0xFFE0;
static const uint16_t CL_ID       = 0x07FF;
static const uint16_t CL_BAD      = 0xF800;
static const uint16_t CL_DIVIDER  = 0x2104;

static const int HEADER_Y = 2;
static const int COLS_Y = 20;
static const int TABLE_Y = 36;
static const int ROW_H = 15;
static const int FOOTER_Y = 224;

static const int ID_X = 4;
static const int HZ_RIGHT = 80;
static const int BYTE_X0 = 94, BYTE_PITCH = 28;

static const unsigned long RECENT_MS = 1000;

void CanPage::begin(int first, int num, int count) {
    firstRow = first;
    pageNum = num;
    pageCount = count;
    showingWaiting = false;

    tft.fillScreen(CL_BG);
    tft.setTextColor(CL_LABEL, CL_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("ID", ID_X, COLS_Y, 2);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("HZ", HZ_RIGHT, COLS_Y, 2);
    tft.setTextDatum(TL_DATUM);
    for (int i = 0; i < 8; i++) tft.drawNumber(i, BYTE_X0 + i * BYTE_PITCH + 4, COLS_Y, 2);
    tft.drawFastHLine(0, TABLE_Y - 2, 320, CL_DIVIDER);
}

void CanPage::update(const CanBusStatus& status, const SdLogStatus& sd, unsigned long nowMs) {
    drawHeader(status);
    drawSdFooter(sd);

    if (tracker.size() == 0) {
        if (!showingWaiting) {
            tft.setTextColor(CL_LABEL, CL_BG);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Waiting for CAN frames...", 160, 120, 2);
            tft.drawString("Ignition on? CAN-H/L wired?", 160, 140, 2);
            showingWaiting = true;
        }
        return;
    }
    if (showingWaiting) {
        tft.fillRect(0, TABLE_Y, 320, FOOTER_Y - TABLE_Y, CL_BG);
        showingWaiting = false;
    }

    for (int r = 0; r < ROWS && firstRow + r < tracker.size(); r++) {
        drawRow(r, tracker.at(firstRow + r), nowMs);
    }
}

void CanPage::drawHeader(const CanBusStatus& s) {
    char buf[48];
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CL_TEXT, CL_BG);
    tft.setTextPadding(150);
    snprintf(buf, sizeof(buf), "CAN %d/%d  %d IDs", pageNum, pageCount, tracker.size());
    tft.drawString(buf, ID_X, HEADER_Y, 2);

    bool healthy = s.running && s.rxMissed == 0 && s.busErrors == 0;
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(healthy ? CL_LABEL : CL_BAD, CL_BG);
    tft.setTextPadding(160);
    if (!s.running && s.faulted) snprintf(buf, sizeof(buf), "CAN FAULT - check wiring");
    else if (!s.running) snprintf(buf, sizeof(buf), "TWAI FAILED TO START");
    else snprintf(buf, sizeof(buf), "%s %lu/s miss %lu err %lu", s.state,
                  (unsigned long)tracker.totalHz(), (unsigned long)s.rxMissed, (unsigned long)s.busErrors);
    tft.drawString(buf, 316, HEADER_Y, 2);
    tft.setTextPadding(0);
}

void CanPage::drawSdFooter(const SdLogStatus& sd) {
    char buf[64];
    bool healthy = sd.recording && sd.droppedLines == 0 && sd.writeErrors == 0;
    if (!sd.recording) snprintf(buf, sizeof(buf), "SD: no card - not recording");
    else snprintf(buf, sizeof(buf), "REC %s  %lu KB  drop %lu  err %lu", sd.fileName + 1,
                  (unsigned long)(sd.bytesWritten / 1024), (unsigned long)sd.droppedLines,
                  (unsigned long)sd.writeErrors);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(healthy ? CL_LABEL : CL_BAD, CL_BG);
    tft.setTextPadding(312);
    tft.drawString(buf, ID_X, FOOTER_Y, 2);
    tft.setTextPadding(0);
}

void CanPage::drawRow(int screenRow, const CanEntry& e, unsigned long nowMs) {
    int y = TABLE_Y + screenRow * ROW_H;
    char buf[12];

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CL_ID, CL_BG);
    if (e.extended) {
        snprintf(buf, sizeof(buf), "%08lX", (unsigned long)e.id);
        tft.setTextPadding(52);
        tft.drawString(buf, ID_X, y + 4, 1);  // small font so 29-bit IDs fit
    } else {
        snprintf(buf, sizeof(buf), "%03lX", (unsigned long)e.id);
        tft.setTextPadding(40);
        tft.drawString(buf, ID_X, y, 2);
    }

    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(CL_LABEL, CL_BG);
    tft.setTextPadding(24);
    tft.drawNumber(e.hz, HZ_RIGHT, y, 2);

    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(BYTE_PITCH - 4);
    for (int i = 0; i < 8; i++) {
        int x = BYTE_X0 + i * BYTE_PITCH;
        if (i >= e.len) {
            tft.drawString("", x, y, 2);
            continue;
        }
        uint16_t c = e.byteRecentlyChanged(i, nowMs, RECENT_MS) ? CL_CHANGED
                   : (e.everChangedMask & (1 << i))              ? CL_TEXT
                                                                  : CL_CONSTANT;
        tft.setTextColor(c, CL_BG);
        snprintf(buf, sizeof(buf), "%02X", e.data[i]);
        tft.drawString(buf, x, y, 2);
    }
    tft.setTextPadding(0);
}
