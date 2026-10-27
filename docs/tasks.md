# Development checklist

## Workspace

- [x] Verify local commits, GitHub push, remote branch, and clean working tree (2026-10-26).
- [ ] Choose and pin a firmware toolchain/core and Twist library.
- [ ] Confirm XIAO USB enumeration.

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
- [ ] Confirm RC-SR1 availability and pinout.
- [ ] Measure remote voltage and test passive resistor commands/learning.
- [ ] Select circuit and resistor values.
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
