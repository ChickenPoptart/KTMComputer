#include "sd_log.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <freertos/semphr.h>
#include <freertos/stream_buffer.h>
#include <esp_timer.h>
#include "log_rotation.h"

// CYD MicroSD slot, on the VSPI bus
static const int SD_SCK = 18;
static const int SD_MISO = 19;
static const int SD_MOSI = 23;
static const int SD_CS = 5;
static const uint32_t SD_SPI_HZ = 20000000;

// SD cards can stall for a few hundred ms during internal housekeeping.
// 64 KB covers roughly half a second of a busy 500 kbps bus.
static const size_t QUEUE_BYTES = 64 * 1024;
static const size_t WRITE_CHUNK = 4096;
// Flush regularly so killing the ignition loses at most this much data
static const unsigned long FLUSH_MS = 1000;

// More ride logs than this on one card are ignored by the cleanup; at
// hundreds of hours per card it won't be reached.
static const int MAX_LOGS = 2048;

static SPIClass sdSpi(VSPI);
static File logFile;
static StreamBufferHandle_t queue = nullptr;
static char fileName[16] = "";
static int logNumber = -1;
// The log writer and ride saves both use the card; one at a time
static SemaphoreHandle_t cardMutex = nullptr;
static bool recording = false;
static volatile uint32_t bytesWritten = 0;
static volatile uint32_t writeErrors = 0;
static volatile uint32_t droppedLines = 0;

static void writerTask(void*) {
    static uint8_t chunk[WRITE_CHUNK];
    unsigned long lastFlushMs = millis();
    for (;;) {
        size_t n = xStreamBufferReceive(queue, chunk, sizeof(chunk), pdMS_TO_TICKS(200));
        if (n > 0) {
            sdLock();
            size_t written = logFile.write(chunk, n);
            sdUnlock();
            bytesWritten += written;
            if (written != n) writeErrors++;
        }
        if (millis() - lastFlushMs >= FLUSH_MS) {
            sdLock();
            logFile.flush();
            sdUnlock();
            lastFlushMs = millis();
        }
    }
}

// Lists the ride logs on the card: their numbers and sizes.
static int listLogs(int* numbers, uint32_t* sizes, int cap) {
    int count = 0;
    File root = SD.open("/");
    for (File f = root.openNextFile(); f && count < cap; f = root.openNextFile()) {
        int n = f.isDirectory() ? -1 : logNumberFromName(f.name());
        if (n >= 0) {
            numbers[count] = n;
            sizes[count] = f.size();
            count++;
        }
        f.close();
    }
    root.close();
    return count;
}

// Deletes the oldest logs until the card has enough free space. Runs at
// boot, before recording starts, so it never competes with logging.
static int freeSpace(int* numbers, uint32_t* sizes, int count) {
    uint64_t total = SD.totalBytes();
    uint64_t freeBytes = total - SD.usedBytes();
    int victim;
    while ((victim = logToDelete(numbers, count, -1, freeBytes, total)) >= 0) {
        char name[24];
        snprintf(name, sizeof(name), "/can_%04d.log", victim);
        for (int i = 0; i < count; i++) {
            if (numbers[i] != victim) continue;
            if (SD.remove(name)) {
                Serial.printf("# Card low on space: deleted %s\n", name);
                freeBytes += sizes[i];
            }
            snprintf(name, sizeof(name), "/rides/ride_%04d.bin", victim);
            SD.remove(name);  // that ride's history goes with its log
            numbers[i] = numbers[--count];  // drop it from the list either way
            sizes[i] = sizes[count];
            break;
        }
    }
    return count;
}

bool sdLogBegin() {
    sdSpi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, sdSpi, SD_SPI_HZ)) return false;

    static int numbers[MAX_LOGS];
    static uint32_t sizes[MAX_LOGS];
    int count = listLogs(numbers, sizes, MAX_LOGS);
    count = freeSpace(numbers, sizes, count);

    logNumber = nextLogNumber(numbers, count);
    snprintf(fileName, sizeof(fileName), "/can_%04d.log", logNumber);
    if (!SD.exists("/rides")) SD.mkdir("/rides");
    cardMutex = xSemaphoreCreateMutex();
    logFile = SD.open(fileName, FILE_WRITE);
    if (!logFile) return false;

    queue = xStreamBufferCreate(QUEUE_BYTES, WRITE_CHUNK / 4);
    if (!queue) return false;

    // Core 0, away from the display/CAN loop on core 1
    xTaskCreatePinnedToCore(writerTask, "sdlog", 4096, nullptr, 1, nullptr, 0);
    recording = true;
    return true;
}

void sdLogWrite(const char* line, int len) {
    if (!recording) return;
    if (xStreamBufferSpacesAvailable(queue) < (size_t)len) {
        droppedLines++;
        return;
    }
    xStreamBufferSend(queue, line, len, 0);
}

void sdLock() { xSemaphoreTake(cardMutex, portMAX_DELAY); }
void sdUnlock() { xSemaphoreGive(cardMutex); }
int sdLogNumber() { return recording ? logNumber : -1; }

void sdLogNote(const char* text) {
    char line[280];
    uint64_t us = esp_timer_get_time();
    int n = snprintf(line, sizeof(line), "(%lu.%06lu) note %s\n", (unsigned long)(us / 1000000ULL),
                     (unsigned long)(us % 1000000ULL), text);
    if (n <= 0 || n >= (int)sizeof(line)) return;
    sdLogWrite(line, n);
    Serial.print(line);
}

SdLogStatus sdLogStatus() {
    SdLogStatus s;
    s.recording = recording;
    s.fileName = fileName;
    s.bytesWritten = bytesWritten;
    s.droppedLines = droppedLines;
    s.writeErrors = writeErrors;
    return s;
}
