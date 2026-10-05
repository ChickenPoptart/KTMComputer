#pragma once

#include <stdint.h>
#include <stdio.h>

// Where the main loop's time goes, for diagnosing a slow or frozen dash on
// the bike. Interrupt time lands in whichever section it interrupted.
enum ProfSection { PROF_CAN, PROF_GPS, PROF_STATS, PROF_SERIAL, PROF_TOUCH, PROF_UI, PROF_OTHER, PROF_COUNT };

class LoopProfiler {
public:
    // Call at the top of loop(); time since the last mark is PROF_OTHER.
    void startPass(uint32_t nowUs) {
        if (running) {
            total[PROF_OTHER] += nowUs - lastUs;
            uint32_t passUs = nowUs - passStartUs;
            if (passUs > slowest) slowest = passUs;
            completed++;
        }
        running = true;
        passStartUs = lastUs = nowUs;
    }

    // Call after each section; the time since the previous mark is its.
    void mark(ProfSection s, uint32_t nowUs) {
        total[s] += nowUs - lastUs;
        lastUs = nowUs;
    }

    uint32_t totalUs(ProfSection s) const { return total[s]; }
    uint32_t passes() const { return completed; }
    uint32_t slowestPassUs() const { return slowest; }

    // One line of milliseconds per section, then starts counting afresh.
    void report(char* out, int cap) {
        snprintf(out, cap,
                 "loop passes=%lu slowest=%lums can=%lums gps=%lums stats=%lums serial=%lums "
                 "touch=%lums ui=%lums other=%lums",
                 (unsigned long)completed, (unsigned long)(slowest / 1000),
                 ms(PROF_CAN), ms(PROF_GPS), ms(PROF_STATS), ms(PROF_SERIAL),
                 ms(PROF_TOUCH), ms(PROF_UI), ms(PROF_OTHER));
        for (uint32_t& t : total) t = 0;
        completed = 0;
        slowest = 0;
    }

private:
    unsigned long ms(ProfSection s) const { return total[s] / 1000; }

    bool running = false;
    uint32_t passStartUs = 0, lastUs = 0;
    uint32_t total[PROF_COUNT] = {};
    uint32_t completed = 0, slowest = 0;
};
