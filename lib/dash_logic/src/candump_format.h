#pragma once

#include <stddef.h>
#include <stdint.h>

// Longest line: "(99999999999.999999) can0 1FFFFFFF#0011223344556677\n"
const int CANDUMP_LINE_MAX = 64;

// Formats one frame in Linux candump log format, readable by SavvyCAN,
// python-can and cantools. Returns the line length including the newline.
int formatCandumpLine(char* out, size_t cap, uint64_t timestampUs,
                      uint32_t id, bool extended, uint8_t len, const uint8_t* data);

// Formats a numbered event marker (from a long press) as a candump line on
// its own "mark" channel, so tools still parse the log and markers can't be
// mistaken for bike frames: "(12.500000) mark 000#0003"
int formatMarkerLine(char* out, size_t cap, uint64_t timestampUs, uint16_t number);

// NMEA sentences are at most 82 characters; room for the timestamp prefix.
const int NMEA_LINE_MAX = 128;

// Formats one GPS sentence on its own "nmea" channel, timestamped like the
// CAN frames: "(3.250000) nmea $GNGGA,...*47". A trailing \r is dropped.
int formatNmeaLine(char* out, size_t cap, uint64_t timestampUs, const char* sentence);
