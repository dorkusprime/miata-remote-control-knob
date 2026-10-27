# Current Sony interface hardware

Selected: **MCP41050-I/P + SN74HCT125N + LCC110 + 2N3904**, all through-hole, shared logic/Sony ground. Parts ordered; not physically validated.

- [Circuit and complete pin-level connections](../../docs/sony-interface.md)
- [Assembly, calibration and acceptance](../../docs/assembly-and-test.md)
- [Harness and controller pins](../../docs/wiring.md)
- [Exact purchased parts and spare quantities](../../docs/parts-sourcing.md)
- [Per-controller BOM](bom.csv)
- [All-component placement SVG](placement-full.svg) / [PNG preview](placement-full.png) — top and mirrored underside
- [Complete pad / jumper table](pad-map.md) and [checked layout model](layout.json)
- [Earlier IC-only fit study](placement.svg) — subset reference

The current all-component candidate places all 14 component references, all eight pigtail landings and 13 insulated jumpers without copper cuts or shared holes. The checker verifies connectivity against the current circuit and nominal same-face body separation. Physical fit, actual sockets, mounting holes, lead forming and height remain unverified. This is a complete proposed electrical pad map, not a physically accepted assembly.

The former five-PhotoMOS JSON/SVG/BOM files are in [hardware/archive](../archive/sony-remote-photomos/README.md). Their generator and tests are retained for history and do not validate this circuit. Never build the current circuit using those drawings or connector allocations.

## Complete proposed placement

![All-component top and underside placement](placement-full.png)

Top: three DIP packages, Q1, four upright 47 kohm resistors and three capacitors. Underside: R1–R3 and insulated jumpers. The separate faces keep component bodies clear. R2 and R3 follow opposite outer edges; the drawing does not stack resistor bodies or route jumpers across them. Extended leads must be sleeved. The backside view is mirrored horizontally while retaining physical pad IDs.

The pigtail wires land on separate pads. Connector housings stay off-board; no in-line header footprint is implied. Q1 requires formed leads and the ceramic capacitors require lead spreading. Socket/mounting interference and actual lead lengths must be dry-fitted before soldering. See [the pad map](pad-map.md) for every endpoint and assembly conditions.

Generate with `python3 scripts/full-placement.py`; verify with `python3 scripts/full-placement.py --check`. The full host suite includes current-layout fault injection. The older `scripts/placement-study.py` still reproduces the IC-only image, but the all-component image above is the current placement proposal.
