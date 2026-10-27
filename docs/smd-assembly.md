# SOIC-8 practice and AD8400 assembly

User chose this direction on 2026-10-27. No soldering or electrical validation has yet been reported. Follow the [current circuit](sony-interface.md) and [parts candidates](parts-sourcing.md); the archived five-relay diagrams are not applicable.

## Tools and geometry

Use a normal soldering iron with a clean small chisel tip, electronics flux, thin solder (approximately 0.3–0.5 mm), tweezers, solder wick, magnification/bright lighting and a multimeter. A stable board holder helps. SOIC-8 has exposed leads at 1.27 mm pitch; no hot-air station is required. The Wagner HT1000 is a general-purpose heat gun, with nominal 399°C/538°C settings, and is not the selected soldering tool.

Use a solder-on adapter for narrow 150 mil-body SOIC-8, with 1.27 mm lead pitch and a suitable 2.54 mm DIP-style output. Inspect the actual board and chip together before heating anything. Do not assume an unlabeled reverse side has the same footprint, or that adapter pin numbering matches the IC. Confirm the notch/dot and marked pad 1. Solder the IC first, headers last.

## First practice: unpowered

1. Use an inexpensive matching SOIC-8 chip on one spare adapter. Leave headers/socket off so the board can sit flat.
2. Hold the board securely. Apply flux to the pads and put a small amount of solder on one corner pad.
3. Place the chip with tweezers and align every lead over its pad. Reheat the tinned corner pad to tack that lead. Remove the iron and let the joint cool.
4. Inspect alignment before continuing. Reheat the single tack and reposition if necessary; do not force the body while solder is solid.
5. Solder the diagonally opposite corner. Recheck alignment and that the leads sit on the pads.
6. Add flux as needed and solder the other six leads individually. Heat the lead and pad together, feed a small amount of solder to the joint, then remove solder and iron. There is no need to learn drag soldering for eight pins. Avoid prolonged heating; let a difficult joint cool before retrying.
7. Inspect all joints with magnification. Use flux and solder wick to remove visible bridges, then reinspect. A bridge is usually repairable; do not discard the practice assembly immediately.
8. With no power, check continuity from each metal lead to its intended adapter through-hole, verifying all eight mappings. Inspect adjacent pads for physical solder bridges. A chip's internal semiconductor paths can affect meter readings, so a beep between pins alone is not conclusive evidence of a bridge.
9. When satisfied, add headers. A spare unpowered breadboard/socket can hold header alignment; do not apply power to the practice chip without its own correctly wired circuit.

A useful completion criterion is eight verified lead-to-header connections, no visible solder bridges, and a securely aligned body. Repeat on a spare if the first attempt needs substantial rework.

## Actual AD8400

Repeat the same process on a fresh adapter with AD8400ARZ50. Its pad/pin assignment is **not the NE555's assignment**; the practice chip only shares the mechanical package. The AD8400's pin-1 mark must align with adapter pad 1. Verify actual IC lead-to-header mappings before installing the module into the Sony-interface breadboard. Record orientation so future removal/reinsertion is unambiguous.

Before power, check the assembled circuit against the pin-by-pin netlist. Check for supply/ground shorts and confirm the LCC110 normally-open contact is in series with Sony Input 1. Keep the Sony disconnected. First power-up and later resistance calibration follow the current interface guide; no automatic flashing or head-unit energizing is authorized by this assembly note.

Final socket/header height, protoboard pad map and enclosure clearance remain to be dry-fitted. This procedure does not establish the separate power-loss transient acceptance check documented in the circuit guide.

## References

- [AD8400 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/AD8400_8402_8403.pdf), pinout and SOIC-8 dimensions.
- [Hand soldering SMT, Wayne and Layne](https://www.wayneandlayne.com/smt/hand-soldering-smt/), exposed-lead tacking and hand soldering.
- [SparkFun adapter soldering guide](https://learn.sparkfun.com/tutorials/ssop-16-to-dip-adapter-hookup-guide), chip-before-header assembly and soldering techniques; this project's chip is SOIC-8 rather than the guide's SSOP-16.
- [Wagner HT1000 specifications](https://www.wagnerspraytech.com/product/ht-series-heat-guns/ht1000-heat-gun/).
