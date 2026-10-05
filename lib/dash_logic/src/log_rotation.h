#pragma once

#include <stdint.h>

// Rules for naming ride logs and freeing space on the SD card. Logs are
// can_NNNN.log; the number only ever goes up, so lowest = oldest.

// N in "<prefix>N<ext>" (any directory in front is ignored), or -1.
int numberedFileName(const char* path, const char* prefix, const char* ext);

// Number in "can_NNNN.log", or -1 if the name isn't a ride log.
int logNumberFromName(const char* name);

// Number for the new log: one past the highest existing, so a number is
// never reused after old logs are deleted.
int nextLogNumber(const int* existing, int count);

// Keep at least this share of the card free. At ~80 MB per riding hour,
// 10% of a 32 GB card is ~40 hours, far more than one ride can fill.
const int MIN_FREE_PERCENT = 10;

// The log to delete to make space (the oldest, never the one being
// recorded), or -1 if there's enough free space or nothing to delete.
int logToDelete(const int* existing, int count, int recording,
                uint64_t freeBytes, uint64_t totalBytes);
