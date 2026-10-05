# Using the dash

## Touch

| Gesture | Does |
|---|---|
| **Tap** | Next page (shows ~⅓ s after lifting, to tell it from a double tap) |
| **Double tap** | Back to the dashboard, from any page, and back to this ride |
| **Hold ~1 s** on the dashboard or CAN pages | Drops a numbered **marker** into the log (yellow `MARK n` box) |
| **Hold ~1 s** on a trip or chart page | Shows **TAP TO CLEAR**: tap within 3 s to clear that page's data for this ride, or wait to cancel |
| **`<` / `>`** (top corners of the trip and chart pages) | Step to older / newer saved rides |

## Pages

Tap to cycle in this order:

1. **Dashboard:** tach bar, RPM, shift light, gear, speed, coolant, GPS
   heading, elevation, satellites, time.
2. **Trip:** distance, max speed, ride time, moving time, max RPM, max
   coolant, climb, descent.
3. **Elevation** chart (GPS).
4. **Speed** chart: wheel speed (white) with GPS speed (green).
5. **RPM zones:** time and share in idle, green, shift and top end.
6. **Coolant** chart.
7. **CAN pages:** every CAN ID with its rate and bytes (yellow = changed
   in the last second, white = has changed, grey = never changed), plus bus
   state and SD recording status.

## Dashboard

| Item | Source | Notes |
|---|---|---|
| Tach bar | RPM (`0x120`) | Cyan from 1,000 (idle), green 3,000–5,700, yellow 6,500–7,300, red at 8,200, where the bar flashes and the shift light comes on. Thresholds are `TACH_SEGMENT_RPM` in `lib/dash_logic/src/dash_logic.h` |
| Gear | `0x129` | `N` in green; `-` with no data |
| MPH | Front wheel speed (`0x12B`) | Instant (100 updates/s). Scale ~27.4 counts per mph, about ±5% until calibrated against GPS |
| Coolant | `0x540` | °F |
| Heading | GPS course | Only updates above 3 mph; holds the last heading when stopped |
| Elevation | GPS altitude | ±30–60 ft typical |
| GPS / SATS | GPS | `NO FIX` until it has a position |
| Time | GPS | Mountain time with daylight saving. `--:--` until the GPS sets the clock. Shows **CAN FAULT** in red instead if the CAN wiring looks broken |

Anything that stops arriving for more than a second turns to `--`.

## Trip, charts and ride history

- Everything is collected all the time, whatever page is showing, starting
  fresh at each key-on.
- Charts take a point every 2 seconds. Each holds 280 points; when full,
  neighbouring points are averaged in pairs, so the chart always shows the
  **whole ride**, at less detail the longer it gets. Maximums on the Trip
  page are tracked separately at full speed, so short spikes are never lost.
- Climb/descent ignores GPS jitter smaller than 30 ft.
- **A ride** is one key-on with at least a minute of engine running. It's
  saved to the SD card every 30 seconds (`/rides/ride_NNNN.bin`), so turning
  the key off loses at most 30 s. Bench power-ups aren't saved.
- On a past ride the header shows its number and date, e.g.
  `#17 Sat Oct 4`. Past rides can't be cleared.

## Markers

Hold on the dashboard to mark a moment ("I squeezed the front brake now").
Markers go into the log as `mark` lines, so the bytes that changed at that
moment are easy to find later. The count restarts at 1 each key-on.

## The SD card

- One `can_NNNN.log` per key-on, plus `/rides/` for ride history.
- Copy logs you want to keep. When the card is under 10% free at boot, the
  oldest logs (and their ride history) are deleted.
- After the GPS has a fix, files get real dates on the computer.
