#include "ride_clock.h"

#include <stdio.h>

// Days since 1970-01-01 for a proleptic Gregorian date (Howard Hinnant's
// days_from_civil), so it doesn't depend on the libc's timegm.
static long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

time_t utcEpoch(int year, int month, int day, int hour, int minute, int second) {
    return (time_t)daysFromCivil(year, month, day) * 86400 + hour * 3600 + minute * 60 + second;
}

void formatClock(char* out, int cap, time_t utc) {
    struct tm local;
    localtime_r(&utc, &local);
    int hour12 = local.tm_hour % 12 == 0 ? 12 : local.tm_hour % 12;
    snprintf(out, cap, "%d:%02d %s", hour12, local.tm_min, local.tm_hour < 12 ? "AM" : "PM");
}

void formatDate(char* out, int cap, time_t utc) {
    static const char* const DAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* const MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm local;
    localtime_r(&utc, &local);
    snprintf(out, cap, "%s %s %d", DAYS[local.tm_wday], MONTHS[local.tm_mon], local.tm_mday);
}
