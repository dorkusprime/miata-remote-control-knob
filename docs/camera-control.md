# Camera 1 control

Manual camera preview is an integral knob function, not an optional future peripheral. Double-click requests Camera 1 through EXT IN 1; the resistive steering-remote connection remains responsible for the six audio commands.

## Sony evidence

Sony XAV-9000ES Operating Instructions, Making connections, page 11: EXT IN 1/2 accept a positive interruption signal of +3.3 V to +12 V. Sony Help Guide says enable Camera 1 Interruption to display CAMERA IN 1 on EXT IN 1 assertion. RearCam always takes priority. The guide documents touchscreen Back to cancel an interrupted camera view; it does not establish that removing the trigger restores the prior source. Release to prior content was subsequently user-verified with an external source; touchscreen Back and reverse-exit interactions remain untested.

Sources:
- https://www.retailspecs.com/files/pdf/attachment/121511/XAV9000ES_O.pdf
- https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940244.html
- https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940273.html

## Selected compact hardware

U2 SN74HCT125N channel 4 drives the assembled camera output.

| Connection | Wiring |
| --- | --- |
| U2 pin 13, active-low enable | GND |
| U2 pin 12, input | XIAO D2 / GPIO28 via breakout J7 and component-board J1.8 at I8 |
| R8, 47 kohm | U2 pin 12 to GND; default request low during reset/unplug |
| U2 pin 11, output | J2.3 camera pigtail through R10 1 kohm to EXT IN 1 |
| R9, 47 kohm | U2 pin 11 to GND; holds buffer-side output down if high impedance |
| R10, 1 kohm | Sleeved inline in camera output pigtail, off-board; strain-relieve both wire ends |

R8 and R9 are on the component board. R10 is sleeved in the cable. Use the [pad map](../hardware/sony-remote/pad-map.md) for physical placement.

TI specifies 2 V minimum high input and 0.8 V maximum low input for this 5 V HCT buffer, so 3.3 V GPIO is suitable. At 4.5 V supply, output high is guaranteed at least 3.84 V at 6 mA over SN74 temperature range. With R10 at its 1% maximum of 1010 ohm, a measured Sony trigger load <=0.25 mA leaves a conservative output >=3.58 V after the series drop (total buffer load includes R9). Require >=3.5 V at EXT IN 1 under actual load and <=0.25 mA through R10 before acceptance. If input load exceeds this budget, this version fails acceptance and needs a different driver; do not bypass R10 blindly.

The buffer output must remain within GND..VCC during normal use. Sony input bias/backfeed is undocumented: measure EXT IN 1 with the interface disconnected, then test Sony-on/controller-off and the reverse order. R10 limits current but is not galvanic isolation, automotive surge protection or proof against back-power. Do not connect to reverse-lamp wiring or 12 V. Install only after off-state and transient tests pass. Hardware low default covers reset/unplug, not firmware stuck high; watchdog release is also required.

TI source: https://www.ti.com/lit/ds/symlink/sn74hct125.pdf (pinout, recommended conditions, electrical characteristics).

## Harness and placement

Use the [internal jumper image](../hardware/internal-jumpers.svg) for the XIAO-to-board request and the [P1 hookup table](wiring.md#8-pin-box-harness-p1) for the Sony end. Component-board J2.3 at H9 reaches Sony EXT IN 1 through P1.7 and the inline R10. Sony steering Input 1 has its own P1.5 path. Insulate the unused Sony Input 2 and EXT IN 2 leads.

## Firmware contract

- Double-click toggles a local camera-request variable; it does not certify what Sony displays.
- Request asserted: drive D2 high continuously. Request released: drive low. Never send the camera gesture as a pot resistance.
- Boot/reset, watchdog recovery, encoder disconnect and controller fault release the request; R8 input pull-down provides reset default.
- LED provides a brief acknowledgement only, never a latched camera-state indication.
- Real reverse goes directly to Sony REVERSE IN independently of the MCU. Sony supplies priority; no reverse GPIO or sensing circuit is required for that priority alone.
- Whether Camera 1 resumes after leaving reverse is a bench acceptance question. No claim that firmware clears a request on reverse without a reverse-sensing input.
- The controller implements the camera request separately from steering pulses. The assembled buffer successfully triggered Camera 1 in the [end-to-end test](bench-results-2026-11-12.md). Loaded voltage/current and independent power transitions remain open.

## Video and camera power

Camera video feeds REAR CAMERA IN for actual reversing and CAMERA IN 1 for manual preview. A passive RCA splitter was user-tested on 2026-10-31: switching between reverse with Sony guidelines and Camera 1 without guidelines worked with no reported image-quality degradation. Keep this passive split; no video amplifier or switch is needed for the tested setup. Camera-generated lines remain disabled. Camera must have power during manual preview, so reverse-lamp-only camera power will not satisfy the requirement. Identify camera/DVR and voltage before selecting its switched accessory supply.

## Acceptance and next work

The assembled board measured 0 V off and 5 V on with Sony disconnected. The full bench setup passed manual preview on/off and reverse override. The following checks remain open.

1. Verify reset, fault release and both supply-loss orders.
2. Measure actual Sony input loading and loaded trigger voltage. Check touchscreen Back and reassertion.
3. Record the screen shown on reverse exit with a standing manual request, and verify installed camera power.

## User-observed Sony bench results — 2026-10-28

The [bench report](bench-results-2026-10-28.md) is the authoritative measurement and observation record. External 5 V/resistor triggering, release to previous screen, Camera 1 live picture, guideline separation and reverse priority passed user testing. EXT IN 1 was 0.019 V disconnected and 4.87 V asserted. Trigger load about 0.13 mA is an estimate assuming exact 5 V/1 kohm, not a measured current. No buffer or RCA splitter was involved.

R9 is retained in the current checked candidate, not asserted essential. Its pull-down is redundant while U2 actively drives low and is not a validated power-off safeguard. Removal can be evaluated during assembly without changing the required camera function.

## Passive RCA splitter follow-up — 2026-10-31

The user connected an RCA splitter and reported successful switching between reverse (with lines) and Camera 1 (without lines), with no degradation in image quality. This validates the passive video split in the tested setup; it does not establish assembled HCT125 electrical acceptance or the specific reverse-exit behavior with a continuously asserted manual request. Splitter model and cable lengths were not recorded.
