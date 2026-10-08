# Build guide

From parts on the bench to a working dash on the bike. Parts are in the
[parts list](parts-list.md); every pin is in [wiring](wiring.md).

## 1. Set up the firmware tools

1. Install [VS Code](https://code.visualstudio.com/) and the
   [PlatformIO extension](https://platformio.org/install/ide?install=vscode).
2. Clone this repo and open the folder in VS Code. PlatformIO downloads the
   ESP32 toolchain and the libraries (TFT_eSPI, TinyGPSPlus) on first build.
3. Run the unit tests on the computer: `pio test -e native`. They should
   all pass.

## 2. Flash the CYD (no wiring yet)

1. Format the MicroSD card **FAT32** and put it in the CYD.
2. Plug the CYD into the computer by USB.
3. Upload: the **→ Upload** button in the PlatformIO toolbar, or
   `pio run -e esp32dev -t upload`. If it sticks at `Connecting....`, hold
   the CYD's **BOOT** button until writing starts.
4. Open the serial monitor (115200 baud) in a terminal:
   `pio device monitor`. You should see:

   ```
   # Recording all CAN frames to SD /can_0001.log
   # CAN listen-only 500k. Frames go to the SD card only
   # 0 past rides on the card
   (0.9) note boot reset_reason=1 can=running sd=ok
   ```

5. Tap the screen: it should cycle Dashboard → Trip → … → CAN pages.
   Double-tap returns to the dashboard.

### Touch calibration

The touch-to-screen mapping is in `lib/dash_logic/src/touch_map.h`,
measured on this build's CYD. If taps land in the wrong place on yours:

1. Temporarily add `Serial.printf("raw %d %d\n", touchRawX, touchRawY);`
   where taps are handled in `src/main.cpp`.
2. Tap the top-left, top-right, bottom-right and bottom-left corners.
3. Put the readings into `TOUCH_RAW_LEFT/RIGHT/TOP/BOTTOM` and update the
   test in `test/test_touch_map`.

## 3. Wire the transceiver and GPS (bench)

1. **Transceiver to CN1:** red → 3V3, yellow → CRX, blue → CTX, black → GND
   ([table](wiring.md#3-cyd-cn1--can-transceiver)). Termination jumper
   **off**.
   - Recommended: tie **CTX to 3.3 V** instead of the blue wire, for a
     guaranteed read-only tap ([why](wiring.md#make-the-tap-truly-read-only)).
2. **GPS:** VCC → CN1 red (3.3 V), GND → P3 GND, TX → P3 IO35, RX not
   connected ([table](wiring.md#4-gps-u-blox-neo-m8n)).
3. Power up over USB and watch the serial monitor. The GPS line
   `# GPS chars=… sats=… fix=…` should show **chars** rising (wiring OK),
   then satellites and `fix=1` near a window or outdoors (2–10 minutes
   the first time, seconds after that).

## 4. Find and check the bike wires

Key off, seat off, at the diagnostic connector / offroad dongle under the
seat. Identify CAN-H, CAN-L, ground and switched 12 V with the meter
([how](wiring.md#finding-the-wires-with-a-meter)). On this bike: CAN-H
blue/black, CAN-L blue/white, ground brown, 12 V tan.

## 5. Build the bike pigtail

1. Splice into the dongle's four wires. **Stagger the joints** an inch
   apart, one piece of heat-shrink per joint, slid on before soldering.
2. **Heat-shrink carefully.** Low heat, one joint at a time, shield the
   neighbours. On this build the heat melted CAN-H into ground inside the
   jacket and shorted the whole bus
   ([details](troubleshooting.md#a-melted-wire-shorted-the-bus)).
3. Tan → fuse → converter +; brown → converter − and the ground conductor
   to the transceiver; blue/black and blue/white → the twisted pair.
4. **Test before plugging into the bike**
   ([checks](wiring.md#checks-before-connecting-to-the-bike)): nothing
   shorted to ground, then ~60 Ω across CAN-H/CAN-L once on the bike.

## 6. Power from the bike

1. Cut the USB-A cable, find +5 V and ground with the meter, and wire them
   to the CYD's **VIN** and **GND** ([table](wiring.md#2-power)).
2. With the key on, check **~5 V at VIN** and **~3.3 V at the transceiver's
   3V3 pin**.

## 7. First key-on

1. Key on. The dash boots; the ECU starts sending about **15 seconds** after
   key-on, so `--` until then is normal.
2. Check the CAN page header: **RUN** with a frame rate (several hundred per
   second), and the footer **REC can_NNNN.log** with KB climbing and
   drop/err at 0.
3. Start the engine: RPM, gear (N in green) and coolant go live; idle shows
   one cyan bar on the tach.
4. If the time spot on the dash shows **CAN FAULT**, the CAN wiring is bad
   (no frames, many errors). See [troubleshooting](troubleshooting.md).

## 8. Mounting

- Case: [`hardware/case/cyd-case.stl`](../hardware/case/README.md), printed in
  PETG or ASA. Leave the SD slot and USB port reachable.
- GPS antenna up, under plastic, away from metal.
- Strain relief on every cable where it enters the case and at every splice.
- In rain: a freezer bag over the case works; the touchscreen works
  through it.
