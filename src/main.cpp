#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_rom_gpio.h>
#include <esp_timer.h>
#include <soc/gpio_pins.h>
#include <soc/gpio_sig_map.h>
#include <sys/time.h>
#include "can_bus.h"
#include "candump_format.h"
#include "can_page.h"
#include "can_tracker.h"
#include "chart_page.h"
#include "dash_ui.h"
#include "gps.h"
#include "history_header.h"
#include "ktm_decode.h"
#include "loop_profiler.h"
#include "press_detector.h"
#include "reset_prompt.h"
#include "ride_history.h"
#include "ride_store.h"
#include "ride_clock.h"
#include "sd_log.h"
#include "tap_sequencer.h"
#include "touch.h"
#include "touch_map.h"
#include "trip_page.h"
#include "trip_stats.h"
#include "zones_page.h"

// 1 or 3 depending on which way the board is mounted (USB port left/right)
static const int SCREEN_ROTATION = 1;

static const unsigned long DASH_FRAME_MS = 40;
static const unsigned long CAN_PAGE_FRAME_MS = 250;
static const unsigned long STATS_PAGE_FRAME_MS = 500;
static const unsigned long GPS_STATUS_MS = 5000;
static const unsigned long TRIP_TICK_MS = 100;
static const unsigned long CHART_SAMPLE_MS = 2000;
static const unsigned long CLOCK_SYNC_MS = 60000;
// Turning the key off loses at most this much of a ride's history
static const unsigned long RIDE_SAVE_MS = 30000;
// How often the loop timing breakdown goes into the log
static const unsigned long PROFILE_REPORT_MS = 10000;

TFT_eSPI tft = TFT_eSPI();
DashUI dash(tft);
CanTracker canTracker;
CanPage canPage(tft, canTracker);
TripPage tripPage(tft);
ChartPage chartPage(tft);
ZonesPage zonesPage(tft);
PressDetector press;
LoopProfiler prof;
TapSequencer taps;
ResetPrompt resetPrompt;
Telemetry telemetry;
KtmState ktm;
GpsView gpsView;

// This ride, collected all the time whatever page is showing, and a saved
// past ride loaded for the history pages' < > buttons
RideRecord live, past;
RideBrowser rides;

// Fixed pages first, then the CAN ID list, CanPage::ROWS IDs per page
enum Page { PAGE_DASH, PAGE_TRIP, PAGE_ELEVATION, PAGE_SPEED, PAGE_ZONES, PAGE_COOLANT, FIRST_CAN_PAGE };

static int currentPage = PAGE_DASH;
static bool clockSet = false;  // set once GPS time arrives (syncClock)
static unsigned long lastFrameMs = 0;

static int canPageCount() {
    int n = canTracker.size();
    return n <= CanPage::ROWS ? 1 : (n + CanPage::ROWS - 1) / CanPage::ROWS;
}

static bool isStatsPage(int page) { return page >= PAGE_TRIP && page < FIRST_CAN_PAGE; }

// The ride the history pages are showing
static const RideRecord& shownRide() { return rides.viewingLive() ? live : past; }

// "THIS RIDE", or "#12 Sat Oct 3" for a saved one
static void rideLabel(char* out, size_t cap) {
    if (rides.viewingLive()) {
        snprintf(out, cap, "THIS RIDE");
        return;
    }
    char date[16] = "";
    if (past.startUtc) formatDate(date, sizeof(date), past.startUtc);
    snprintf(out, cap, "#%d %s", past.number, date);
}

static void showPage(int page) {
    currentPage = page;
    const RideRecord& r = shownRide();
    char label[24];
    rideLabel(label, sizeof(label));
    bool older = rides.hasOlder(), newer = rides.hasNewer();

    switch (page) {
    case PAGE_DASH: dash.begin(); break;
    case PAGE_TRIP: tripPage.begin(label, older, newer); break;
    case PAGE_ZONES: zonesPage.begin(label, older, newer); break;
    case PAGE_ELEVATION:
        chartPage.begin(r.elevation, {"ELEVATION", "ft", TFT_CYAN, 100, false}, CHART_SAMPLE_MS,
                        label, older, newer);
        break;
    case PAGE_SPEED: {
        ChartStyle style{"SPEED", "mph", TFT_WHITE, 20, true};
        style.overlay = &r.gpsSpeed;
        style.overlayColor = TFT_DARKGREEN;
        style.overlayLabel = "GPS";
        chartPage.begin(r.speed, style, CHART_SAMPLE_MS, label, older, newer);
        break;
    }
    case PAGE_COOLANT:
        chartPage.begin(r.coolant, {"COOLANT", "F", TFT_CYAN, 40, false}, CHART_SAMPLE_MS,
                        label, older, newer);
        break;
    default: {
        int canIndex = page - FIRST_CAN_PAGE;
        canPage.begin(canIndex * CanPage::ROWS, canIndex + 1, canPageCount());
    }
    }
    lastFrameMs = 0;  // draw immediately
}

static void nextPage() {
    showPage((currentPage + 1) % (FIRST_CAN_PAGE + canPageCount()));
}

// ---- Event markers: long press on the dash or CAN pages ----
static const unsigned long MARK_BANNER_MS = 1500;
static uint16_t markCount = 0;
static unsigned long markBannerUntilMs = 0;

static void dropMarker(unsigned long nowMs) {
    markCount++;
    char line[CANDUMP_LINE_MAX];
    int n = formatMarkerLine(line, sizeof(line), esp_timer_get_time(), markCount);
    sdLogWrite(line, n);
    Serial.write((const uint8_t*)line, n);  // rare, so a brief block is harmless

    tft.fillRect(90, 96, 140, 48, TFT_YELLOW);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("MARK " + String(markCount), 160, 120, 4);
    markBannerUntilMs = nowMs + MARK_BANNER_MS;
}

// ---- Reset: long press on a trip/chart page, then tap to confirm ----
static bool promptShown = false;

static void showResetPrompt(unsigned long nowMs) {
    resetPrompt.open(nowMs);
    promptShown = true;
    tft.fillRoundRect(50, 80, 220, 80, 8, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("TAP TO CLEAR", 160, 108, 4);
    tft.drawString("wait to cancel", 160, 140, 2);
}

static void resetCurrentPage() {
    switch (currentPage) {
    case PAGE_TRIP: live.trip = TripStats(); break;
    case PAGE_ZONES: live.trip.clearZones(); break;
    case PAGE_ELEVATION: live.elevation = Series(); break;
    case PAGE_SPEED: live.speed = Series(); live.gpsSpeed = Series(); break;
    case PAGE_COOLANT: live.coolant = Series(); break;
    }
}

// ---- Ride history: < older, > newer on the trip/chart/zones pages ----
static void stepRide(bool toOlder) {
    bool moved = toOlder ? rides.older() : rides.newer();
    if (!moved) return;
    // A file that can't be read (e.g. from older firmware) is skipped
    while (!rides.viewingLive() && !rideLoad(rides.viewing(), past)) {
        if (!(toOlder ? rides.older() : rides.newer())) {
            rides.showLive();
            break;
        }
    }
    showPage(currentPage);
}

static void saveRide(unsigned long nowMs) {
    static unsigned long lastSaveMs = 0;
    if (nowMs - lastSaveMs < RIDE_SAVE_MS) return;
    lastSaveMs = nowMs;
    if (!worthSaving(live.trip) || sdLogNumber() < 0) return;
    live.number = sdLogNumber();
    if (live.startUtc == 0 && clockSet) live.startUtc = time(nullptr) - live.trip.elapsedMs() / 1000;
    if (!rideSave(live)) Serial.println("# Could not save ride history");
}

// ---- Clock: set from GPS time; also gives SD log files real dates ----

static void syncClock(unsigned long nowMs) {
    static unsigned long lastSyncMs = 0;
    if (clockSet && nowMs - lastSyncMs < CLOCK_SYNC_MS) return;
    time_t utc;
    if (!gpsUtcTime(utc)) return;
    struct timeval tv = {utc, 0};
    settimeofday(&tv, nullptr);
    clockSet = true;
    lastSyncMs = nowMs;
}

// Engine values from the bike; "--" on the dash when their frames stop
static void updateTelemetry(Telemetry& t, const KtmState& k, unsigned long nowMs) {
    t.rpmLive = ktmRpmLive(k, nowMs);
    t.rpm = t.rpmLive ? k.rpm : 0;
    t.gear = ktmGearLive(k, nowMs) ? k.gear : -1;
    t.coolantLive = ktmCoolantLive(k, nowMs);
    t.coolantF = k.coolantF;
    t.speedLive = ktmSpeedLive(k, nowMs);
    t.speedMph = k.speedMph;

    t.canFault = canStatus().faulted;
    t.timeValid = clockSet;
    t.utc = time(nullptr);

    gpsUpdateView(gpsView, gpsReading());
    t.gpsFix = gpsView.fix;
    t.satellites = gpsView.satellites;
    t.headingDeg = gpsView.headingDeg;
    t.elevationFt = gpsView.elevationFt;
}

static void collectStats(const Telemetry& t, uint32_t dtMs) {
    TripSample s;
    s.speedLive = t.speedLive;
    s.speedMph = t.speedMph;
    s.rpmLive = t.rpmLive;
    s.rpm = t.rpm;
    s.coolantLive = t.coolantLive;
    s.coolantF = t.coolantF;
    s.elevationValid = t.gpsFix;
    s.elevationFt = t.elevationFt;
    live.trip.update(dtMs, s);
}

static void sampleCharts(const Telemetry& t) {
    live.elevation.add(t.gpsFix ? t.elevationFt : CHART_NO_DATA);
    live.speed.add(t.speedLive ? t.speedMph : CHART_NO_DATA);
    live.gpsSpeed.add(t.gpsFix ? gpsView.speedMph : CHART_NO_DATA);
    live.coolant.add(t.coolantLive ? t.coolantF : CHART_NO_DATA);
}

void setup() {
    Serial.setTxBufferSize(1024);  // only status lines and markers now
    // Transmit only. Powered through VIN with no USB cable, the CYD's USB-serial
    // chip is unpowered and can hold the ESP32's receive pin low, which looks
    // like an endless stream of broken characters, each one an interrupt.
    // Nothing reads Serial, so its receive pin (GPIO 3) isn't connected at all.
    // Flashing doesn't depend on this; the ROM bootloader handles it.
    Serial.begin(115200, SERIAL_8N1, -1, 1);
    // UART0's receive input is also wired to GPIO 3 by default inside the
    // chip; feed it a constant idle-high instead and leave the pin unused.
    pinMode(3, INPUT_PULLUP);
    esp_rom_gpio_connect_in_signal(GPIO_MATRIX_CONST_ONE_INPUT, U0RXD_IN_IDX, false);
    setenv("TZ", MOUNTAIN_TZ, 1);
    tzset();

    tft.init();
    tft.invertDisplay(true); // this CYD panel needs inversion for correct colours
    tft.setRotation(SCREEN_ROTATION);
    touchBegin();

    if (sdLogBegin()) Serial.printf("# Recording all CAN frames to SD %s\n", sdLogStatus().fileName);
    else Serial.println("# No SD card: not recording");
    if (!canBegin()) Serial.println("# TWAI failed to start");
    gpsBegin();

    static int saved[RideBrowser::CAPACITY];
    int n = rideList(saved, RideBrowser::CAPACITY);
    rides.setRides(saved, n, sdLogNumber());
    Serial.printf("# %d past rides on the card\n", n);

    char note[96];
    snprintf(note, sizeof(note), "boot reset_reason=%d can=%s sd=%s", (int)esp_reset_reason(),
             canStatus().running ? "running" : "NOT RUNNING", sdLogNumber() >= 0 ? "ok" : "none");
    sdLogNote(note);

    showPage(PAGE_DASH);
}

void loop() {
    unsigned long now = millis();
    prof.startPass(micros());
    static unsigned long lastProfileMs = 0;
    if (now - lastProfileMs >= PROFILE_REPORT_MS) {
        lastProfileMs = now;
        char report[200], note[240];
        prof.report(report, sizeof(report));
        SdLogStatus sd = sdLogStatus();
        snprintf(note, sizeof(note), "%s sd_kb=%lu sd_dropped=%lu sd_errors=%lu", report,
                 (unsigned long)(sd.bytesWritten / 1024), (unsigned long)sd.droppedLines,
                 (unsigned long)sd.writeErrors);
        sdLogNote(note);
    }

    // Keep the CAN queue drained every pass, whatever page is showing
    canPoll(canTracker, ktm, now);
    canTracker.updateRates(now);
    prof.mark(PROF_CAN, micros());
    gpsPoll();
    syncClock(now);
    prof.mark(PROF_GPS, micros());

    // Trip stats and charts keep collecting on every page
    static unsigned long lastTripMs = now, lastChartMs = now;
    if (now - lastTripMs >= TRIP_TICK_MS) {
        updateTelemetry(telemetry, ktm, now);
        collectStats(telemetry, now - lastTripMs);
        lastTripMs = now;
    }
    if (now - lastChartMs >= CHART_SAMPLE_MS) {
        sampleCharts(telemetry);
        lastChartMs += CHART_SAMPLE_MS;
    }
    saveRide(now);
    prof.mark(PROF_STATS, micros());

    // GPS status for bench checks over USB, every few seconds
    static unsigned long lastGpsStatusMs = 0;
    if (now - lastGpsStatusMs >= GPS_STATUS_MS) {
        lastGpsStatusMs = now;
        char clock[16] = "--:--", date[16] = "";
        if (clockSet) {
            formatClock(clock, sizeof(clock), time(nullptr));
            formatDate(date, sizeof(date), time(nullptr));
        }
        Serial.printf("# GPS chars=%lu sats=%d fix=%d time=%s %s\n", (unsigned long)gpsCharsReceived(),
                      gpsView.satellites, gpsView.fix, date, clock);
    }
    prof.mark(PROF_SERIAL, micros());

    // Where the screen is being touched, kept from while the finger was down
    static int touchRawX = 0, touchRawY = 0;
    touchReadRaw(touchRawX, touchRawY);

    PressEvent ev = press.update(touchIsPressed(), now);
    prof.mark(PROF_TOUCH, micros());
    if (ev == PRESS_TAP) {
        int x, y;
        touchToScreen(touchRawX, touchRawY, x, y);
        taps.tap(now, x, y);
    }
    if (ev == PRESS_LONG) {
        // Saved rides can't be reset; long press does nothing there
        if (!isStatsPage(currentPage)) dropMarker(now);
        else if (rides.viewingLive()) showResetPrompt(now);
    }

    TapResult tap = taps.poll(now);
    if (tap.kind == TAP_DOUBLE) {
        // Double tap anywhere: back to the dashboard and this ride
        resetPrompt.confirm(0);  // closes it without resetting
        promptShown = false;
        rides.showLive();
        showPage(PAGE_DASH);
    } else if (tap.kind == TAP_SINGLE) {
        if (resetPrompt.isOpen(now)) {
            if (resetPrompt.confirm(now)) resetCurrentPage();
            promptShown = false;
            showPage(currentPage);
        } else if (isStatsPage(currentPage) && tapHitsOlder(tap.x, tap.y)) {
            stepRide(true);
        } else if (isStatsPage(currentPage) && tapHitsNewer(tap.x, tap.y)) {
            stepRide(false);
        } else {
            nextPage();
        }
    }

    // Hold the marker banner, then redraw the page underneath it
    if (markBannerUntilMs != 0) {
        if ((long)(now - markBannerUntilMs) < 0) return;
        markBannerUntilMs = 0;
        showPage(currentPage);
    }
    // Hold the reset prompt; if it timed out, put the page back
    if (promptShown) {
        if (resetPrompt.isOpen(now)) return;
        promptShown = false;
        showPage(currentPage);
    }

    unsigned long frameMs = currentPage == PAGE_DASH ? DASH_FRAME_MS
                          : isStatsPage(currentPage)  ? STATS_PAGE_FRAME_MS
                                                      : CAN_PAGE_FRAME_MS;
    if (lastFrameMs != 0 && now - lastFrameMs < frameMs) return;
    lastFrameMs = now;

    switch (currentPage) {
    case PAGE_DASH: dash.update(telemetry, now); break;
    case PAGE_TRIP: tripPage.update(shownRide().trip); break;
    case PAGE_ZONES: zonesPage.update(shownRide().trip); break;
    case PAGE_ELEVATION:
    case PAGE_SPEED:
    case PAGE_COOLANT: chartPage.update(); break;
    default: canPage.update(canStatus(), sdLogStatus(), now);
    }
    prof.mark(PROF_UI, micros());
}
