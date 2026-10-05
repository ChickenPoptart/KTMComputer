#include "dash_ui.h"
#include "dash_logic.h"
#include "ride_clock.h"

// RGB565 colours
static const uint16_t CL_BG        = 0x0000;
static const uint16_t CL_TEXT      = 0xFFFF;
static const uint16_t CL_LABEL     = 0x8410; // mid grey
static const uint16_t CL_DIVIDER   = 0x2104;
static const uint16_t CL_COOLANT   = 0x07FF;
static const uint16_t CL_NO_DATA   = 0x4208;
static const uint16_t CL_GOOD      = 0x7FE8;
static const uint16_t CL_BAD       = 0xF800;

// Tach segment colours, left (low RPM) to right, matching the zones in
// dash_logic.h: cyan idle, green riding, yellow shift, red top end. Lit
// colours are fully saturated; unlit are ~12% brightness of the same hue.
static const uint16_t SEG_ON[TACH_SEGMENTS] = {
    0x07FF, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xFFE0, 0xFFE0, 0xF800};
static const uint16_t SEG_OFF[TACH_SEGMENTS] = {
    0x0104, 0x0100, 0x0100, 0x0100, 0x0100, 0x2100, 0x2100, 0x2000};
static const uint16_t SHIFT_ON = 0xF800;
static const uint16_t SHIFT_OFF = 0x0841;

// ---- Top band: gear box, tapered tach, RPM, shift light ----
static const int GEAR_X = 4, GEAR_Y = 6, GEAR_W = 52, GEAR_H = 66, GEAR_R = 9;

static const int SEG_X0 = 62, SEG_W = 20, SEG_GAP = 5, SEG_TOP = 8;
static const int SEG_BOTTOM_LEFT = 66, SEG_BOTTOM_RIGHT = 50;
static const int SEG_X_END = SEG_X0 + TACH_SEGMENTS * (SEG_W + SEG_GAP) - SEG_GAP;

static const int SHIFT_CX = 290, SHIFT_CY = 30, SHIFT_R = 22;
static const int RPM_RIGHT = SEG_X_END, RPM_Y = 58;

// Bar bottoms slope upward left to right, like the reference image
static int segBottomAt(int x) {
    return SEG_BOTTOM_LEFT - (x - SEG_X0) * (SEG_BOTTOM_LEFT - SEG_BOTTOM_RIGHT) / (SEG_X_END - SEG_X0);
}

// ---- Lower area: coolant / elevation | speed | heading / GPS ----
static const int DIVIDER_Y = 92;
static const int LEFT_X = 6, RIGHT_X = 314, CENTER_X = 160;
static const int ROW1_LABEL_Y = 102, ROW1_VALUE_Y = 120;
static const int ROW2_LABEL_Y = 170, ROW2_VALUE_Y = 188;
static const int SPEED_Y = 108;
static const int CLOCK_Y = 210;

static const int SHIFT_BLINK_MS = 80;

void DashUI::begin() {
    drawStatic();
    firstFrame = true;
}

void DashUI::update(const Telemetry& t, unsigned long nowMs) {
    bool blinkOn = (nowMs / SHIFT_BLINK_MS) % 2 == 0;

    drawTach(t.rpm, blinkOn);  // diffs per segment internally

    if (firstFrame || t.gear != last.gear) drawGear(t.gear);
    if (firstFrame || t.rpmLive != last.rpmLive || t.rpm != last.rpm) drawRpm(t);
    if (firstFrame || t.speedLive != last.speedLive || t.speedMph != last.speedMph) drawSpeed(t);
    if (firstFrame || t.coolantLive != last.coolantLive || t.coolantF != last.coolantF) drawCoolant(t);

    bool fixChanged = t.gpsFix != last.gpsFix;
    if (firstFrame || fixChanged || t.headingDeg != last.headingDeg) drawHeading(t);
    if (firstFrame || fixChanged || t.elevationFt != last.elevationFt) drawElevation(t);
    if (firstFrame || fixChanged || t.satellites != last.satellites) drawGpsStatus(t);

    // The clock spot shows CAN FAULT while CAN wiring looks broken
    long clockMinute = t.canFault ? -2 : t.timeValid ? (long)(t.utc / 60) : -1;
    if (firstFrame || clockMinute != lastClockMinute) {
        drawClock(t);
        lastClockMinute = clockMinute;
    }

    last = t;
    firstFrame = false;
}

void DashUI::drawStatic() {
    tft.fillScreen(CL_BG);

    tft.drawRoundRect(GEAR_X, GEAR_Y, GEAR_W, GEAR_H, GEAR_R, CL_TEXT);
    tft.drawRoundRect(GEAR_X + 1, GEAR_Y + 1, GEAR_W - 2, GEAR_H - 2, GEAR_R - 1, CL_TEXT);
    tft.drawRoundRect(GEAR_X + 2, GEAR_Y + 2, GEAR_W - 4, GEAR_H - 4, GEAR_R - 2, CL_TEXT);

    tft.setTextColor(CL_TEXT, CL_BG);
    tft.setTextDatum(BR_DATUM);
    tft.drawString("RPM", RPM_RIGHT - tft.textWidth("8888", 4) - 6, RPM_Y + 22, 2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("SHIFT", SHIFT_CX, SHIFT_CY + SHIFT_R + 6, 2);

    tft.drawFastHLine(0, DIVIDER_Y, 320, CL_DIVIDER);

    tft.setTextColor(CL_LABEL, CL_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("COOLANT", LEFT_X, ROW1_LABEL_Y, 2);
    tft.drawString("ELEV", LEFT_X, ROW2_LABEL_Y, 2);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("HEADING", RIGHT_X, ROW1_LABEL_Y, 2);
    tft.drawString("GPS", RIGHT_X, ROW2_LABEL_Y, 2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("MPH", CENTER_X, SPEED_Y + 80, 2);
}

void DashUI::drawGear(int gear) {
    // Free fonts don't paint a background, so clear the box interior first
    tft.fillRoundRect(GEAR_X + 3, GEAR_Y + 3, GEAR_W - 6, GEAR_H - 6, GEAR_R - 3, CL_BG);
    tft.setFreeFont(&FreeSansBold24pt7b);
    tft.setTextColor(gear == 0 ? CL_GOOD : gear < 0 ? CL_NO_DATA : CL_TEXT);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(gearLabel(gear), GEAR_X + GEAR_W / 2, GEAR_Y + GEAR_H / 2);
    tft.setTextFont(1);
}

void DashUI::drawTach(int rpm, bool blinkOn) {
    int lit = tachLitSegments(rpm);
    bool shifting = shiftActive(rpm);

    for (int i = 0; i < TACH_SEGMENTS; i++) {
        bool on = shifting ? blinkOn : i < lit;
        if (!firstFrame && on == lastSegLit[i]) continue;
        lastSegLit[i] = on;

        int x1 = SEG_X0 + i * (SEG_W + SEG_GAP);
        int x2 = x1 + SEG_W - 1;
        uint16_t c = on ? SEG_ON[i] : SEG_OFF[i];
        // One flat column at a time: a single solid colour with no
        // triangle seams along the tapered bottom edge
        for (int x = x1; x <= x2; x++) {
            tft.drawFastVLine(x, SEG_TOP, segBottomAt(x) - SEG_TOP + 1, c);
        }
    }

    // Shift light stays dark until the shift point, then holds solid red
    // while the bar flashes
    uint16_t shiftColor = shifting ? SHIFT_ON : SHIFT_OFF;

    if (firstFrame || shiftColor != lastShiftColor) {
        tft.fillCircle(SHIFT_CX, SHIFT_CY, SHIFT_R, shiftColor);
        lastShiftColor = shiftColor;
    }
}

void DashUI::drawRpm(const Telemetry& t) {
    tft.setTextColor(t.rpmLive ? CL_TEXT : CL_NO_DATA, CL_BG);
    tft.setTextDatum(TR_DATUM);
    tft.setTextPadding(tft.textWidth("8888", 4));
    tft.drawString(t.rpmLive ? String(t.rpm) : String("--"), RPM_RIGHT, RPM_Y, 4);
    tft.setTextPadding(0);
}

void DashUI::drawSpeed(const Telemetry& t) {
    tft.setTextColor(t.speedLive ? CL_TEXT : CL_NO_DATA, CL_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextPadding(tft.textWidth("888", 8));
    tft.drawString(t.speedLive ? String(t.speedMph) : String("--"), CENTER_X, SPEED_Y, 8);
    tft.setTextPadding(0);
}

void DashUI::drawCoolant(const Telemetry& t) {
    tft.setTextColor(t.coolantLive ? CL_COOLANT : CL_NO_DATA, CL_BG);
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(tft.textWidth("888F", 4));
    tft.drawString(t.coolantLive ? String(t.coolantF) + "F" : String("--"), LEFT_X, ROW1_VALUE_Y, 4);
    tft.setTextPadding(0);
}

void DashUI::drawHeading(const Telemetry& t) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextPadding(tft.textWidth("MMM", 4));
    if (t.gpsFix) {
        tft.setTextColor(CL_TEXT, CL_BG);
        tft.drawString(headingCardinal(t.headingDeg), RIGHT_X, ROW1_VALUE_Y, 4);
    } else {
        tft.setTextColor(CL_NO_DATA, CL_BG);
        tft.drawString("--", RIGHT_X, ROW1_VALUE_Y, 4);
    }

    tft.setTextColor(CL_LABEL, CL_BG);
    tft.setTextPadding(tft.textWidth("888", 2));
    tft.drawString(t.gpsFix ? String(t.headingDeg) : String(""), RIGHT_X - 6, ROW1_VALUE_Y + 28, 2);
    tft.setTextPadding(0);
    // Degree mark
    tft.fillRect(RIGHT_X - 4, ROW1_VALUE_Y + 28, 5, 5, CL_BG);
    if (t.gpsFix) tft.drawCircle(RIGHT_X - 2, ROW1_VALUE_Y + 30, 2, CL_LABEL);
}

void DashUI::drawElevation(const Telemetry& t) {
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(tft.textWidth("88888", 4));
    if (t.gpsFix) {
        tft.setTextColor(CL_TEXT, CL_BG);
        tft.drawNumber(t.elevationFt, LEFT_X, ROW2_VALUE_Y, 4);
    } else {
        tft.setTextColor(CL_NO_DATA, CL_BG);
        tft.drawString("--", LEFT_X, ROW2_VALUE_Y, 4);
    }
    tft.setTextPadding(0);
    tft.setTextColor(CL_LABEL, CL_BG);
    tft.drawString("FT", LEFT_X, ROW2_VALUE_Y + 28, 2);
}

void DashUI::drawClock(const Telemetry& t) {
    char buf[16] = "--:--";
    uint16_t color = CL_NO_DATA;
    if (t.canFault) {
        snprintf(buf, sizeof(buf), "CAN FAULT");
        color = CL_BAD;
    } else if (t.timeValid) {
        formatClock(buf, sizeof(buf), t.utc);
        color = CL_TEXT;
    }
    tft.setTextColor(color, CL_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextPadding(tft.textWidth("CAN FAULT", 4));
    tft.drawString(buf, CENTER_X, CLOCK_Y, 4);
    tft.setTextPadding(0);
}

void DashUI::drawGpsStatus(const Telemetry& t) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextPadding(tft.textWidth("88", 4));
    tft.setTextColor(t.gpsFix ? CL_TEXT : CL_NO_DATA, CL_BG);
    tft.drawNumber(t.satellites, RIGHT_X, ROW2_VALUE_Y, 4);
    tft.setTextPadding(tft.textWidth("NO FIX", 2));
    tft.setTextColor(t.gpsFix ? CL_GOOD : CL_BAD, CL_BG);
    tft.drawString(t.gpsFix ? "SATS" : "NO FIX", RIGHT_X, ROW2_VALUE_Y + 28, 2);
    tft.setTextPadding(0);
}
