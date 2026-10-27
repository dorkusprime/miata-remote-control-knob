# Development checklist

## Workspace

- [x] Verify local commits, GitHub push, remote branch, and clean working tree (2026-10-26).
- [x] Choose and pin a firmware toolchain/core and Twist library (see firmware/toolchain.json; 2026-10-27).
- [x] Confirm XIAO USB enumeration (Seeed XIAO RP2040, /dev/cu.usbmodem1412301; 2026-10-27).
- [x] Compile serial-only diagnostic for XIAO; no compiler warnings (2026-10-27).

## Encoder diagnostics and follow-up: Sony disconnected

Initial functional diagnostic complete: USB/I2C, rotation, button, RGB, and repeated Picotool uploads are verified by user observations. Gesture implementation and installation checks remain separate.

- [ ] Dry-fit headers, cable plugs, and enclosure clearances.
- [ ] Verify wiring continuity and absence of shorts while unpowered.
- [x] User uploaded the first serial diagnostic via BOOT-mode UF2 copy (2026-10-27).
- [x] User uploaded and tested diagnostic v2; rotation/buttons/held rotation confirmed in supplied serial output.
- [x] Build Picotool-enabled firmware and add build/upload helper (2026-10-27).
- [x] User installed Picotool-enabled build; subsequent Picotool uploads succeeded (2026-10-27).
- [x] User confirmed two successive USB uploads without button access or unplugging (2026-10-27).
- [x] Verify Twist at 0x3F: working v2 snapshot reads require that address and expected device ID. A separate scan transcript was not supplied.
- [x] Button DOWN/UP events observed by user (2026-10-27).
- [x] V2 reads rotation in both directions and while pressed (user serial output; 2026-10-27). Original diagnostic failure cause remains unconfirmed.
- [x] User reports RGB diagnostic tests worked after r/g/b/o test instructions (2026-10-26, client local date).
- [x] Preserve original diagnostic; implement separate gesture controller with serial-only command output.
- [x] Add live translated monitor with port detection and reconnection.
- [x] Pass 19 host gesture scenarios and 15 monitor tests, including virtual serial testing (2026-10-26 client local date).
- [x] User uploaded controller and reports all gestures functional through the live monitor (2026-10-26 client local date).
- [x] Implement full-brightness idle blue and nonblocking gesture LED feedback, including camera acknowledgement pulse only.
- [x] Pass 10 LED scenarios and compile controller v2 (2026-10-26 client local date).
- [x] User uploaded LED build and reports all feedback working (2026-10-26 client local date).
- [ ] Confirm repeat Picotool uploads with the controller sketch.

## Sony interface

- [ ] Record supply rating/limit and startup voltage/current. Earlier supplies dropped voltage; cause unknown.
- [x] Included steering-remote adapter is on hand: Input 1, Input 2, ST Remote GND (user observation recorded 2026-10-27).
- [ ] Confirm adapter model and which remote input to use; verify plug-contact mapping if needed.
- [ ] Measure remote voltage and test passive resistor commands/learning.
- [x] Archive five-relay ladder; select one LCC110 plus digital pot (user direction, 2026-10-27).
- [x] Document AD8400ARZ50, NPN input driver, shared ground, fresh cable pinout, nominal codes and bench checks.
- [x] Choose a small soldered protoboard near the Sony (prior user decision).
- [x] User chose SOIC-8 hand soldering after practice; retain AD8400 design (2026-10-27).
- [x] Record Amazon candidates for the exact 50 kohm pot, solder-on adapter and practice chip; price/delivery not confirmed.
- [ ] Obtain components and verify actual packages/adapter dimensions; check passives and tools on hand first.
- [ ] Practice soldering one SOIC-8 chip; inspect and verify all eight pin-to-header connections.
- [ ] Solder actual AD8400 onto adapter, inspect and check continuity before adding headers.
- [ ] Assemble Sony-disconnected breadboard; measure LED current and calibrate five resistance settings.
- [ ] Add separate calibration/driver diagnostic; user performs flashing.
- [ ] Validate reset defaults and simulated-input power-loss transitions before installation approval.
- [ ] Dry-fit permanent components and finish a new pad map; old placement is archived.
- [ ] Validate connector orientation, cable length, mounting and strain relief.
- [ ] Validate Sony voltage/current, command recognition and timing.
- [ ] Enable controller outputs only after interface validation.

## Camera and installation

- [ ] Identify actual camera/DVR model and supply requirements.
- [ ] Test guideline behavior on rear versus Camera 1 input.
- [ ] Test direct versus passive-split picture.
- [ ] Validate camera trigger and reverse priority.
- [ ] Measure lighter opening and rear clearance.
- [ ] Finalize mechanical support, power protection, and strain relief.
