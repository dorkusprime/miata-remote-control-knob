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

## Sony resistor-interface prototype — receiver disconnected

The selected prototype uses five AQY212EH normally-open PhotoMOS switches and a seven-resistor ladder. It supersedes the analog-multiplexer proposal and its cable assignment. The schematic, exact ElectroCookie Mini pad layout, BOM, and receiver-disconnected multimeter procedure are in [Sony interface prototype](sony-interface.md). The design is ready to mock up; no physical interface or Sony acceptance test has been performed.

The Sony includes a Steering Remote adapter with an apparently 3.5mm plug and leads labeled Input 1, Input 2, ST Remote GND (user observation). Use those labeled leads; no added audio breakout is needed. Exact adapter model/plug-contact mapping is not needed for the labeled-lead mock-up. Input selection/recognition remains a receiver acceptance test.

### Controller cable J1 — replaces the old eight-wire mux allocation

| Contact | XIAO label | GPIO | Purpose |
| --- | --- | --- | --- |
| 1 | GND | — | PhotoMOS LED return, logic side only |
| 2 | D0 | 26 | Mute/ATT, U1 |
| 3 | D1 | 27 | Next track, U2 |
| 4 | D2 | 28 | Previous track, U3 |
| 5 | D3 | 29 | Volume up, U4 |
| 6 | D6 | 0 | Volume down, U5 |

No 5V/VBUS wire is required. The old pin-2 5V assignment must not be used. An optional eight-conductor cable may reserve contact 7 for D7/GPIO1 to a future camera interface and contact 8 as spare; leave both unconnected now. D8–D10 remain free. This cable is separate from the four-wire Twist cable.

### Sony connector J2

| Contact | Adapter lead |
| --- | --- |
| 1 | Input 1, resistor-ladder input |
| 2 | ST Remote GND, PhotoMOS contact return |

Input 2 remains individually insulated. Keep logic GND and Sony ST Remote GND separate on this board; PhotoMOS control inputs and output contacts are optically separate. Do not connect Sony signal leads to GPIO, 3V3, 5V, or raw vehicle 12V. Sony stays disconnected throughout resistance-output testing. Existing firmware does not drive these command outputs.

Before connecting to the receiver, measure the prototype, verify release/reset behavior with its future backend, check Sony remote voltage against the design envelope, and test command learning/recognition. Camera trigger and video routing remain separate work.

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
