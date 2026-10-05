#include "chart_page.h"
#include "dash_logic.h"
#include "history_header.h"

static const uint16_t CL_BG = 0x0000;
static const uint16_t CL_TEXT = 0xFFFF;
static const uint16_t CL_LABEL = 0x8410;
static const uint16_t CL_GRID = 0x2104;

// Plot area: x from PLOT_X for CHART_POINTS pixels, y from PLOT_TOP down
static const int PLOT_X = 36, PLOT_TOP = HEADER_H + 10, PLOT_BOTTOM = 212;
static const int FOOTER_Y = 222;

void ChartPage::begin(const Series& s, const ChartStyle& st, unsigned long sampleIntervalMs,
                      const char* rideLabel, bool canOlder, bool canNewer) {
    series = &s;
    style = st;
    sampleMs = sampleIntervalMs;
    drawnSize = drawnStride = -1;

    tft.fillScreen(CL_BG);
    drawHistoryHeader(tft, style.title, style.color, rideLabel, canOlder, canNewer);
    if (style.overlayLabel) {
        tft.setTextDatum(ML_DATUM);
        tft.setTextColor(style.overlayColor, CL_BG);
        tft.drawString(style.overlayLabel, historyTitleEnd(tft, style.title) + 8, HEADER_H / 2, 2);
    }
    update();
}

void ChartPage::update() {
    if (series->size() == drawnSize && series->samplesPerPoint() == drawnStride) return;
    drawnSize = series->size();
    drawnStride = series->samplesPerPoint();
    drawPlot();
}

void ChartPage::drawPlot() {
    tft.fillRect(0, PLOT_TOP - 8, 320, 240 - (PLOT_TOP - 8), CL_BG);
    tft.drawFastHLine(PLOT_X, PLOT_BOTTOM, CHART_POINTS, CL_GRID);
    tft.drawFastVLine(PLOT_X - 1, PLOT_TOP, PLOT_BOTTOM - PLOT_TOP, CL_GRID);

    int16_t lo, hi;
    bool any = series->range(lo, hi);
    if (style.overlay) {
        int16_t olo, ohi;
        if (style.overlay->range(olo, ohi)) {
            if (!any || olo < lo) lo = olo;
            if (!any || ohi > hi) hi = ohi;
            any = true;
        }
    }

    // Latest value, bottom right
    char buf[24];
    int16_t latest = series->size() ? series->at(series->size() - 1) : CHART_NO_DATA;
    if (latest == CHART_NO_DATA) snprintf(buf, sizeof(buf), "-- %s", style.unit);
    else snprintf(buf, sizeof(buf), "%d %s", latest, style.unit);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(CL_TEXT, CL_BG);
    tft.drawString(buf, 316, FOOTER_Y, 2);

    // Time covered, bottom left
    unsigned long spanMs = (unsigned long)series->size() * series->samplesPerPoint() * sampleMs;
    formatDuration(buf, sizeof(buf), spanMs);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(CL_LABEL, CL_BG);
    tft.drawString(String("last ") + buf, PLOT_X, FOOTER_Y, 2);

    if (!any) {
        tft.setTextDatum(MC_DATUM);
        tft.drawString("No data yet", PLOT_X + CHART_POINTS / 2, (PLOT_TOP + PLOT_BOTTOM) / 2, 2);
        return;
    }

    if (style.fromZero) lo = 0;
    if (hi - lo < style.minSpan) {
        int mid = (hi + lo) / 2;
        lo = style.fromZero ? 0 : mid - style.minSpan / 2;
        hi = lo + style.minSpan;
    }

    tft.setTextDatum(TR_DATUM);
    tft.drawNumber(hi, PLOT_X - 4, PLOT_TOP - 6, 1);
    tft.drawNumber(lo, PLOT_X - 4, PLOT_BOTTOM - 6, 1);

    if (style.overlay) drawLine(*style.overlay, style.overlayColor, lo, hi);
    drawLine(*series, style.color, lo, hi);
}

void ChartPage::drawLine(const Series& s, uint16_t color, int16_t lo, int16_t hi) {
    int span = hi - lo;
    int prevX = -1, prevY = -1;
    for (int i = 0; i < s.size(); i++) {
        int16_t v = s.at(i);
        if (v == CHART_NO_DATA) {
            prevX = -1;  // gap: lift the pen
            continue;
        }
        int x = PLOT_X + i;
        int y = PLOT_BOTTOM - (int)((long)(v - lo) * (PLOT_BOTTOM - PLOT_TOP) / span);
        if (y < PLOT_TOP) y = PLOT_TOP;
        if (y > PLOT_BOTTOM) y = PLOT_BOTTOM;
        if (prevX >= 0) tft.drawLine(prevX, prevY, x, y, color);
        else tft.drawPixel(x, y, color);
        prevX = x;
        prevY = y;
    }
}
