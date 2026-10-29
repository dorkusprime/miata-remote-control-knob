# Camera 1 control — required, proposed

Manual camera preview is an integral knob function, not an optional future peripheral. Double-click requests Camera 1 through EXT IN 1; the resistive steering-remote connection remains responsible for volume, ATT and track commands.

## Sony evidence

Sony XAV-9000ES Operating Instructions, Making connections, page 11: EXT IN 1/2 accept a positive interruption signal of +3.3 V to +12 V. Sony Help Guide says enable Camera 1 Interruption to display CAMERA IN 1 on EXT IN 1 assertion. RearCam always takes priority. The guide documents touchscreen Back to cancel an interrupted camera view; it does not establish that removing the trigger restores the prior source. Release to prior content was subsequently user-verified with an external source; touchscreen Back and reverse-exit interactions remain untested.

Sources:
- https://www.retailspecs.com/files/pdf/attachment/121511/XAV9000ES_O.pdf
- https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940244.html
- https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940273.html

## Selected compact hardware

Reuse U2 SN74HCT125N channel 4. No K2 or Q2 is added. The earlier second-LCC110 proposal is superseded.

| Connection | Wiring |
| --- | --- |
| U2 pin 13, active-low enable | GND (formerly +5 V; remove old connection) |
| U2 pin 12, input | XIAO D2 / GPIO28, harness contact 8 (formerly GND; remove old connection) |
| R8, 47 kohm | U2 pin 12 to GND; default request low during reset/unplug |
| U2 pin 11, output | J2.3 camera pigtail through R10 1 kohm to EXT IN 1 |
| R9, 47 kohm | U2 pin 11 to GND; holds buffer-side output down if high impedance |
| R10, 1 kohm | Sleeved inline in camera output pigtail, off-board; strain-relieve both wire ends |

R8/R9 fit on top of the same 38.1 × 50.8 mm ElectroCookie Mini. R10 lives in the cable, with insulated joints, rather than adding a chip or another board. No component bodies are added underneath. Current layout JSON/SVG/pad map include the camera nets and two board resistors. All 16 board bodies and all modeled lead routes pass the checker; actual socket/lead fit still requires dry-fitting. C2/C3 and power/ground bus taps changed; do not build from the previous diagram. J1.2 +5 V now lands H1; C3 positive J6 and negative J10.

TI specifies 2 V minimum high input and 0.8 V maximum low input for this 5 V HCT buffer, so 3.3 V GPIO is suitable. At 4.5 V supply, output high is guaranteed at least 3.84 V at 6 mA over SN74 temperature range. With R10 at its 1% maximum of 1010 ohm, a measured Sony trigger load <=0.25 mA leaves a conservative output >=3.58 V after the series drop (total buffer load includes R9). Require >=3.5 V at EXT IN 1 under actual load and <=0.25 mA through R10 before acceptance. If input load exceeds this budget, this version fails acceptance and needs a different driver; do not bypass R10 blindly.

The buffer output must remain within GND..VCC during normal use. Sony input bias/backfeed is undocumented: measure EXT IN 1 with the interface disconnected, then test Sony-on/controller-off and the reverse order. R10 limits current but is not galvanic isolation, automotive surge protection or proof against back-power. Do not connect to reverse-lamp wiring or 12 V. Install only after off-state and transient tests pass. Hardware low default covers reset/unplug, not firmware stuck high; watchdog release is also required.

TI source: https://www.ti.com/lit/ds/symlink/sn74hct125.pdf (pinout, recommended conditions, electrical characteristics).

## Harness and placement

J1 contacts 1–6 keep their electrical functions; contact 7 is second GND; contact 8 CAMERA_REQUEST from D2/GPIO28. Physical J1.2 board landing changes to H1. J2 gains contact 3: CAMERA_BUFFER from J9 through inline R10 to Sony EXT IN 1. Do not confuse EXT IN 1 with steering Input 1. Sony Input 2 and EXT IN 2 remain insulated.

The current placement and pad map replace the remote-only layout. They include R8/R9; R10 is explicitly an off-board cable component. There are still six underside wire pieces, now 18 wire solder landings. BG has seven taps, BV three. Insulate the power bus as it passes the former G7 power tap: row 7 right is now ground.

## Firmware contract

- Double-click toggles a local camera-request variable; it does not certify what Sony displays.
- Request asserted: drive D2 high continuously. Request released: drive low. Never send the camera gesture as a pot resistance.
- Boot/reset, watchdog recovery, encoder disconnect and controller fault release the request; R8 input pull-down provides reset default.
- LED provides a brief acknowledgement only, never a latched camera-state indication.
- Real reverse goes directly to Sony REVERSE IN independently of the MCU. Sony supplies priority; no reverse GPIO or sensing circuit is required for that priority alone.
- Whether Camera 1 resumes after leaving reverse is a bench acceptance question. No claim that firmware clears a request on reverse without a reverse-sensing input.
- Hardware backend remains disabled pending acceptance. Existing serial camera-toggle intent remains a simulation.

## Video and camera power

Camera video feeds REAR CAMERA IN for actual reversing and CAMERA IN 1 for manual preview. Test a passive RCA Y against direct connection; add a video switch/buffer only if loading degrades the picture. Camera-generated lines were disabled and Sony guideline separation was user-verified with separate input connections; repeat with the splitter. Camera must have power during manual preview, so reverse-lamp-only camera power will not satisfy the requirement. Identify camera/DVR and voltage before selecting its switched accessory supply.

## Acceptance and next work

1. Dry-fit the revised placement and sleeved cable resistor before permanent assembly.
2. With Sony disconnected, verify a controlled 0/5 V output and pull-down behavior; test reset, fault and both supply-loss orders.
3. External-source Sony trigger recognition passed. Repeat loading and recognition with the assembled HCT125 output.
4. External-source release returned to prior content. Repeat with buffer; test touchscreen Back and reassertion; keep local request distinct from actual screen state.
5. Reverse priority and separate-input guidelines passed. Test reverse exit with a standing manual request, installed camera power, and split-video quality.
6. Implement/test nonblocking camera backend and release-on-fault; user performs flashing and energizing.

The buffer circuit has not been assembled or connected to Sony. The updated board netlist and nominal geometry pass software checks. Sony-side tests below used independent bench supplies, not the proposed buffer.

## User-observed Sony bench results — 2026-10-28

The [bench report](bench-results-2026-10-28.md) is the authoritative measurement and observation record. External 5 V/resistor triggering, release to previous screen, Camera 1 live picture, guideline separation and reverse priority passed user testing. EXT IN 1 was 0.019 V disconnected and 4.87 V asserted. Trigger load about 0.13 mA is an estimate assuming exact 5 V/1 kohm, not a measured current. No buffer or RCA splitter was involved.

R9 is retained in the current checked candidate, not asserted essential. Its pull-down is redundant while U2 actively drives low and is not a validated power-off safeguard. Removal can be evaluated during assembly without changing the required camera function.
