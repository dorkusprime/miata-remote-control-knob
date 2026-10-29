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

- [x] User powered Sony successfully from bench supply at nominal 12 V (2026-10-28).
- [ ] Record supply model/rating, current limit, startup current and loaded voltage; previous supply-collapse cause unknown.
- [x] Included steering-remote adapter is on hand: Input 1, Input 2, ST Remote GND (user observation recorded 2026-10-27).
- [ ] Confirm adapter model and which remote input to use; verify plug-contact mapping if needed.
- [x] Measure remote input unloaded and with known load: approximately +3.3 V, 9.86 kohm load, 2.939 V loaded (2026-10-28).
- [x] Record actual steering-learning menu: ten functions including Mute, no camera assignment; see bench report.
- [ ] Test passive resistor commands/learning; a loaded voltage measurement is not command recognition.
- [x] Archive five-relay ladder; select one LCC110 plus digital pot (user direction, 2026-10-27).
- [x] Select MCP41050-I/P + SN74HCT125N + existing LCC110 and NPN driver; all through-hole.
- [x] Document pin-level circuit, current limits, harness, calibration and acceptance procedure.
- [x] User ordered DigiKey components, including intentional spares (2026-10-27).
- [x] Consolidate active docs and archive superseded build instructions.
- [x] Choose a small soldered protoboard near the Sony (prior user decision).
- [ ] Receive and inspect parts; verify actual packages, polarity and pin orientation.
- [x] Add a provisional current-design DIP placement image with pin/net reference; not a complete routed layout.
- [x] Place every component and pigtail in a complete proposed pad map; document top / underside views and check all nets, holes and nominal body separation.
- [x] Simplify placement: all component bodies on top, with two tapped buses and four signal wires underneath; all drawn lead and wire routes orthogonal.
- [ ] Dry-fit actual components, sockets, mounting holes, formed leads and bus construction; verify insulation, clearances and overall height.
- [ ] Assemble the receiver-disconnected permanent board in stages (solderless duplicate optional); verify rails and relay LED current.
- [ ] Implement a separate calibration/driver diagnostic with relay-off defaults; user performs flashing.
- [ ] Measure and record the five calibrated output resistances.
- [ ] Validate simulated-input current, reset/fault behavior and both supply-loss orders.
- [x] Estimate Sony source impedance from user measurements: about 1.1–1.2 kohm; see bench report.
- [ ] Resolve ATT current estimate about 0.59 mA versus previous <=0.5 mA design target; verify actual limits/margins before activation.
- [ ] Validate Sony recognition, learning and pulse timing at low volume.
- [ ] Validate final cable length, connector orientation, mounting and strain relief.
- [ ] Repeat electrical acceptance after permanent assembly.
- [ ] Add a nonblocking bounded backend and watchdog; enable controller outputs only after validation.

## Camera and installation

Required scope: [Camera 1 hardware/firmware plan](camera-control.md).

- [x] Integrate spare HCT125 camera channel, R8/R9 pull-downs and inline R10 into checked layout/BOM/docs; hardware untested.
- [ ] Allocate harness contact 8 to D2/GPIO28; verify eight-wire connector orientation.
- [ ] Implement camera-request backend, reset/fault release and tests after trigger validation.
- [x] Verify external-source EXT IN 1 assertion and release to previous screen (2026-10-28).
- [ ] Test touchscreen Back/retrigger and repeat release with actual buffer.
- [ ] Provide camera power during manual preview, independent of reverse-lamp-only power.

- [ ] Identify actual camera/DVR model and supply requirements.
- [x] User confirmed Camera 1 without camera-generated lines, rear input with Sony guidelines, and reverse priority in sequential-input bench test (2026-10-28).
- [ ] Test direct versus passive-split picture; no RCA splitter on hand.
- [x] User verified external 5 V/resistor Camera 1 trigger assertion/release, live picture, and reverse priority (2026-10-28); proposed buffer remains untested.
- [ ] Validate buffer trigger electrical limits and reverse exit behavior.
- [ ] Measure lighter opening and rear clearance.
- [ ] Finalize mechanical support, power protection, and strain relief.

## Bench evidence and next session

[2026-10-28 results](bench-results-2026-10-28.md) separate observations from estimates. Head unit may be packed away pending components and RCA splitter. Next work is staged permanent assembly and a disabled-by-default calibration/backend diagnostic. No firmware was flashed or live outputs enabled during documentation work.
