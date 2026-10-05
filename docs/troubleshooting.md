# Troubleshooting

Problems hit while building this, how each was tracked down, and the fix.
Start with the [quick checks](#quick-checks).

## Quick checks

| Symptom | Look at |
|---|---|
| Dash shows `--` for everything right after key-on | Normal for ~15 s: the ECU starts sending after the ABS unit |
| **CAN FAULT** in red where the time goes | [CAN fault](#can-fault-on-the-dash) |
| No CAN frames, both CAN lines ~0 V | [A melted wire shorted the bus](#a-melted-wire-shorted-the-bus) |
| Screen frozen, taps do nothing (on the bike only) | [The dash froze on VIN power](#the-dash-froze-on-vin-power) |
| `No SD card: not recording` | Card not fully clicked in, or formatted exFAT (needs FAT32, 32 GB or less) |
| GPS `chars=0` | GPS TX not on IO35, or no power to the GPS |
| GPS `chars` rising but `sats=0` | It can't see the sky. Outdoors, antenna up; first fix can take 10 minutes |
| Upload stuck at `Connecting....` | Hold the CYD's BOOT button until writing starts |
| Taps land in the wrong place | Redo the [touch calibration](build-guide.md#touch-calibration) |

## A melted wire shorted the bus

**Symptom:** the dash got no CAN frames at all. With the key on, CAN-H and
CAN-L both read about **0.07 V** to ground, even back-probed at the dongle
with the dash unplugged. A healthy bus sits around 2.5 V.

**How it was found:** with the key off, CAN-H measured **~0 Ω to ground**.
Splitting the wiring in half each time: the short stayed with the dash
disconnected, stayed with the quick disconnect cut off, and turned out to be
in the dongle-side splices. Heat from shrinking the tubing had melted the
insulation between CAN-H and the ground wire inside the jacket.

**Fix:** cut back past the damage, insulate each wire separately,
re-splice with staggered joints, and use low heat with the neighbouring
wires shielded. Re-test: CAN-H and CAN-L to ground open, ~60 Ω between
them once on the bike, ~2.5 V with the key on.

**Note:** a shorted bus also stops the bike's own modules talking to each
other. After a fix, if the ABS behaves oddly, have its fault codes cleared.

## The dash froze on VIN power

**Symptom:** on the bike, the screen sat on the dashboard and ignored taps.
On the computer (USB power) everything worked.

**What the logs showed:** the GPS sends ~8 sentences a second, but only one
every 1–2 seconds reached the log, some garbled. The main loop was running
about once a second instead of tens of thousands of times.

**Suspected cause:** it started exactly when power moved from the USB port
to the VIN pin. Without USB, the CYD's USB-serial chip is unpowered and can
hold the ESP32's serial receive pin (GPIO 3) low, which looks like an endless
stream of broken characters, each one an interrupt.

**Fix (in the firmware):** the serial receive pin is disconnected
internally; the dash never reads from serial. Output and flashing still
work.

**Status:** the bike-side short above was found at the same time, so this
fix **still needs a ride to confirm**. If it recurs, the `loop` notes in the
log (every 10 s) show which part of the loop is slow, or whether all of it
is (something outside the loop stealing time).

## CAN fault on the dash

If the CAN controller sees a flood of bus errors with few or no real frames
(transceiver unpowered, CAN-H/CAN-L swapped, no ground), the ESP32's driver
handles each error in an interrupt, which can starve everything else. The
firmware's guard (`lib/dash_logic/src/can_guard.h`) switches CAN off for 10
seconds when errors outnumber frames, then tries again. Meanwhile the dash
stays usable and shows **CAN FAULT**. The log gets `note CAN error flood …`
lines.

**Check:** transceiver 3V3 ~3.3 V, transceiver GND to battery − ~0 Ω,
CAN-H 2.5–3.5 V and CAN-L 1.5–2.5 V with the key on, yellow/blue on
CRX/CTX.

## Lost CAN frames (fixed)

**Symptom:** logs held ~350 frames/s, and rolling counters inside messages
skipped (25–50% of frames lost).

**Cause:** early firmware echoed every changed frame to the USB serial port.
At 115200 baud that couldn't keep up, and writes blocked the loop about
every 10 ms (an earlier "is there room?" check didn't help: the serial
library reports free space that doesn't account for its own overhead).

**Fix:** frames go to the SD card only; serial carries only status lines.

## Termination

The bike's bus already has both 120 Ω terminators: **~60 Ω** across CAN-H
and CAN-L with the key off. Keep the transceiver's termination jumper off.
If you measure ~120 Ω (one terminator missing), fitting the jumper restores
it; ~40 Ω means three, so take one off.
