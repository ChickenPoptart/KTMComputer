#pragma once

#include <stdint.h>
#include <time.h>
#include "gps_view.h"

// u-blox NEO-M8N on UART2. Only its TX is wired (to IO35 on the P3
// connector), so it runs at the factory default: 9600 baud NMEA, 1 Hz.
void gpsBegin();

// Feeds whatever bytes have arrived to the NMEA parser, and logs each
// complete sentence to the SD card. Never blocks.
void gpsPoll();

GpsReading gpsReading();

// Current UTC time from the GPS, once it has a fresh date and time.
bool gpsUtcTime(time_t& out);

// Bytes received from the module so far; stuck at 0 means a wiring problem.
uint32_t gpsCharsReceived();
