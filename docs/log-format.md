# SD card ride logs

One file per power-up: `can_NNNN.log`. Numbers only go up (one past the
highest on the card), so the lowest number is the oldest log.

Every line starts with a timestamp in seconds since boot, then a channel:

| Channel | Example | What |
|---|---|---|
| `can0` | `(12.345678) can0 120#05DC101000000030` | Every CAN frame from the bike, candump format |
| `mark` | `(31.676005) mark 000#0001` | Long-press marker number (hex), restarts at 1 each boot |
| `nmea` | `(3.250000) nmea $GNRMC,…*47` | Every sentence from the NEO-M8N GPS, unchanged |
| `note` | `(10.000172) note loop passes=576448 slowest=21ms …` | Diagnostics in plain text (see below) |

`can0` and `mark` lines are valid candump syntax; tools that read candump
logs may need the `nmea` and `note` lines filtered out first
(`grep -v -e ' nmea ' -e ' note ' can_0012.log`). GPS tools that read NMEA can take the
sentences with `grep ' nmea ' can_0012.log | cut -d' ' -f3-`.

## Notes

| Note | When | Says |
|---|---|---|
| `boot reset_reason=… can=… sd=…` | At startup | Why the board last reset (1 = power-on), whether CAN and the SD card started |
| `loop passes=… slowest=…ms can=…ms gps=…ms …` | Every 10 s | How many main-loop passes ran and where the time went; ~600,000 passes per 10 s is normal. Also SD bytes written and lines dropped |
| `can frames=… errors=…` | Every 10 s | Running totals from the CAN controller; `FAULT` while an error flood is being handled |
| `CAN error flood …: CAN off for 10 s` / `CAN restarted` | On a fault | The [CAN error guard](troubleshooting.md#can-fault-on-the-dash) acting |

## Ride history files

`/rides/ride_NNNN.bin` holds the trip stats and charts for the ride logged
in `can_NNNN.log`, saved every 30 seconds. It's a raw memory image with a
small header (magic `KTMR`, version, size); files from a different firmware
version are skipped rather than misread.

## Space on the card

At boot, before recording starts, the firmware deletes the oldest logs
until at least 10% of the card is free (`MIN_FREE_PERCENT` in
`lib/dash_logic/src/log_rotation.h`). A log is roughly 50–80 MB per hour of
riding, so 10% of a 32 GB card is ~40 hours: one ride can't fill it, and
nothing is deleted mid-ride. Copy logs you want to keep off the card; old
ones disappear once it fills.
