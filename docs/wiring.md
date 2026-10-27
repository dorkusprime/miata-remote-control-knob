# Wiring and cable plan

## Encoder: hardware I2C

| Twist | XIAO board label | RP2040 GPIO | Purpose |
| --- | --- | --- | --- |
| GND | GND | — | Ground |
| 3.3V | 3V3 | — | Encoder and LED power |
| SDA | D4 | GPIO6 | I2C0 data |
| SCL | D5 | GPIO7 | I2C0 clock |
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
| Left | D4 / SDA | 6 | I2C0 SDA |
| Left | D5 / SCL | 7 | I2C0 SCL |
| Left | D6 / TX | 0 | UART TX |
| Right | 5V / VBUS | — | USB 5 V rail |
| Right | GND | — | Ground |
| Right | 3V3 | — | 3.3 V output |
| Right | D10 / MOSI | 3 | SPI MOSI |
| Right | D9 / MISO | 4 | SPI MISO |
| Right | D8 / SCK | 2 | SPI clock |
| Right | D7 / RX | 1 | UART RX |

D pins support digital I/O and PWM. GPIO signals use 3.3 V logic.

## Proposed Sony control harness — NOT ready to assemble

### Included steering-remote adapter

User observation recorded 2026-10-27: the Sony includes a cable marked "Steering Remote," with an apparently 3.5 mm plug at one end and three labeled wires at the other. The exact adapter model and plug-contact mapping have not been verified.

| Adapter lead label | Proposed connection |
| --- | --- |
| Input 1 | Switched-resistance command interface, pending bench verification |
| Input 2 | Leave individually insulated and unconnected initially |
| ST Remote GND | Remote-command return for the resistor interface |

Use these factory-labeled leads as the connection point; no additional audio-style breakout cable is needed. The initial proposal switches resistance between Input 1 and ST Remote GND. Input selection, voltage, and command behavior still require validation. Do not connect Input 1 or Input 2 directly to XIAO GPIO or to vehicle 12 V.

### Controller-to-interface cable

The remote input needs switched resistances, not a direct GPIO connection. One proposal places an analog multiplexer and fixed resistors near the Sony. IC choice, resistor values, and electrical defaults still need validation.

Construction decision (2026-10-27): mount the interface on a small soldered protoboard near the Sony. It will support the switching component, resistors, bypass capacitor, and any required bias/protection components. Use removable cable connections, strain relief, and an insulating enclosure or cover. Secure the assembly independently of its wires. Choose board dimensions and the solder layout after validating the circuit; this is not a solderless breadboard installation. The Twist and XIAO do not need a shared carrier board.

Reserve these pins provisionally:

| Conductor | XIAO | GPIO | Proposed purpose |
| --- | --- | --- | --- |
| 1 | GND | — | Interface ground |
| 2 | 5V / VBUS | — | Interface supply if required |
| 3 | D0 | 26 | Resistance select A |
| 4 | D1 | 27 | Resistance select B |
| 5 | D2 | 28 | Resistance select C |
| 6 | D3 | 29 | Command enable / disconnect |
| 7 | D6 | 0 | Manual camera-view request to interface |
| 8 | Not assigned | — | Spare |

D7–D10 remain unassigned. Power and signal cables are separate; this proposed eight-wire harness is not the four-wire Twist cable.

Before finalizing the Sony harness, verify:

1. Included adapter identity, remote input voltage, input selection, and resistance/learning behavior. Verify plug-contact mapping only if needed; use the labeled leads for assembly.
2. Switch analog limits, control thresholds, on-resistance, and leakage.
3. No back-powering when Sony is on and XIAO/interface is off, including USB programming scenarios.
4. Hardware defaults that release all commands at startup/reset/disconnection.
5. Camera trigger voltage margin, switching circuit, and ground requirements.
6. Connector pin numbers/viewing direction, cable length, and strain relief.

## Camera routing — pending bench test

| Mode | Sony input | Trigger | Desired overlay |
| --- | --- | --- | --- |
| Actual reverse | REAR CAMERA IN | Reverse-light signal to REVERSE IN | Sony parking lines |
| Manual view | CAMERA IN 1 | Interface to EXT IN 1 | No lines |

First verify that the overlays differ as desired. Then compare a direct video connection against one source feeding both inputs through an RCA Y. Check whether connecting the inactive branch changes brightness, color, sharpness, or sync. Sony's inactive-input termination is unknown. Add buffering or switching only if evidence warrants it.

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
