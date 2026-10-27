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
- [x] Select PhotoMOS ladder prototype and exact resistor values; document schematic, BOM, pad layout, and meter checks (2026-10-26 client local date).
- [ ] Assemble and measure receiver-disconnected breadboard prototype; record LED current and all five resistance outputs.
- [x] Choose a small soldered protoboard near the Sony for interface construction (user decision, 2026-10-27).
- [x] Draft no-cut ElectroCookie Mini placement and six-wire control/two-wire Sony pinouts.
- [ ] Dry-fit actual components and finish insulation/mounting before permanent assembly.
- [ ] Validate reset defaults and powered-off behavior.
- [x] Define prototype harness/contact numbers; old mux pin allocation superseded.
- [ ] Validate connector mating orientation, cable lengths, and strain relief on hardware.
- [ ] Enable hardware outputs only after interface validation.

## Camera and installation

- [ ] Identify actual camera/DVR model and supply requirements.
- [ ] Test guideline behavior on rear versus Camera 1 input.
- [ ] Test direct versus passive-split picture.
- [ ] Validate camera trigger and reverse priority.
- [ ] Measure lighter opening and rear clearance.
- [ ] Finalize mechanical support, power protection, and strain relief.
