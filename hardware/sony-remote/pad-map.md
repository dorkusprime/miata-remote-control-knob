# Complete proposed pad allocation

**Electrically checked design, not a physically verified build.** Use the [circuit guide](../../docs/sony-interface.md) for device limits and acceptance. [All-component image](placement-full.svg) separates top and horizontally mirrored underside views. No firmware outputs were enabled.

## Component terminals

| Ref | Part / side | Terminal → pad |
| --- | --- | --- |
| U1 | MCP41050-I/P / top | 1 → E1, 2 → E2, 3 → E3, 4 → E4, 8 → F1, 7 → F2, 6 → F3, 5 → F4 |
| U2 | SN74HCT125N / top | 1 → E6, 2 → E7, 3 → E8, 4 → E9, 5 → E10, 6 → E11, 7 → E12, 14 → F6, 13 → F7, 12 → F8, 11 → F9, 10 → F10, 9 → F11, 8 → F12 |
| K1 | LCC110 / top | 1 → E14, 2 → E15, 3 → E16, 4 → E17, 8 → F14, 7 → F15, 6 → F16, 5 → F17 |
| Q1 | 2N3904BU / top | E → A12, B → C13, C → D16 |
| R1 | 330 ohm / bottom | 1 → C15, 2 → H13 |
| R2 | 1 kohm / top | 1 → A5, 2 → A13 |
| R3 | 2.7 kohm / top | 1 → J5, 2 → J17 |
| R4 | 47 kohm / top | 1 → C7, 2 → C6 |
| R5 | 47 kohm / top | 1 → C10, 2 → C9 |
| R6 | 47 kohm / top | 1 → H11, 2 → H10 |
| R7 | 47 kohm / top | 1 → B13, 2 → B12 |
| C1 | 100 nF / top | 1 → J1, 2 → J4 |
| C2 | 100 nF / top | 1 → H6, 2 → G8 |
| C3 | 10 uF / top | + → J7, - → J8 |

## Distributed pigtails

These are wire landing pads, not two in-line headers. Fit the existing removable connectors off-board. J1 numbering matches the current harness. Sony Input 2 stays insulated.

| Contact | Net | Pad |
| --- | --- | --- |
| J1.1 | GND | B4 |
| J1.2 | +5V | J6 |
| J1.3 | SCK_IN | A10 |
| J1.4 | MOSI_IN | I11 |
| J1.5 | CS_IN | A7 |
| J1.6 | RELAY_EN | B5 |
| J2.1 | SONY_IN | I5 |
| J2.2 | GND | H4 |

## Underside wire pieces and bus taps

Crossings are not junctions. Use insulated wire; route clear of resistor bodies and keep leads sleeved over unrelated pads. Coordinates are endpoint and routing references, not validated three-dimensional wire bends.

| Wire | Net | Solder pads in order |
| --- | --- | --- |
| BG | GND | C4 → G4 → I8 → J10 → D12 → A9 → B6 |
| BV | +5V | G1 → I6 → G7 → F13 |
| W10 | CS_OUT | A1 → A8 |
| W11 | SCK_OUT | B2 → B11 |
| W12 | MOSI_OUT | C3 → J12 |
| W13 | PW | H3 → G16 |

BG and BV are each one continuous conductor with multiple solder taps, not chains of separate jumpers. Expose only the listed tap pads; sleeve all intervening sections. Untapped pads along the drawn route stay insulated. Tack the middle taps to the underside pads rather than threading a wire repeatedly through holes. This reduces wire pieces from 13 to 6 and wire solder landings from 26 to 19; it does not remove electrical nets. Four signal wires remain separate.

If making continuous tapped buses is awkward, separate insulated segments following the same net connections remain electrically equivalent; they restore some of the original cut/strip work.

## Assembly and physical conditions

- Fit sockets and top components first, without solder. The IC-only image is now a subset of this candidate.
- R4–R7 use upright mounting to span adjacent rows; the axial bodies do not lie flat between those pads.
- Q1 has a formed-lead footprint, not the default three adjacent TO-92 holes. E=A12, B=C13, C=D16. Check the actual part orientation and preform without force at the body.
- Only R1 remains on the underside. R2 moves onto the top left edge; R3 moves onto the top right edge and its body shifts down to avoid C3. Top R3 leads jog to x=37 mm past C3. Sleeve extended leads, retain solder-joint clearance and dry-fit before trimming.
- Check supplied lead lengths before trimming. If a lead cannot follow its proposed route safely, revise the model or use an insulated extension; do not force the component.
- C1/C2 leads are spread to the listed pads, rather than matching the stock 2.5 mm lead pitch. Avoid stress at the ceramic coating. C3 is polarized.
- Body envelopes are provisional. The resistor envelope allows 6.8 × 2.6 mm, based on Yageo MFR-25 maximum body dimensions. Socket outlines, capacitor lead availability, mounting-hole locations and assembled height still require actual measurements.
- This candidate uses both faces and distributed pigtails to preserve the no-cut board. It does not promise a single flat layer or accessible center mounting holes.
- On the actual board, verify A–E / F–J strip continuity and orientation first. Do not solder until the dry-fit and receiver-disconnected electrical checks pass.

## Verification scope

`python3 scripts/full-placement.py --check` verifies the 14 component references, terminal connectivity against an independent device-pin contract, all resistor values, factory strips, no accidental net joins, no reused holes, pigtail assignments, nominal same-face body separation, jumper avoidance of underside resistor bodies, and relocated top-resistor lead avoidance of other component bodies. It also verifies that the SVG/table match the model. This is not a timing, analog-current, power-sequencing or physical-fit simulation.

Regenerate with `python3 scripts/full-placement.py`. Tests deliberately introduce a rail short, an open jumper, a transistor-pin swap, pad reuse, a resistor-value error, a body overlap and a wire crossing a resistor body.

Source for resistor envelope: [Yageo MFR datasheet, dimensions](https://yageogroup.com/content/datasheet/asset/file/YAGEO-MFR_DATASHEET). Other envelopes are conservative drawing assumptions, not new manufacturer guarantees.
