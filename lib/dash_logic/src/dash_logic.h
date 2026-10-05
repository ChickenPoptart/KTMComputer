#pragma once

// Pure dashboard logic with no hardware dependencies, so it can be unit
// tested on the host (pio test -e native).

// 2020 KTM 690 Enduro R (LC4 693cc): peak power ~8,200 rpm (Cycle World
// dyno), rev limiter ~9,000 rpm (2016+). Each segment lights at its own
// RPM, grouped into colour zones:
//   1    cyan    running / idle (this bike idles ~1,700)
//   2-5  green   healthy riding range; the single lugs below 3,000
//   6-7  yellow  time to shift
//   8    red     top end; the bar flashes from SHIFT_RPM
const int TACH_SEGMENTS = 8;
const int SHIFT_RPM = 8200;
const int TACH_SEGMENT_RPM[TACH_SEGMENTS] = {1000, 3000, 3900, 4800, 5700, 6500, 7300, SHIFT_RPM};

// Number of tach segments lit for a given RPM (0..TACH_SEGMENTS).
int tachLitSegments(int rpm);

bool shiftActive(int rpm);

// Which colour zone of the tach bar an RPM falls in.
enum TachZone { ZONE_OFF, ZONE_IDLE, ZONE_GREEN, ZONE_YELLOW, ZONE_RED, ZONE_COUNT };
TachZone tachZone(int rpm);

// "m:ss" under an hour, "h:mm:ss" from then on.
void formatDuration(char* out, int cap, unsigned long ms);

// "N" for neutral, "1".."6" for gears, "-" for anything unexpected.
const char* gearLabel(int gear);

// 8-point compass direction ("N", "NE", ...) nearest to a heading in degrees.
const char* headingCardinal(int degrees);
