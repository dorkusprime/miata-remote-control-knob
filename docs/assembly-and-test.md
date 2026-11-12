# Through-hole assembly and acceptance

This procedure applies only to the [MCP41050/HCT125/LCC110 circuit](sony-interface.md). The board is assembled. See the [November 12 bench record](bench-results-2026-11-12.md) for end-to-end results and links to the initial electrical measurements. The separate [interface diagnostic](interface-diagnostic.md) provides gated bench controls for SPI, relay and camera testing.

## Connect and check the box

1. Disconnect Sony, vehicle wiring and USB while making connections. Use the [internal jumper image](../hardware/internal-jumpers.svg) and the [box hookup guide](wiring.md) to connect and verify the jumpers, P1 and P2.
2. Follow the [preconnection checks](wiring.md#before-connecting-the-sony) for unpowered continuity, rail voltage and Twist input. Use the regulated supply path. Keep laptop VBUS separate from the converter output and leave Sony disconnected for the initial checks.
3. Use the staged checks below for calibration, reset and power-order testing. Keep the relay released at power-up and stop if rail voltage, current or temperature is unexpected.

## Staged electrical checks

| Gate | Check | Required observation |
| --- | --- | --- |
| A: unpowered | Visual inspection, rail and net continuity | Correct polarity; no accidental strip bridges; connector labels agree |
| B: powered, relay released | Rails and relay-control voltages | Interface target 4.75–5.25 V; transistor not enabling relay |
| C: relay drive | Controlled pulse with Sony disconnected | About 10.5 mA nominal optical LED current; clean release |
| D: calibration | Measure output for each programmed command | Recorded nearest achievable resistance and release behavior |
| E: simulated source | Loaded voltage/current and power transitions | Wiper within current/voltage limits, including transitions |
| F: actual Sony | Input voltage/current, passive learning, then active commands | Safe electrical levels and reliable receiver recognition |
| G: final assembly | Repeat A–F as appropriate after soldering/cabling | No regression with final wiring, reset or power ordering |

Measure LED current via voltage across the 330 ohm resistor divided by its measured resistance. This avoids moving the meter into current mode in the circuit. Relay-on nominal estimates assume a typical LED drop; measure rather than treating that estimate as a tolerance guarantee.

## Calibration with Sony disconnected

Use the [interface diagnostic](interface-diagnostic.md). It must start relay-off, set GPIO pad drive appropriately, hold CS inactive, program the pot while disconnected and require an explicit action to close the relay.

With no external voltage source on INPUT1, use resistance mode from INPUT1 to REMOTE_GND. The ICs remain powered for programming and optical activation. Check the meter's resistance-mode test voltage/current first: it must not drive the pot above VDD, below GND or above its ±1 mA wiper limit. If the meter's test conditions are unsuitable or unknown, use a known limited source and voltage measurements instead. Do not probe resistance across a live Sony or simulated-source input.

For each target in the [circuit guide](sony-interface.md#calibration-model-and-firmware-contract), program an initial estimated code with relay open, close it for the measurement, then release. A special bench mode may hold the relay for a meter reading only while Sony is physically disconnected. Never reuse this hold mode for receiver operation. Adjust to the nearest achievable safe code. Repeat readings for reproducibility. Released output should be electrically open apart from leakage; do not treat a continuity buzzer alone as a calibrated resistance check.

Record measured RAB if useful, resistor values, supply voltage, selected codes and total output resistance. No values in the calculation table are calibration results.

## Simulated Sony and current check

Connect a regulated 5 V test source through a known 10 kohm resistor to INPUT1, with common GND. This limits even an accidental short to roughly 0.5 mA. Use voltage mode. Record the voltage at INPUT1, the downstream pot PW voltage and the drop across the 10 kohm resistor. Current is `(Vsource − Vinput) / 10000`; confirm it remains below the prototype 0.5 mA target and the pot's ±1 mA limit. An independent simulated source is needed to test the case where Sony stays live after controller power is lost; a single common supply cannot reproduce that case.

Measure release and startup, MCU reset, I2C disconnect/fault, interrupted SPI, and any diagnostic fault/release control. For rapid movement, verify bounded pulses and release gaps. A software reset does not prove watchdog handling of a stalled MCU.

Capture both power-loss orders and restoration orders, including simulated Sony remaining powered while interface +5 V falls. Use a scope to observe +5 V and pot PW together through the transition. Pot terminals must stay between GND and actual VDD, not merely below a nominal 5 V label. LCC110 release takes time; an unpowered pot can be exposed during that delay. No hold-up or sequencing fix is yet validated. Any violation blocks installation and requires a design revision; steady-state relay-off is insufficient evidence.

## Sony measurements and recognition

Sony has now powered successfully at nominal 12 V from the new bench supply; rating/current/loaded voltage remain to be recorded. The earlier supply voltage collapse has no confirmed cause. Follow the Sony installation manual for bench wiring and fuse requirements. Do not power the head unit from the knob converter.

Leave the interface unplugged. In the Sony's Custom steering-wheel mode, measure Input 1 relative to ST Remote GND using DC voltage mode. Record open-circuit voltage, then voltage with a known 10 kohm resistor to ground. Under a steady pull-up model, estimate `Rsource = 10000 × (Vopen / Vloaded − 1)`. For each calibrated command estimate `I = Vopen / (Rsource + Rcommand)`, then verify actual loaded measurements. Unstable signals or a poor fit require further investigation; the model is not a Sony specification. Do not short the input with the meter's current range.

Proceed only when voltage/current are compatible with the pot and power-transition tests pass. Confirm passive command learning first if practical. Connect the active circuit with all outputs released, use low listening volume, then test individual commands at the controller's current timing. The [gesture guide](gestures.md#real-output-backend--2026-11-06) records the 30 ms press and 30 ms release defaults and how to tune them. Record recognition, repeats, wrong commands and any required timing changes. Confirm manual head-unit controls still work. Software has no acknowledgement from Sony; serial output and RGB feedback cannot certify acceptance.

## Board assembly reference

For a replacement or rework, use the [all-component placement](../hardware/sony-remote/placement-full.svg), [pad map](../hardware/sony-remote/pad-map.md) and [circuit pin table](sony-interface.md#physical-pin-wiring). The ElectroCookie Mini has separate five-pad strips on each side of the center gap. Check continuity across each strip. Confirm the DIP notches, Q1 emitter/base/collector and C3 polarity against the actual parts before powering the board.

Dry-fit sockets, headers, sleeved leads, fasteners and insulation in the enclosure. Use mechanical support and strain relief at the cable exits. Verify the actual boards against the top and mirrored underside views. Update and regenerate the checked placement if any pad allocation changes. The layout checks cover connectivity and nominal component spacing. Final mounting height, cable clearance and electrical acceptance require checks on the build.

## Observation record

Copy this table for a new test session. See the [assembled-board record](bench-results-2026-11-05.md) for prior observations.

| Field | Observation |
| --- | --- |
| Date, tester, firmware commit and sketch | |
| Supply rating/current limit and measured rails | |
| Parts/package markings; cable length and connector orientation | |
| Relay optical current; released output | |
| Six command codes and measured resistances | |
| Simulated-source voltage/current and transition captures | |
| Sony open/loaded voltage and source estimate | |
| Recognition/learning/pulse timing | |
| Reset, fault, restoration and supply-loss results | |
| Permanent fit and mounting | |

## Camera acceptance

Follow [camera-control.md](camera-control.md): receiver-disconnected low/high/reset checks, then Sony bias/loading, >=3.5 V active trigger, <=0.25 mA load and both power-off orders. Verify trigger-release behavior before enabling a toggle. Updated C2/C3 pins and bus taps are in pad-map.md. R10 is sleeved inline in the camera pigtail.

## Existing receiver evidence

Consult [bench report](bench-results-2026-10-28.md) before repeating Sony-only tests. Camera assertion/release, live picture, guideline separation and reverse priority passed with external supplies. Repeat through the assembled interface; do not mark electrical gates passed from external-source results. Check the active profile's [wiper current limits](sony-interface.md#acceptance-boundaries-before-installation) before enabling commands. Passive RCA splitter image quality and reverse/manual switching passed user testing on 2026-10-31; the assembled buffer passed functional switching and reverse override on November 12. Loaded trigger voltage/current and fault-transition measurements remain open.
