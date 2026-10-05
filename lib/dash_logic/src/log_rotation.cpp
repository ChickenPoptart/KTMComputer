#include "log_rotation.h"

#include <string.h>

int numberedFileName(const char* path, const char* prefix, const char* ext) {
    const char* name = strrchr(path, '/');
    name = name ? name + 1 : path;
    size_t prefixLen = strlen(prefix);
    if (strncmp(name, prefix, prefixLen) != 0) return -1;
    const char* p = name + prefixLen;

    int number = 0, digits = 0;
    while (*p >= '0' && *p <= '9') {
        number = number * 10 + (*p - '0');
        digits++;
        p++;
    }
    if (digits == 0 || strcmp(p, ext) != 0) return -1;
    return number;
}

int logNumberFromName(const char* name) {
    return numberedFileName(name, "can_", ".log");
}

int nextLogNumber(const int* existing, int count) {
    int highest = 0;
    for (int i = 0; i < count; i++) {
        if (existing[i] > highest) highest = existing[i];
    }
    return highest + 1;
}

int logToDelete(const int* existing, int count, int recording,
                uint64_t freeBytes, uint64_t totalBytes) {
    if (freeBytes * 100 >= totalBytes * MIN_FREE_PERCENT) return -1;

    int oldest = -1;
    for (int i = 0; i < count; i++) {
        if (existing[i] == recording) continue;
        if (oldest == -1 || existing[i] < oldest) oldest = existing[i];
    }
    return oldest;
}
