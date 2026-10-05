# Firmware

PlatformIO project, Arduino framework on the ESP32. Two environments:

| Environment | Command | What |
|---|---|---|
| `esp32dev` | `pio run -e esp32dev -t upload` | Builds and flashes the CYD |
| `native` | `pio test -e native` | Runs the unit tests on the computer |

## Layout

Logic that doesn't touch hardware lives in `lib/dash_logic/` and has unit
tests in `test/`. Code that talks to the display, CAN controller, GPS, touch
panel or SD card lives in `src/`.

### `lib/dash_logic/src/` (tested on the computer)

| File | Does |
|---|---|
| `ktm_decode` | Turns CAN frames into RPM, gear, coolant, speed, with staleness |
| `can_tracker` | Per-ID table for the CAN pages (rates, changed bytes) |
| `can_guard.h` | Decides when a CAN error flood means CAN should pause |
| `candump_format` | Log line formats: CAN frames, markers, GPS sentences |
| `dash_logic` | Tach segments and colour zones, gear labels, compass points, durations |
| `gps_view.h` | GPS fix, heading hold below 3 mph, elevation |
| `ride_clock` | GPS date/time to epoch, Mountain time with daylight saving |
| `trip_stats.h` | Distance, times, maximums, climb/descent, time per RPM zone |
| `chart_series.h` | Chart storage that merges pairs so a whole ride fits |
| `ride_history.h` | Ride file names, which rides to save, `<` `>` browsing |
| `log_rotation` | Log file numbering and deleting the oldest when the card fills |
| `press_detector.h`, `tap_sequencer.h` | Tap / long press / double tap from the touch panel |
| `reset_prompt.h` | The two-step "TAP TO CLEAR" |
| `touch_map.h` | Touch calibration: raw panel readings to screen pixels |
| `loop_profiler.h` | Where the main loop's time goes (diagnostic notes) |

### `src/` (hardware)

| File | Does |
|---|---|
| `main.cpp` | Setup, the main loop, pages, touch handling, ride saving |
| `can_bus` | ESP32 TWAI controller, listen-only, 500 kbps, plus the error guard |
| `gps` | NEO-M8N on UART2 (IO35), TinyGPSPlus parser, logs every sentence |
| `sd_log` | SD card, background writer task on core 0, log rotation, notes |
| `ride_store` | Saves and loads `/rides/ride_NNNN.bin` |
| `touch` | Bit-banged XPT2046 touch controller |
| `dash_ui`, `trip_page`, `chart_page`, `zones_page`, `can_page`, `history_header` | Screens |
| `telemetry.h` | What the dashboard shows |

## Settings you might change

| Setting | File | Default |
|---|---|---|
| Tach colour zones | `TACH_SEGMENT_RPM` in `dash_logic.h` | 1000 / 3000 / 3900 / 4800 / 5700 / 6500 / 7300 / 8200 |
| Shift light | `SHIFT_RPM` in `dash_logic.h` | 8,200 |
| Speed scale | `FRONT_COUNTS_PER_10_MPH` in `ktm_decode.cpp` | 274 (27.4 counts per mph) |
| Time zone | `MOUNTAIN_TZ` in `ride_clock.h` | US Mountain with DST |
| Screen rotation | `SCREEN_ROTATION` in `main.cpp` | 1 (redo touch calibration if changed) |
| Free space kept on the card | `MIN_FREE_PERCENT` in `log_rotation.h` | 10% |
| Minimum engine time for a saved ride | `RIDE_MIN_SECONDS` in `ride_history.h` | 60 s |
| Double-tap window | `DOUBLE_TAP_MS` in `tap_sequencer.h` | 350 ms |

Changing anything in `RideRecord` (`src/ride_store.h`) or what it contains
changes the saved-ride file layout: bump `RIDE_VERSION` in
`ride_store.cpp` so older files are skipped instead of misread.

## How the main loop runs

Each pass: drain up to 64 CAN frames, read the GPS, update trip stats
(every 100 ms) and charts (every 2 s), save the ride (every 30 s), handle
touch, then redraw the current page at its own rate (dashboard every
40 ms). Nothing in the loop waits on hardware: SD writes go through a queue
to a task on the other core, and serial carries only occasional status
lines.

## Serial output (USB, 115200 baud)

Status only: boot messages, a GPS line every 5 seconds
(`# GPS chars=… sats=… fix=… time=…`), and the same `note` lines that go
into the log.
