#pragma once

#include <stdint.h>

// Marks a sample with no data (no GPS fix, bike not sending); drawn as a gap.
const int16_t CHART_NO_DATA = INT16_MIN;

// Points for one chart, sampled at a fixed interval. When full, neighbouring
// points are averaged in pairs and each point then covers twice as many
// samples, so the chart always holds the whole ride at the best detail
// that fits. CAPACITY must be even.
template <int CAPACITY>
class ChartSeries {
public:
    void add(int16_t sample) {
        if (count == CAPACITY && pendingSamples == 0) mergePairs();

        pendingSamples++;
        if (sample != CHART_NO_DATA) {
            pendingSum += sample;
            pendingValid++;
        }
        if (pendingSamples == stride) {
            points[count++] = pendingValid ? (int16_t)(pendingSum / pendingValid) : CHART_NO_DATA;
            pendingSamples = pendingValid = 0;
            pendingSum = 0;
        }
    }

    int size() const { return count; }
    int16_t at(int i) const { return points[i]; }
    int samplesPerPoint() const { return stride; }

    // Lowest and highest points with data; false if there are none.
    bool range(int16_t& lo, int16_t& hi) const {
        bool any = false;
        for (int i = 0; i < count; i++) {
            if (points[i] == CHART_NO_DATA) continue;
            if (!any || points[i] < lo) lo = points[i];
            if (!any || points[i] > hi) hi = points[i];
            any = true;
        }
        return any;
    }

private:
    void mergePairs() {
        for (int i = 0; i < CAPACITY / 2; i++) {
            points[i] = average(points[2 * i], points[2 * i + 1]);
        }
        count = CAPACITY / 2;
        stride *= 2;
    }

    static int16_t average(int16_t a, int16_t b) {
        if (a == CHART_NO_DATA) return b;
        if (b == CHART_NO_DATA) return a;
        return (int16_t)(((int32_t)a + b) / 2);
    }

    int16_t points[CAPACITY];
    int count = 0;
    int stride = 1;  // samples per point
    int pendingSamples = 0, pendingValid = 0;
    int32_t pendingSum = 0;
};
