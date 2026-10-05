#include "can_bus.h"

#include <Arduino.h>
#include <driver/twai.h>
#include <esp_timer.h>
#include "can_guard.h"
#include "candump_format.h"
#include "sd_log.h"

// CYD CN1 connector -> SN65HVD230
static const gpio_num_t CAN_TX_PIN = GPIO_NUM_22;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_27;

// Deep enough to ride out a full-screen redraw at a busy bus's frame rate
static const int RX_QUEUE_LEN = 256;

// Frames handled per canPoll() call, so touch and the display always get a
// turn even if frames arrive faster than one pass can drain them
static const int MAX_FRAMES_PER_POLL = 64;

// How often the error guard looks at the controller, and how often a
// health line goes into the SD log
static const unsigned long GUARD_CHECK_MS = 250;
static const unsigned long HEALTH_LOG_MS = 10000;

static bool installed = false;
static bool running = false;
static CanGuard guard;
static uint32_t framesTotal = 0;
static uint32_t errorsTotal = 0;  // never goes backwards, even if the driver's does

bool canBegin() {
    twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    g.rx_queue_len = RX_QUEUE_LEN;
    twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
    installed = true;
    if (twai_start() != ESP_OK) return false;

    running = true;
    Serial.println("# CAN listen-only 500k. Frames go to the SD card only");
    return true;
}

static void updateErrorTotal() {
    static uint32_t lastDriverCount = 0;
    twai_status_info_t info;
    if (!running || twai_get_status_info(&info) != ESP_OK) return;
    uint32_t count = info.bus_error_count;
    errorsTotal += count >= lastDriverCount ? count - lastDriverCount : count;
    lastDriverCount = count;
}

static void guardCheck(unsigned long nowMs) {
    static unsigned long lastCheckMs = 0, lastHealthMs = 0;
    if (!installed || nowMs - lastCheckMs < GUARD_CHECK_MS) return;
    lastCheckMs = nowMs;

    updateErrorTotal();
    char text[80];
    switch (guard.check(nowMs, framesTotal, errorsTotal)) {
    case GUARD_STOP:
        twai_stop();
        running = false;
        snprintf(text, sizeof(text), "CAN error flood (frames=%lu errors=%lu): CAN off for %lu s",
                 (unsigned long)framesTotal, (unsigned long)errorsTotal, CAN_GUARD_PAUSE_MS / 1000);
        sdLogNote(text);
        break;
    case GUARD_RESTART:
        running = twai_start() == ESP_OK;
        sdLogNote(running ? "CAN restarted" : "CAN restart failed");
        break;
    case GUARD_NONE:
        break;
    }

    if (nowMs - lastHealthMs >= HEALTH_LOG_MS) {
        lastHealthMs = nowMs;
        snprintf(text, sizeof(text), "can frames=%lu errors=%lu%s", (unsigned long)framesTotal,
                 (unsigned long)errorsTotal, guard.faulted() ? " FAULT" : "");
        sdLogNote(text);
    }
}

void canPoll(CanTracker& tracker, KtmState& ktm, unsigned long nowMs) {
    guardCheck(nowMs);
    if (!running) return;

    twai_message_t m;
    for (int i = 0; i < MAX_FRAMES_PER_POLL && twai_receive(&m, 0) == ESP_OK; i++) {
        if (m.rtr) continue;
        framesTotal++;

        // 64-bit microseconds since boot; micros() would wrap after 71 min
        char line[CANDUMP_LINE_MAX];
        int n = formatCandumpLine(line, sizeof(line), esp_timer_get_time(),
                                  m.identifier, m.extd, m.data_length_code, m.data);
        sdLogWrite(line, n);  // every frame goes to the card
        ktmDecodeFrame(ktm, nowMs, m.identifier, m.extd, m.data_length_code, m.data);

        tracker.record(m.identifier, m.extd, m.data_length_code, m.data, nowMs);
    }
}

CanBusStatus canStatus() {
    CanBusStatus s;
    s.running = running;
    s.faulted = guard.faulted();
    s.busErrors = errorsTotal;
    if (!running) {
        s.state = s.faulted ? "CAN FAULT" : "OFF";
        return s;
    }

    twai_status_info_t info;
    if (twai_get_status_info(&info) != ESP_OK) return s;
    s.rxMissed = info.rx_missed_count;
    switch (info.state) {
        case TWAI_STATE_RUNNING:    s.state = s.faulted ? "CHECKING" : "RUN"; break;
        case TWAI_STATE_BUS_OFF:    s.state = "BUS OFF"; break;
        case TWAI_STATE_RECOVERING: s.state = "RECOVER"; break;
        default:                    s.state = "STOPPED"; break;
    }
    return s;
}
