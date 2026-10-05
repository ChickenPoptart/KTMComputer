# CAN signals — 2020 KTM 690 Enduro R

What each CAN message on this bike carries, as far as it's been worked out.
The bus runs at **500 kbps** with standard 11-bit IDs. Bytes are D0..D7;
multi-byte values are big-endian (high byte first).

## What the dash uses

| Shown as | ID | Bytes | Conversion |
|---|---|---|---|
| RPM | `0x120` | D0–D1 | value = RPM |
| Gear | `0x129` | D0 high nibble | 0 = neutral, 1–6 |
| Coolant | `0x540` | D6–D7 | value ÷ 10 = °C (dash shows °F) |
| Speed | `0x12B` | D0–D1 (front wheel) | value ÷ 27.4 = mph |

Decoding lives in `lib/dash_logic/src/ktm_decode.cpp`, with tests in
`test/test_ktm_decode`. A value that stops arriving for over a second shows
as `--`.

## All IDs seen
"Source" is which module starts sending first after key-on: the ECU starts
3–70 s after key-on, the others immediately (most likely the ABS unit).

Starting point: [blalor/ktm-can](https://github.com/blalor/ktm-can) (also a
2020 690 Enduro R). Marked **confirmed** where our own logs show it; ride
evidence is [`captures/2026-10-01_can_0010.log`](../captures/README.md) (13 markers, see below).

| ID | Period | Source | Contents | Status |
|---|---|---|---|---|
| `120` | 20 ms | ECU | D0–D1 engine rpm. D2 commanded throttle 0–255. D3 bit 4 kill switch (1 = run). D4 bit 0 throttle map in use. D7 high bits rolling counter | rpm, map **confirmed**; on dash |
| `121` | 20 ms | ECU | D1 and D3 rise together with the throttle grip (likely the two redundant ride-by-wire grip sensors). D7 rolling counter | candidate |
| `128` | 20 ms | key-on | `7FFF…` until the ECU wakes, then fixed values. D2 toggles 02/03 with the engine running | unknown |
| `129` | 20 ms | ECU | D0 high nibble gear (0 = N, 1–6); D0 bit 3 clutch pulled. D7 rolling counter | gear, clutch **confirmed**; gear on dash |
| `12A` | 50 ms | key-on | D1 bit 6 requested throttle map. D0 throttle open/closed bits | map **confirmed** |
| `12B` | 10 ms | key-on | D0–D1 front wheel speed, D2–D3 rear wheel speed (front and rear agree within ~3%). D5–D7 tilt/lean (12-bit pairs) | speed **confirmed**; front wheel on dash (~27.4 counts/mph); lean unverified |
| `12C` | 1 s | key-on | All zero when stopped; two 12-bit values that follow wheel speed (slower, different scale) | candidate |
| `12E` | ~100 ms | key-on | D0 status flag (00/40). D7 counter | unknown |
| `174` `178` `17C` | 10 ms | key-on | Lean/motion sensor: each has two 16-bit values centred near 0x8000 (one offset by gravity), a 4-bit counter in D6 low nibble, and a checksum in D7. `178` D0–D1 and `17C` D4–D5 swing most when leaning the bike | strong candidate |
| `290` | 10 ms | key-on | D0–D1 front brake pressure (rises only with the front lever) | **confirmed** |
| `300` | 200 ms | key-on | Always `0F…` | possibly the offroad ABS dongle; check with it unplugged |
| `450` | 50 ms | key-on | D2 bit 0 traction control button. D4 `09`/`00` with the map button | TC, map **confirmed** |
| `540` | 100 ms | ECU | D1–D2 rpm (slower copy). D3 low nibble gear. D4 bit 0 kickstand up; bit 4 key on, engine not running. D6–D7 coolant, tenths of °C | kickstand, coolant **confirmed**; coolant on dash |
| `541` | 200 ms | ECU | Always `02 80` | ECU status, unknown |
| `550` | 200 ms | key-on | D0 status flag (00/10). D7 counter | unknown |
| `5A0` | 1 s | key-on | D1/D2 change once after the ECU wakes | unknown |

Not found yet: **rear brake** (the pedal changed nothing distinctive).

## Wheel speed scale (`12B`)

Steady stretches from the 2026-10-01 ride, read against the bike's
speedometer:

| Speedometer | Steady stretch | Rear raw | Front raw | Gear |
|---|---|---|---|---|
| 20–25 mph | 219–232 s | ~615 | ~602 | 1 |
| 38–42 mph | 265–270 s | ~1,113 | ~1,095 | 2 |

The only scale that fits both runs is 26.5–29.3 rear counts per mph; the
midpoint ≈ 28 gives 22 and 40 mph. The front wheel reads ~2% lower, so the
dash uses 27.4 front counts per mph (`FRONT_COUNTS_PER_10_MPH` in
`lib/dash_logic/src/ktm_decode.cpp`). Expect about ±5%; a GPS track would
pin it down. The raw value is not a round km/h unit (0.05 and 0.0625 km/h
per count both fall outside the speedometer ranges). Logs now include GPS
speed, which will give an exact calibration.

## Ride markers (`captures/2026-10-01_can_0010.log`)

1 front brake · 2 rear brake · 3 clutch · 4 kickstand · 5 TC button ·
6 throttle map · 7 lean left/right · 8 throttle sweep · 9 roll-on to 5,000 ·
10 blips · 11 steady 20 mph · 12 steady 40 mph · 13 normal ride

## Log quality

Firmware before 2026-10-01 echoed changed frames to Serial, which blocked
the loop about every 10 ms and lost 25–50% of frames (visible as skipped
rolling counters in `120`, `129`, `174`). Slow signals are still usable.

## Reading the bytes yourself

- A frame in the log looks like `(12.345678) can0 120#06A4101000000030`:
  ID `120`, data `06 A4 10 10 00 00 00 30`. D0–D1 = `06A4` = 1700 rpm.
- **Rolling counters** (a byte or nibble stepping by a fixed amount every
  frame) show up in many IDs. A skipped step means a frame was lost, which
  is how frame loss in early logs was measured.
- To find a new signal: hold the screen on the dashboard to drop a marker,
  do the action, and compare which bytes change after the marker but not
  in quiet stretches. That's how the brake, clutch, kickstand, TC and map
  bits above were found.

## Still to find

- Rear brake (the pedal changed nothing distinctive)
- What `300` is: log one key-on with the offroad dongle unplugged
- `121`, `128`, `12C`, `12E`, `541`, `550`, `5A0`: unknown or only partly known
- Lean angle scaling (`12B` D5–D7, and the motion sensor IDs)
