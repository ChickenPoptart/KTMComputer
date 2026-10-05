#pragma once

#include <TFT_eSPI.h>

// Top row of the trip, chart and zones pages: < older ride, the page title
// and which ride is shown, > newer ride.
//   [<]  TITLE            #12 Sat Oct 3  [>]
const int HEADER_H = 24;
const int NAV_W = 38;

// Tap targets are larger than the drawn buttons; fingers and a resistive
// panel aren't precise.
inline bool tapHitsOlder(int x, int y) { return x < 70 && y < 50; }
inline bool tapHitsNewer(int x, int y) { return x >= 250 && y < 50; }

void drawHistoryHeader(TFT_eSPI& tft, const char* title, uint16_t titleColor,
                       const char* rideLabel, bool canOlder, bool canNewer);

// Left edge for text after the title (e.g. a legend).
int historyTitleEnd(TFT_eSPI& tft, const char* title);
