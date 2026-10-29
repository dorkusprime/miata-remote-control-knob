# Wiring and cable plan

## Encoder: hardware I2C

| Twist | XIAO board label | RP2040 GPIO | Purpose |
| --- | --- | --- | --- |
| GND | GND | — | Ground |
| 3.3V | 3V3 | — | Encoder and LED power |
| SDA | D4 | GPIO6 | I2C1 data |
| SCL | D5 | GPIO7 | I2C1 clock |
| INT | Unconnected initially | — | Optional active-low event notification; poll initially |
| RST | Unconnected | — | Reset input |

The Twist operates at 3.3 V and includes I2C pull-ups. Its two Qwiic connectors duplicate the four main power/data signals.

The XIAO GND/3V3 pair is adjacent on one edge; D4/D5 is adjacent on the opposite edge. The full connection is not a straight-through board-to-board header.

Preferred baseline: one short four-wire cable, one four-position housing at the Twist, splitting into two two-position housings at the XIAO. A directly mated power pair plus two signal wires was discussed but is not a finalized mechanical layout. Do not force-fit boards or treat two header pins as mechanical support.

Both use 2.54 mm header spacing. Twist header pins project away from the knob; XIAO pins project toward its component/USB-C side, subject to a dry-fit. Other XIAO D pins may receive headers and remain disconnected.

Never connect Twist power to 5V/VBUS. Verify continuity by signal label rather than connector orientation. Keep USB-C, BOOT, and RESET accessible.

## XIAO external header reference

Component side facing you, USB-C at top; each edge listed top to bottom:

| Edge | Label | GPIO | Alternate function |
| --- | --- | --- | --- |
| Left | D0 / A0 | 26 | ADC |
| Left | D1 / A1 | 27 | ADC |
| Left | D2 / A2 | 28 | ADC |
| Left | D3 / A3 | 29 | ADC |
| Left | D4 / SDA | 6 | I2C1 SDA |
| Left | D5 / SCL | 7 | I2C1 SCL |
| Left | D6 / TX | 0 | UART TX |
| Right | 5V / VBUS | — | USB 5 V rail |
| Right | GND | — | Ground |
| Right | 3V3 | — | 3.3 V output |
| Right | D10 / MOSI | 3 | SPI MOSI |
| Right | D9 / MISO | 4 | SPI MISO |
| Right | D8 / SCK | 2 | SPI clock |
| Right | D7 / RX | 1 | UART RX |

D pins support digital I/O and PWM. GPIO signals use 3.3 V logic.

## Sony interface: one SSR plus digital pot — proposed

The selected prototype uses one LCC110, MCP41050-I/P, SN74HCT125N SPI buffer, and a 2N3904 input driver. [Complete circuit, physical IC pin mapping, BOM and acceptance checks](sony-interface.md). The old five-relay and other candidate cable allocations are obsolete.

**New harness: contact 2 carries 5 V. Do not plug in an earlier harness by matching contact numbers.** Number and label the actual connector at both ends. Contact numbers are logical assignments, not a view of an unspecified connector: choose a keyed housing, mark contact 1, and record its mating-face view before crimping. Verify every conductor by continuity; mirrored mating views can reverse a ribbon.

| Contact | XIAO | GPIO | Purpose |
| --- | --- | ---: | --- |
| 1 | GND | — | Shared ground |
| 2 | 5V/VBUS | — | Interface power |
| 3 | D8/SCK | 2 | Buffer clock input |
| 4 | D10/MOSI | 3 | Buffer data input |
| 5 | D0 | 26 | Buffer CS input |
| 6 | D1 | 27 | Relay enable via base resistor |
| 7 | GND | — | Second ground |
| 8 | D2 | 28 | Camera request to U2 channel 4 |

Six conductors serve the steering interface alone; use eight for the required Camera 1 control and second ground. D9/MISO remains free. The separate Twist cable is unchanged.

Sony Input 1 goes through 2.7 kohm, the LCC110 normally-open contact, and the pot W-to-B resistance to ST Remote GND. **ST Remote GND and logic GND are joined** in this design. Input 2 stays insulated. The included Sony adapter already supplies labeled leads; no audio breakout is required. No Sony signal or raw vehicle 12 V connects to GPIO.

All outputs remain disabled. Verify the Sony-disconnected staged assembly, commanded resistances, voltage/current limits, reset and supply transitions before receiver connection. A [complete proposed pad map](../hardware/sony-remote/pad-map.md) now includes the pigtail landing pads; actual fit, soldered assembly and connector orientation remain unvalidated. The [hardware index](../hardware/sony-remote/README.md) links the current BOM; [old generated layouts](../hardware/archive/sony-remote-photomos/README.md) are archived and must not be used.

## Camera routing — Sony behavior verified; interface and split video pending

See [camera-control circuit and behavior plan](camera-control.md). The revised placement uses U2 channel 4, two on-board pull-downs and one inline camera-cable resistor; no second relay.

| Mode | Sony input | Trigger | Desired overlay |
| --- | --- | --- | --- |
| Actual reverse | REAR CAMERA IN | Reverse-light signal to REVERSE IN | Sony parking lines |
| Manual view | CAMERA IN 1 | Interface to EXT IN 1 | No lines |

User verified the desired overlay separation and reverse priority using one camera input at a time; see [bench report](bench-results-2026-10-28.md). Next compare a direct video connection against one source feeding both inputs through an RCA Y. Check whether connecting the inactive branch changes brightness, color, sharpness, or sync. Sony's inactive-input termination is unknown. Add buffering or switching only if evidence warrants it.

Disable camera-generated guidelines for these tests. Verify the actual camera's supply voltage. Do not substitute an audio breakout cable for an RCA Y.

## Mechanical and power constraints

- Cigarette-lighter opening size and rear depth remain unmeasured.
- Twist PCB: approximately 30.48 × 25.4 mm; XIAO: 21 × 17.8 mm.
- Housing/bracket supports both boards, knob-press forces, and cable strain.
- Development: USB-C to XIAO; XIAO 3V3 to Twist.
- Installed knob: existing regulated 5 V converter to XIAO; Twist stays on 3.3 V.
- Pair budget: 100 mA at 5 V (0.5 W) with margin; interfaces/relays extra. Actual draw is unmeasured.
- MagSafe uses its own suitable PD converter. Never passively share its negotiated high-voltage output with the XIAO.
- Vehicle power requires appropriate fusing and switching. Transient protection must be evaluated for the actual converters.
