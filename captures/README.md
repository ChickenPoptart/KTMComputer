# CAN captures

Raw logs from the bike, in the [log format](../docs/log-format.md)
(candump lines, plus `mark` lines for markers). All are from firmware that
lost 25–50% of frames ([why](../docs/troubleshooting.md#lost-can-frames-fixed)),
so rolling counters skip; slower signals are still usable.

GPS lines are removed from anything published here, since they show where
the bike was.

| File | Length | What happened |
|---|---|---|
| `2026-09-28_first_start_can_0005.log` | 40 s | First key-on: ECU wakes at ~16 s, cold start at ~25 s, idle ~1,700 rpm, engine off at 39 s |
| `2026-10-01_can_0007.log` | 35 s | Key-on |
| `2026-10-01_can_0008.log` | 89 s | Key-on with 5 markers (first attempt at the test plan) |
| `2026-10-01_can_0009.log` | 14 s | Key-on |
| `2026-10-01_can_0010.log` | 6.5 min | **Full test ride with 13 markers** (below) |

## Markers in `2026-10-01_can_0010.log`

| Mark | Action | Found |
|---|---|---|
| 1 | Front brake, squeezed 3× | `290` D0–D1 |
| 2 | Rear brake, 3× | nothing distinctive |
| 3 | Clutch in/out 3× | `129` D0 bit 3 |
| 4 | Kickstand up, then down | `540` D4 bit 0 |
| 5 | TC button | `450` D2 bit 0 |
| 6 | Throttle map changed and back | `12A` D1 bit 6, `120` D4 bit 0, `450` D4 |
| 7 | Leaning left / right | `178`, `17C` swing |
| 8 | Throttle sweeps (engine started in this window) | `120` D2, `121` D1/D3 |
| 9 | Roll-on to ~5,000 rpm | |
| 10 | Throttle blips | |
| 11 | Steady ~20–25 mph (1st gear) | `12B` wheel speeds ~615 raw |
| 12 | Steady ~38–42 mph (2nd gear) | `12B` ~1,113 raw |
| 13 | Normal riding | |

## Reading them

```sh
# CAN frames only
grep ' can0 ' 2026-10-01_can_0010.log

# One ID
grep ' can0 120#' 2026-10-01_can_0010.log
```

Tools that read candump logs (python-can, SavvyCAN, cantools) can open the
files once the `mark` lines are filtered out.
