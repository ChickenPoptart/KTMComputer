#include "history_header.h"

static const uint16_t CL_BG = 0x0000;
static const uint16_t CL_LABEL = 0x8410;
static const uint16_t CL_BUTTON = 0xFFFF;
static const uint16_t CL_BUTTON_OFF = 0x2104;

static const int TITLE_X = NAV_W + 8;

static void drawButton(TFT_eSPI& tft, int x, const char* label, bool enabled) {
    uint16_t c = enabled ? CL_BUTTON : CL_BUTTON_OFF;
    tft.drawRoundRect(x, 1, NAV_W, HEADER_H - 2, 4, c);
    tft.setTextColor(c, CL_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(label, x + NAV_W / 2, HEADER_H / 2, 2);
}

void drawHistoryHeader(TFT_eSPI& tft, const char* title, uint16_t titleColor,
                       const char* rideLabel, bool canOlder, bool canNewer) {
    tft.fillRect(0, 0, 320, HEADER_H, CL_BG);
    drawButton(tft, 0, "<", canOlder);
    drawButton(tft, 320 - NAV_W, ">", canNewer);

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(titleColor, CL_BG);
    tft.drawString(title, TITLE_X, HEADER_H / 2, 2);

    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(CL_LABEL, CL_BG);
    tft.drawString(rideLabel, 320 - NAV_W - 6, HEADER_H / 2, 2);
}

int historyTitleEnd(TFT_eSPI& tft, const char* title) {
    return TITLE_X + tft.textWidth(title, 2);
}
