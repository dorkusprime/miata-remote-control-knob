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
- [x] Receive parts and assemble permanent board; replaced defective LCC110 after diode comparison.
- [x] Add a provisional current-design DIP placement image with pin/net reference; not a complete routed layout.
- [x] Place every component and pigtail in a complete proposed pad map; document top / underside views and check all nets, holes and nominal body separation.
- [x] Simplify placement: all component bodies on top, with two tapped buses and four signal wires underneath; all drawn lead and wire routes orthogonal.
- [ ] Dry-fit actual components, sockets, mounting holes, formed leads and bus construction; verify insulation, clearances and overall height.
- [x] Assemble permanent board; user verified rails, continuity corrections and relay LED current.
- [x] Implement interface diagnostic; user verified GPIO outputs and pot codes 64/128/192.
- [ ] Measure and record the five calibrated output resistances.
- [ ] Validate simulated-input current, reset/fault behavior and both supply-loss orders.
- [x] Estimate Sony source impedance from user measurements: about 1.1–1.2 kohm; see bench report.
- [x] Replace historical low-resistance ATT candidate with Sony-learned codes 64–192; measured minimum 16.4 kohm gives estimated steady-state current below 0.5 mA. Receiver acceptance remains pending.
- [ ] Validate Sony recognition, learning and pulse timing at low volume.
- [ ] Validate final cable length, connector orientation, mounting and strain relief.
- [ ] Repeat electrical acceptance after permanent assembly.
- [x] Implement controller v3 bounded pulse queue, fault release, watchdog and real requests; host-tested/compiled.
- [ ] User upload and disconnected controller test, repeat cold-start upload, then controlled Sony learning; integrated hardware behavior remains unverified.

## Camera and installation

Required scope: [Camera 1 hardware/firmware plan](camera-control.md).

- [x] Integrate spare HCT125 camera channel, R8/R9 pull-downs and inline R10 into checked layout/BOM/docs; hardware untested.
- [ ] Allocate harness contact 8 to D2/GPIO28; verify eight-wire connector orientation.
- [x] Implement camera-request backend and host fault-release tests; user upload and integrated hardware acceptance pending.
- [x] Verify external-source EXT IN 1 assertion and release to previous screen (2026-10-28).
- [ ] Test touchscreen Back/retrigger and repeat release with actual buffer.
- [ ] Provide camera power during manual preview, independent of reverse-lamp-only power.

- [ ] Identify actual camera/DVR model and supply requirements.
- [x] User confirmed Camera 1 without camera-generated lines, rear input with Sony guidelines, and reverse priority in sequential-input bench test (2026-10-28).
- [x] User tested passive RCA split: reverse with lines and Camera 1 without lines, switching successfully with no reported image-quality degradation (2026-10-31).
- [x] User verified external 5 V/resistor Camera 1 trigger assertion/release, live picture, and reverse priority (2026-10-28); proposed buffer remains untested.
- [ ] Validate buffer trigger electrical limits and reverse exit behavior.
- [ ] Measure lighter opening and rear clearance.
- [ ] Finalize mechanical support, power protection, and strain relief.

## Bench evidence and next session

[2026-10-28 results](bench-results-2026-10-28.md) separate observations from estimates. Head unit may be packed away pending interface components. Passive RCA split-video and reverse/manual switching passed on 2026-10-31. Next work is staged permanent assembly and a disabled-by-default calibration/backend diagnostic. No firmware was flashed or live outputs enabled during documentation work.

## Latest implementation — 2026-11-06

See [assembled bench results](bench-results-2026-11-05.md) and [controller v3 workflow](gestures.md). Historical checklist descriptions above are evidence of earlier plans, not a reason to repeat already completed strip-continuity tests. Sony learning, integrated physical behavior and power-loss acceptance remain open.

## Learning mode — 2026-11-06

- [x] User uploaded integrated controller and reported knob/LED operation working.
- [x] Implement ten-second knob hold entry/exit, LED flash feedback and three-second learning holds; host testing and compile recorded in gestures.md.
- [ ] User uploads v3.2 and verifies Sony Custom learning/recognition; 55 ms learning attempt did not register.
