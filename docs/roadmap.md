# Roadmap

## Done

- Read-only CAN sniffer with SD logging (every frame, candump format)
- Live dashboard: RPM and colour-zoned tach, shift light, gear, wheel
  speed, coolant
- CAN signal decoding for RPM, gear, coolant, wheel speed, brake, clutch,
  kickstand, TC and map buttons ([CAN signals](can-signals.md))
- Event markers (hold on the dashboard)
- GPS (NEO-M8N): fix, satellites, heading, elevation, local time with
  daylight saving; every sentence logged
- Trip page, elevation / speed / coolant charts, RPM zone breakdown
- Ride history: every ride saved, `<` `>` to browse; double tap to the
  dashboard
- Automatic log rotation when the card fills
- CAN error guard and loop profiling notes
- Running on the bike from VIN power, with the bus short repaired (2026-10-05)

## Next

- **Calibrate wheel speed against GPS speed** from a ride log, replacing
  the ±5% estimate
- **Read-only hardware change:** tie the transceiver's CTX to 3.3 V
  ([why](wiring.md#make-the-tap-truly-read-only))
- **GPX export** script for the computer: ride tracks coloured by speed or RPM
- More signals: rear brake, `300` (check with the dongle unplugged),
  lean angle, and the unknown IDs
- 3D-printed case and handlebar mount

## Maybe later

- GPS at 10 Hz (needs the GPS RX wire, from a freed RGB LED pin)
- BMP280 barometer for smoother elevation
- Magnetometer for heading at a standstill
- Live displays for the newly found signals (map, TC, kickstand warning)
