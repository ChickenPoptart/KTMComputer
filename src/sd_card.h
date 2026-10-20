#pragma once

// The MicroSD card's filesystem, whichever way the board drives it:
//   CYD:         SPI (SD library), chip select on GPIO 5
//   Waveshare 5": the ESP32-S3's SD controller in 1-bit mode (SD_MMC). Its
//                 card slot is wired for SPI with chip select on the CH422G
//                 expander, which the SPI library can't drive; SD mode
//                 needs no chip select, only that line held high (D3).
#ifdef BOARD_W5
#include <SD_MMC.h>
#define CARD SD_MMC
#else
#include <SD.h>
#define CARD SD
#endif
