# Through-hole assembly and acceptance

This procedure applies only to the [MCP41050/HCT125/LCC110 circuit](sony-interface.md). Parts ordered 2026-10-27; no assembly or electrical acceptance has been reported. Existing firmware offers gesture simulation, not a hardware calibration command. Steps requiring SPI or relay control wait for a separate diagnostic; do not assume the controller sketch drives these pins.

## Preparation and orientation

Use the existing solderless protoboard, multimeter, jumpers and regulated 5 V USB supply first. A 10 kohm test resistor is useful for the simulated Sony source; it is a bench aid, not an installed BOM component. A scope is needed to characterize brief supply-loss excursions; a steady multimeter reading cannot prove their absence.

Disconnect Sony, vehicle wiring and USB before changing connections. Label the two DIP-8 devices: the MCP41050 and LCC110 have different pinouts. Viewed from above with notch at the top, DIP pin 1 is upper left; numbering runs down the left and up the right. Confirm the actual marking against each datasheet. The HCT125 is DIP-14. For the ordered onsemi 2N3904BU, the manufacturer identifies lead 1 emitter, 2 base, 3 collector; identify the drawing's viewing orientation rather than assuming a left/right rule.

Check the solderless board's rail continuity: long rails may be split. Place each DIP across its center gap so opposite IC legs are not joined. Keep bypass capacitors close to the corresponding power pins. Do not use the permanent board's old five-relay placement drawing.

## Assembly order

1. **Ground and power, unpowered.** Establish common GND and regulated +5 V. Add C3 10 uF with positive to +5 V, negative to GND. Verify rail polarity and no short. Do not connect raw 12 V. For initial tests use one USB supply path; do not parallel an external 5 V supply with laptop USB VBUS.
2. **U1 pot.** Connect pins 8 +5 V, 4 GND, 5 PB GND, C1 100 nF across 8/4. Leave PA pin 7 unconnected. PW pin 6 is the signal output toward K1. Complete CS/SCK/SI only through U2.
3. **U2 buffer.** Pins 14 +5 V, 7 GND, C2 100 nF across 14/7. Ground active-low enables 1, 4 and 10. Disable unused channel with pin 13 +5 V, pin 12 GND and pin 11 unconnected. Wire input/output pairs 2→3 for CS, 5→6 for clock, 9→8 for MOSI. Outputs 3/6/8 go to U1 pins 1/2/3 respectively. Put R4/R5/R6 47 kohm from each buffer input to GND.
4. **Q1 and optical input.** Q1 emitter GND; collector K1 pin 3; R2 1 kohm from relay-enable cable lead to Q1 base; R7 47 kohm base to GND. +5 V through R1 330 ohm to K1 pin 2. Do not connect this load to a buffer output. No flyback diode is required for the optical LED.
5. **Sony-side path, still disconnected from Sony.** A labeled test point INPUT1 goes through R3 2.7 kohm to K1 pin 5; K1 pin 6 to U1 PW pin 6. K1 pins 7/8 (normally closed) and package NC pins 1/4 stay unconnected. A second test point REMOTE_GND joins shared GND.
6. **Harness.** Verify each conductor against the [six-wire allocation](wiring.md). Leave the optional camera conductor disconnected. With power off, check continuity from each XIAO label to its destination and check that no U2 output goes back to GPIO. Recheck +5 V versus 3V3: Twist uses 3V3; interface uses +5 V.
7. **First power.** Start without Sony or simulated input attached. Check +5 V at both ICs and 3V3 at the Twist. Keep relay-enable low. Stop if current, temperature or rail voltage is unexpected. The approximately 20 mA interface budget is a design allowance, not an observed baseline.

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

Use an interface-only diagnostic once implemented. It must start relay-off, set GPIO pad drive appropriately, hold CS inactive, program the pot while disconnected and require an explicit action to close the relay. Current gesture firmware cannot perform these operations.

With no external voltage source on INPUT1, use resistance mode from INPUT1 to REMOTE_GND. The ICs remain powered for programming and optical activation. Check the meter's resistance-mode test voltage/current first: it must not drive the pot above VDD, below GND or above its ±1 mA wiper limit. If the meter's test conditions are unsuitable or unknown, use a known limited source and voltage measurements instead. Do not probe resistance across a live Sony or simulated-source input.

For each target in the [circuit guide](sony-interface.md#calibration-model-and-firmware-contract), program an initial estimated code with relay open, close it for the measurement, then release. A special bench mode may hold the relay for a meter reading only while Sony is physically disconnected. Never reuse this hold mode for receiver operation. Adjust to the nearest achievable safe code. Repeat readings for reproducibility. Released output should be electrically open apart from leakage; do not treat a continuity buzzer alone as a calibrated resistance check.

Record measured RAB if useful, resistor values, supply voltage, selected codes and total output resistance. No values in the calculation table are calibration results.

## Simulated Sony and current check

Connect a regulated 5 V test source through a known 10 kohm resistor to INPUT1, with common GND. This limits even an accidental short to roughly 0.5 mA. Use voltage mode. Record the voltage at INPUT1, the downstream pot PW voltage and the drop across the 10 kohm resistor. Current is `(Vsource − Vinput) / 10000`; confirm it remains below the prototype 0.5 mA target and the pot's ±1 mA limit. An independent simulated source is needed to test the case where Sony stays live after controller power is lost; a single common supply cannot reproduce that case.

Measure release and startup, MCU reset, I2C disconnect/fault, interrupted SPI, and any diagnostic fault/release control. For rapid movement, verify bounded pulses and release gaps after a backend exists. A software reset does not prove watchdog handling of a stalled MCU.

Capture both power-loss orders and restoration orders, including simulated Sony remaining powered while interface +5 V falls. Observe +5 V and pot PW together. Pot terminals must stay between GND and actual VDD, not merely below a nominal 5 V label. LCC110 release takes time; an unpowered pot can be exposed during that delay. No hold-up or sequencing fix is yet validated. Any violation blocks installation and requires a design revision; steady-state relay-off is insufficient evidence.

## Sony measurements and recognition

Wait for an adequate receiver supply; the earlier supply voltage collapse has no confirmed cause. Follow the Sony installation manual for bench wiring and fuse requirements. Do not power the head unit from the knob converter.

Leave the interface unplugged. In the Sony's Custom steering-wheel mode, measure Input 1 relative to ST Remote GND using DC voltage mode. Record open-circuit voltage, then voltage with a known 10 kohm resistor to ground. Under a steady pull-up model, estimate `Rsource = 10000 × (Vopen / Vloaded − 1)`. For each calibrated command estimate `I = Vopen / (Rsource + Rcommand)`, then verify actual loaded measurements. Unstable signals or a poor fit require further investigation; the model is not a Sony specification. Do not short the input with the meter's current range.

Proceed only when voltage/current are compatible with the pot and power-transition tests pass. Confirm passive command learning first if practical. Connect the active circuit with all outputs released, use low listening volume, then test individual commands with initial 55 ms on / at least 55 ms off timing. Record recognition, repeats, wrong commands and any required timing changes. Confirm manual head-unit controls still work. Software has no acknowledgement from Sony; serial output and RGB feedback cannot certify acceptance.

## Permanent assembly and fit

The proposed ElectroCookie Mini is 38.1 × 50.8 mm with 17 rows, each side connected in five-pad strips and a center gap. Three DIP packages use 4 + 7 + 4 pin rows before clearance. This arithmetic is not a finished fit check. Dry-fit sockets, transistor, passives, connectors, mounting screws and insulation on the actual board. Check underside access, cable bend radius and enclosure clearance before soldering. The [all-component placement](../hardware/sony-remote/placement-full.svg) and [pad map](../hardware/sony-remote/pad-map.md) now specify a complete proposed allocation: ICs, Q1, all passives, distributed pigtails and insulated underside jumpers. The checker confirms logical connectivity and nominal body separation. All component bodies are now on top. R1 stands upright at C15 with a sleeved return through the U2/K1 gap to F13; the power bus now lands at H13. Q1 and R7 move to clear that route, and the Q1 emitter lands at B12. R2/R3 remain on the outer edges, and the four 47 kohm resistors stand upright. All drawn routes use horizontal/vertical segments. The underside wiring is two continuous tapped buses plus four signal wires. See the pad map for exposed tap locations and sleeve all intervening wire sections. This remains a physical dry-fit task: sockets, mounting-hole keepouts, lead availability, forming and overall height are not yet measured. No physically verified permanent map exists yet.

Keep all placement claims provisional until that dry-fit. Check the proposed top and mirrored underside views against the actual board, confirm every labeled net and verify the connected strips. Revise the model and regenerate the image if any physical change is needed. Use insulated jumpers, strain relief and mechanical support independent of the cable. Repeat acceptance after soldering. Never copy the archived isolated-ground layout into this shared-ground circuit.

## Observation record

Copy this table for each physical test session. Empty fields mean untested; date/code changes require a new record.

| Field | Observation |
| --- | --- |
| Date, tester, firmware commit and sketch | Not yet recorded |
| Supply rating/current limit and measured rails | Not yet measured |
| Parts/package markings; cable length and connector orientation | Not yet recorded |
| Relay optical current; released output | Not yet measured |
| Five command codes and measured resistances | Not yet calibrated |
| Simulated-source voltage/current and transition captures | Not yet tested |
| Sony open/loaded voltage and source estimate | Not yet measured |
| Recognition/learning/pulse timing | Not yet tested |
| Reset, fault, restoration and supply-loss results | Not yet tested |
| Permanent fit and mounting | Not yet validated |
