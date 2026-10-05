#include "dash_logic.h"

#include <stdio.h>

int tachLitSegments(int rpm) {
    int lit = 0;
    while (lit < TACH_SEGMENTS && rpm >= TACH_SEGMENT_RPM[lit]) lit++;
    return lit;
}

bool shiftActive(int rpm) {
    return rpm >= SHIFT_RPM;
}

TachZone tachZone(int rpm) {
    int lit = tachLitSegments(rpm);
    if (lit == 0) return ZONE_OFF;
    if (lit == 1) return ZONE_IDLE;
    if (lit <= 5) return ZONE_GREEN;
    if (lit <= 7) return ZONE_YELLOW;
    return ZONE_RED;
}

const char* gearLabel(int gear) {
    static const char* const labels[] = {"N", "1", "2", "3", "4", "5", "6"};
    if (gear < 0 || gear > 6) return "-";
    return labels[gear];
}

const char* headingCardinal(int degrees) {
    static const char* const points[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int normalized = ((degrees % 360) + 360) % 360;
    return points[((normalized * 2 + 45) / 90) % 8];
}

void formatDuration(char* out, int cap, unsigned long ms) {
    unsigned long sec = ms / 1000;
    unsigned long h = sec / 3600, m = (sec / 60) % 60, s = sec % 60;
    if (h > 0) snprintf(out, cap, "%lu:%02lu:%02lu", h, m, s);
    else snprintf(out, cap, "%lu:%02lu", m, s);
}
