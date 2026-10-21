// Bring-up test for the 5" board: display, backlight, touch, SD logging
// and CAN. Shows colour bars, a dot wherever the screen is touched, and a
// live status line for the SD card and the CAN bus.

#include <Arduino.h>
#include <Wire.h>
#include "board_w5.h"
#include "can_bus.h"
#include "can_tracker.h"
#include "gps.h"
#include "ktm_decode.h"
#include "sd_log.h"
#include "ch422g.h"
#include "gt911.h"
#include "lgfx_w5.h"

static LGFX_W5 lcd;
static CanTracker canTracker;
static KtmState ktm;

static void drawTestScreen() {
    lcd.fillScreen(TFT_BLACK);

    // Colour bars across the top: checks every colour channel is wired
    const uint16_t bars[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA,
                             TFT_YELLOW, TFT_WHITE, TFT_DARKGREY};
    const int n = sizeof(bars) / sizeof(bars[0]);
    for (int i = 0; i < n; i++) lcd.fillRect(i * W5_WIDTH / n, 0, W5_WIDTH / n, 80, bars[i]);

    // The tach bar colours, as they'll appear on the dash
    const uint16_t tach[] = {0x07FF, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xFFE0, 0xFFE0, 0xF800};
    for (int i = 0; i < 8; i++) lcd.fillRect(40 + i * 90, 110, 80, 60, tach[i]);

    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextDatum(textdatum_t::middle_center);
    lcd.setFont(&fonts::Font4);
    lcd.drawString("KTM dash - 5\" bring-up", W5_WIDTH / 2, 210);
    lcd.setFont(&fonts::Font7);
    lcd.setTextSize(2);
    lcd.drawString("8888", W5_WIDTH / 2, 320);
    lcd.setTextSize(1);
    lcd.setFont(&fonts::Font2);
    lcd.drawString("Touch anywhere: a dot should appear under your finger", W5_WIDTH / 2, 440);
}

void setup() {
    Serial.begin(115200);
    delay(1500);  // let the USB serial port come up
    Serial.println("# 5\" bring-up");

    bool expander = ch422gBegin();
    Serial.printf("# CH422G %s\n", expander ? "ok" : "NOT RESPONDING");

    // Everything answering on the I2C bus (expected: 0x23/0x24/0x38 CH422G,
    // 0x51 RTC, 0x5D touch, 0x4D GPS bridge)
    Serial.print("# I2C devices:");
    for (uint8_t a = 0x08; a < 0x78; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) Serial.printf(" 0x%02X", a);
    }
    Serial.println();

    // Panel reset, then backlight on
    ch422gWrite(EXIO_LCD_RST, false);
    delay(10);
    ch422gWrite(EXIO_LCD_RST, true);
    delay(100);

    bool display = lcd.init();
    Serial.printf("# display %s, %dx%d, PSRAM free %u KB\n", display ? "ok" : "FAILED",
                  lcd.width(), lcd.height(), (unsigned)(ESP.getFreePsram() / 1024));
    ch422gWrite(EXIO_BACKLIGHT, true);
    drawTestScreen();

    bool touch = gt911Begin();
    Serial.printf("# touch %s\n", touch ? "ok (GT911)" : "NOT RESPONDING");

    ch422gWrite(EXIO_SD_CS, true);  // SD mode: the card's D3 line must be high
    if (sdLogBegin()) Serial.printf("# SD recording to %s\n", sdLogStatus().fileName);
    else Serial.println("# SD: no card");
    Serial.printf("# CAN %s\n", canBegin() ? "listen-only 500k" : "FAILED TO START");
    gpsBegin();
}

// One status line at the bottom of the screen, twice a second
static void drawStatus(unsigned long nowMs) {
    static unsigned long lastMs = 0;
    if (nowMs - lastMs < 500) return;
    lastMs = nowMs;

    CanBusStatus can = canStatus();
    SdLogStatus sd = sdLogStatus();
    char line[160];
    int n = snprintf(line, sizeof(line), "SD %s %luKB   CAN %s %lu/s err %lu",
                     sd.recording ? sd.fileName + 1 : "none", (unsigned long)(sd.bytesWritten / 1024),
                     can.state, (unsigned long)canTracker.totalHz(), (unsigned long)can.busErrors);
    GpsReading g = gpsReading();
    n += snprintf(line + n, sizeof(line) - n, "   GPS chars %lu sats %d %s", (unsigned long)gpsCharsReceived(),
                  g.satellites, g.locationValid ? "FIX" : "no fix");
    if (ktmRpmLive(ktm, nowMs)) {
        snprintf(line + n, sizeof(line) - n, "   %d rpm  gear %d  %dF", ktm.rpm, ktm.gear, ktm.coolantF);
    }
    lcd.setFont(&fonts::Font2);
    lcd.setTextSize(1);
    lcd.setTextDatum(textdatum_t::middle_center);
    lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    lcd.setTextPadding(W5_WIDTH);
    lcd.drawString(line, W5_WIDTH / 2, 400);
    lcd.setTextPadding(0);
}

void loop() {
    unsigned long now = millis();
    canPoll(canTracker, ktm, now);
    canTracker.updateRates(now);
    gpsPoll();
    drawStatus(now);

    int x, y;
    if (gt911Read(x, y)) {
        lcd.fillCircle(x, y, 8, TFT_YELLOW);
        Serial.printf("# touch x=%d y=%d\n", x, y);
    }
    delay(5);
}
