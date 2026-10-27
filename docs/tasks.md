# Development checklist

## Workspace

- [x] Verify local commits, GitHub push, remote branch, and clean working tree (2026-10-26).
- [x] Choose and pin a firmware toolchain/core and Twist library (see firmware/toolchain.json; 2026-10-27).
- [x] Confirm XIAO USB enumeration (Seeed XIAO RP2040, /dev/cu.usbmodem1412301; 2026-10-27).
- [x] Compile serial-only diagnostic for XIAO; no compiler warnings (2026-10-27).

## Encoder milestone: Sony disconnected

- [ ] Dry-fit headers, cable plugs, and enclosure clearances.
- [ ] Verify wiring continuity and absence of shorts while unpowered.
- [ ] Upload a serial diagnostic when explicitly instructed.
- [ ] Scan I2C and verify the Twist address.
- [ ] Read rotation and button state.
- [ ] Verify low-brightness RGB and LED-off behavior.
- [ ] Implement gestures with serial-only command output.

## Sony interface

- [ ] Record supply rating/limit and startup voltage/current. Earlier supplies dropped voltage; cause unknown.
- [x] Included steering-remote adapter is on hand: Input 1, Input 2, ST Remote GND (user observation recorded 2026-10-27).
- [ ] Confirm adapter model and which remote input to use; verify plug-contact mapping if needed.
- [ ] Measure remote voltage and test passive resistor commands/learning.
- [ ] Select circuit and resistor values.
- [x] Choose a small soldered protoboard near the Sony for interface construction (user decision, 2026-10-27).
- [ ] After circuit validation, lay out the protoboard and select its size, connectors, insulation, and mounting.
- [ ] Validate reset defaults and powered-off behavior.
- [ ] Finalize remote harness and connector pinout.
- [ ] Enable hardware outputs only after interface validation.

## Camera and installation

- [ ] Identify actual camera/DVR model and supply requirements.
- [ ] Test guideline behavior on rear versus Camera 1 input.
- [ ] Test direct versus passive-split picture.
- [ ] Validate camera trigger and reverse priority.
- [ ] Measure lighter opening and rear clearance.
- [ ] Finalize mechanical support, power protection, and strain relief.
