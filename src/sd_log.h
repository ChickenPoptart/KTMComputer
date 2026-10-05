#pragma once

#include <stdint.h>

struct SdLogStatus {
    bool recording = false;
    const char* fileName = "";
    uint32_t bytesWritten = 0;
    uint32_t droppedLines = 0;  // lines lost because the card fell behind
    uint32_t writeErrors = 0;   // e.g. card pulled out
};

// Mounts the MicroSD card, deletes the oldest logs if it is low on space,
// and opens /can_NNNN.log one past the highest existing number.
// Returns false (and logging stays off) if there's no card.
bool sdLogBegin();

// Queues one line for the background writer. Never blocks: if the queue is
// full the line is dropped and counted.
void sdLogWrite(const char* line, int len);

// A plain-text diagnostic line, "(12.345678) note <text>", to the log and Serial.
void sdLogNote(const char* text);

SdLogStatus sdLogStatus();

// Number of this power-up's can_NNNN.log, or -1 when not recording.
int sdLogNumber();

// Other SD card users (ride saves) wrap their file access in these so they
// never touch the card at the same time as the background log writer.
void sdLock();
void sdUnlock();
