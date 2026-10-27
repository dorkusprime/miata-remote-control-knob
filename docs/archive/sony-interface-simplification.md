> ARCHIVED — superseded. Do not build, order parts, or assign cable contacts from this document. Use the [current design](../sony-interface.md). Statements below describe historical decisions.

> Research history only. These earlier MCP candidates preceded the [AD8400 proposal](sony-interface-ad8400.md); neither is the selected MCP41050 circuit.

# Digital-potentiometer simplification research

Research date: 2026-10-27. Proposed alternative, not an assembled or validated interface. Existing firmware outputs remain disabled. The five-PhotoMOS layout remains an archived candidate pending a replacement schematic; do not buy its full BOM solely on the basis of the earlier recommendation.

## Voltage evidence

- PanuWorld's builder reports that a working Sony MDX-C5960R adapter reads the remote resistance at a voltage below the adapter's 5 V PIC supply. This is first-hand evidence for that older Sony, not a voltage specification for the XAV-9000ES: https://panuworld.net/hardware/fordsony/index.htm (Operation section).
- Pounamu Electronics' RE_SWC Rev C.1 schematic, dated 13 November 2025, uses U105 MCP4131-104E/SN (100 kohm), powered from USB VBUS, with a 100 nF bypass capacitor. Its wiper goes to SWC_IN/OUT and its B terminal to SWC_GND. Q104 AO3400A switches SWC_GND to circuit ground. Its separate Sony firmware selects the five previously documented resistance targets and pulses the ground-enable output. The maker advertises Sony compatibility. This establishes an existing implementation approach, not this receiver's exact voltage or power-off behavior.
  - [Hardware repository](https://github.com/pounamu-electronics/RE_SWC)
  - [Rev C.1 schematic](https://github.com/pounamu-electronics/RE_SWC/blob/main/RE_SWC%20Controller%20PCB/Design/RE_SWC_RevC.pdf)
  - [Sony firmware](https://github.com/pounamu-electronics/RE_SWC_FW/blob/main/lib/headunit_swc/src/sony/sony_swc.cpp)
  - [Maker's description](https://lectronz.com/products/car-stereo-volume-knob-kit-re_swc)
- Sony's [XAV-9000ES settings guide](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940274.html) supports Custom steering-wheel settings but does not specify the sensing voltage. No authoritative exact-model voltage was established in this research.
- A search result reporting 3.3 V in an Arduino discussion concerned a **Pioneer AVH-A205BT**, not a Sony. Do not use it as evidence of the Sony's voltage.

## Recommended direction

Use one programmable resistor and one normally-open series disconnect on **Sony Input 1**, ahead of every potentiometer terminal connection. Connect the potentiometer's ground end and supply ground to ST Remote GND. Leave Input 2 insulated. This changes the previous isolated-ground arrangement: the digital-pot approach uses a shared logic/Sony ground.

The signal-side disconnect is intended to release commands and prevent the live Sony signal reaching the unpowered pot. Disconnecting only the pot's ground end does not establish that protection. The disconnect must default open through MCU reset and power loss, and open before the pot supply falls outside its allowed range. Brownout sequencing, leakage, and control-line back-powering still need a schematic review and bench checks; a normally-open relay alone does not prove all power-transition behavior.

Candidate: **MCP4151-503E/P**, 50 kohm, 8-bit, through-hole DIP-8. Its nominal step is 50,000/256 = 195.3 ohms, finer than the reference's 100 kohm 7-bit part. MCP4131-503E/P is a lower-resolution alternative. Availability has not been sourced. Calibrate actual resistances with a meter: the chip's end-to-end tolerance is ±20%, and wiper resistance adds an offset. Nominal code calculations alone are not calibration.

Microchip specifies analog terminals between VSS and VDD, a maximum terminal current of 2.5 mA, and a midscale power-on setting. At 5 V supply its minimum digital-high threshold is 2.25 V, compatible with 3.3 V commands. However, this does not make a 5 V return signal safe for the RP2040: review the shared SDI/SDO pin, internal pull-ups, reset states, and power sequencing before assigning direct SPI wiring. The datasheet explicitly discusses split-supply interfaces. [Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/22060b.pdf), printed pages 5, 9, 38, and 59.

One existing LCC110 could supply the normally-open disconnect. Its drive circuit must be sized from its own datasheet; the previous AQY212EH drive circuit cannot simply be reused. A complete replacement BOM and placement are not yet released.

## Final receiver checks

When the Sony can be powered, leave the controller disconnected. Set Custom steering-wheel mode on. Use DC-voltage mode: black probe ST Remote GND, red probe Input 1; record the open-circuit voltage. Never use resistance mode across a powered Sony input.

If needed, measure again with a known 10 kohm resistor from Input 1 to ST Remote GND. With a steady simple pull-up, estimate source resistance as 10,000 × (Vopen/Vloaded − 1), and loaded current as Vloaded/10,000. These are estimates under that circuit assumption; unstable readings require further investigation. Do not use the meter's current range to short the input.

A measured voltage comfortably within the pot's actual powered rail supports proceeding with the 5 V candidate. A higher, negative, or varying signal needs investigation before connection. Then verify calibrated command resistances, idle disconnection, startup/reset, both power-loss orders, and actual Sony recognition. No exact-model electrical or command acceptance is claimed yet.
