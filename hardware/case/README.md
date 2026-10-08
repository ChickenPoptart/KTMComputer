# Case for the 2.8" CYD dash

`cyd-case.stl` is the 3D-printed case for the current build (ESP32-2432S028R
"Cheap Yellow Display"). All parts are laid out on one plate, 207 × 167 mm,
so it prints in one go on a 220 mm bed or larger.

| Part | Size (mm) | What it is |
|---|---|---|
| Dash front | 119 × 55 × 13 | Holds the CYD, with the screen opening |
| Dash back | 119 × 55 × 8 | Back cover |
| Electronics box | 58 × 123 × 17 | Houses the GPS and wiring |
| Posts (×4) | 6.5 × 6.5 × 20 | Standoffs |

The file also contains two zero-thickness slivers left over from export;
slicers ignore them.

## Printing

- **PETG or ASA**, not PLA: a black case on handlebars gets hot enough in
  the sun to warp PLA.
- The GPS antenna works through the plastic (1.5 mm PETG tested fine), so
  keep the GPS box's lid plastic, with no metal above the antenna.
- Leave the SD card slot and USB-C reachable for pulling logs and flashing.

See the [build guide](../../docs/build-guide.md#8-mounting) for mounting.
