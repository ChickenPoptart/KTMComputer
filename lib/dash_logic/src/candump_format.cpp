#include "candump_format.h"

#include <stdio.h>
#include <string.h>

int formatCandumpLine(char* out, size_t cap, uint64_t timestampUs,
                      uint32_t id, bool extended, uint8_t len, const uint8_t* data) {
    if (len > 8) len = 8;

    int n = snprintf(out, cap, extended ? "(%lu.%06lu) can0 %08lX#" : "(%lu.%06lu) can0 %03lX#",
                     (unsigned long)(timestampUs / 1000000ULL),
                     (unsigned long)(timestampUs % 1000000ULL),
                     (unsigned long)id);
    for (int i = 0; i < len; i++) {
        n += snprintf(out + n, cap - n, "%02X", data[i]);
    }
    n += snprintf(out + n, cap - n, "\n");
    return n;
}

int formatMarkerLine(char* out, size_t cap, uint64_t timestampUs, uint16_t number) {
    return snprintf(out, cap, "(%lu.%06lu) mark 000#%04X\n",
                    (unsigned long)(timestampUs / 1000000ULL),
                    (unsigned long)(timestampUs % 1000000ULL),
                    (unsigned)number);
}

int formatNmeaLine(char* out, size_t cap, uint64_t timestampUs, const char* sentence) {
    int len = (int)strlen(sentence);
    if (len > 0 && sentence[len - 1] == '\r') len--;
    return snprintf(out, cap, "(%lu.%06lu) nmea %.*s\n",
                    (unsigned long)(timestampUs / 1000000ULL),
                    (unsigned long)(timestampUs % 1000000ULL),
                    len, sentence);
}
