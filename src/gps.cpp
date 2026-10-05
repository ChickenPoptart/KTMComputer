#include "gps.h"

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <esp_timer.h>
#include "candump_format.h"
#include "ride_clock.h"
#include "sd_log.h"

static const int GPS_RX_PIN = 35;  // input-only pin, fine for receive
static const uint32_t GPS_BAUD = 9600;

static TinyGPSPlus parser;

// The sentence being received, logged to the SD card once complete
static char sentence[NMEA_LINE_MAX];
static int sentenceLen = 0;

static void collect(char c) {
    if (c == '$') sentenceLen = 0;  // a new sentence always starts with $
    if (c == '\n') {
        if (sentenceLen > 0 && sentence[0] == '$') {
            sentence[sentenceLen] = '\0';
            char line[NMEA_LINE_MAX];
            int n = formatNmeaLine(line, sizeof(line), esp_timer_get_time(), sentence);
            if (n < (int)sizeof(line)) sdLogWrite(line, n);
        }
        sentenceLen = 0;
        return;
    }
    if (sentenceLen < (int)sizeof(sentence) - 40) sentence[sentenceLen++] = c;
    else sentenceLen = 0;  // garbage or a broken line: wait for the next $
}

void gpsBegin() {
    Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, -1);
}

void gpsPoll() {
    while (Serial2.available() > 0) {
        char c = Serial2.read();
        parser.encode(c);
        collect(c);
    }
}

bool gpsUtcTime(time_t& out) {
    // Before a fix the module reports a placeholder date (1980 or 2080)
    if (!parser.date.isValid() || !parser.time.isValid()) return false;
    if (parser.time.age() > 2000) return false;
    int year = parser.date.year();
    if (year < 2024 || year > 2079) return false;
    out = utcEpoch(year, parser.date.month(), parser.date.day(),
                   parser.time.hour(), parser.time.minute(), parser.time.second());
    return true;
}

uint32_t gpsCharsReceived() { return parser.charsProcessed(); }

GpsReading gpsReading() {
    GpsReading r;
    r.locationValid = parser.location.isValid();
    r.ageMs = parser.location.age();
    r.satellites = parser.satellites.isValid() ? parser.satellites.value() : 0;
    r.courseDeg = parser.course.deg();
    r.speedMph = parser.speed.mph();
    r.altitudeValid = parser.altitude.isValid();
    r.altitudeFt = parser.altitude.feet();
    return r;
}
