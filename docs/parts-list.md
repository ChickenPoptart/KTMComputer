# Parts list

What this build uses. Exact brands mostly don't matter; the notes say what
does.

## Electronics

| Part | Qty | Notes |
|---|---|---|
| **ESP32-2432S028R** ("Cheap Yellow Display", CYD) | 1 | ESP32 with a 2.8" 320×240 ILI9341 screen, resistive touch, MicroSD slot. The firmware's pins and touch calibration are for this board |
| **SN65HVD230 CAN transceiver board** | 1 | 3.3 V CAN transceiver, chip marked "VP230". Leave its 120 Ω termination jumper **off** (the bike's bus is already terminated) |
| **u-blox NEO-M8N GPS breakout** with ceramic patch antenna | 1 | Breakout marked NEO-M8N-0-10. Pins: VCC, GND, TX, RX. Runs from the CYD's 3.3 V. No compass on this one |
| **12 V → 5 V converter** with USB-A output | 1 | Any automotive buck converter rated 1 A or more at 5 V. Use USB-A, not a USB-C PD charger |
| **Inline fuse holder + 1–2 A fuse** | 1 | On the converter's 12 V input |
| **MicroSD card**, FAT32 | 1 | 32 GB or smaller (larger cards ship as exFAT, which the board can't read). Holds hundreds of hours of logs |

## Wire and connectors

| Part | Notes |
|---|---|
| **Cat5e or Cat6 cable** | For the run from the bike tap to the transceiver. CAN-H and CAN-L go on **one twisted pair**; a spare conductor carries ground |
| **USB-A cable** (sacrificial) | Cut to make the 5 V lead from the converter to the CYD's VIN. Inside: +5 V (gray in the one used), ground (bare drain wire/shield), data wires unused |
| **JST 1.25 mm 4-pin pigtails** (also sold as MX1.25 / Molex PicoBlade-compatible) | Fit the CYD's CN1, P3 and power connectors. One cable usually comes with the CYD. Not the same as 2.0 mm JST-PH |
| **Heat-shrink tubing**, assorted | One piece per joint. See the [warning in troubleshooting](troubleshooting.md#a-melted-wire-shorted-the-bus) about heat |
| **Solder**, flux, thin hookup wire (28–30 AWG for fine joints) | |
| Zip ties | Strain relief at every joint |
| Quick disconnect (optional) | Any cheap latching connector between the bike pigtail and the dash, e.g. JST-SM 4-pin. Put the socket half on the bike side |

## Already on the bike

| Part | Notes |
|---|---|
| **Offroad ABS dongle** | Plugs into the 6-pin diagnostic connector under the seat. The dash taps the dongle's wires, so the dongle must stay plugged in |

## Tools

| Tool | Used for |
|---|---|
| Multimeter (a basic manual-range one is fine) | Finding the CAN pair, checking power, finding shorts |
| Soldering iron | All joints |
| Heat gun or lighter | Heat-shrink, carefully |
| Computer with VS Code + PlatformIO | Building and flashing the firmware |
| USB cable for the CYD | Flashing and serial monitor |

## Planned / optional

| Part | Why |
|---|---|
| 3D-printed case (PETG or ASA, not PLA) | PLA warps in the sun. The GPS works through 1.5 mm of PETG |
| Handlebar mount (e.g. RAM 1" ball with an AMPS plate) | Quick removal when it rains |
| BMP280 barometer | Smoother elevation than GPS alone |
| Magnetometer | Compass heading while stopped (hard to calibrate near the frame) |
