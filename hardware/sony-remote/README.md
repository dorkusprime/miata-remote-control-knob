# Current Sony interface hardware

Selected: **MCP41050-I/P + SN74HCT125N + LCC110 + 2N3904**, all through-hole, shared logic/Sony ground. Parts ordered; not physically validated.

- [Circuit and complete pin-level connections](../../docs/sony-interface.md)
- [Assembly, calibration and acceptance](../../docs/assembly-and-test.md)
- [Harness and controller pins](../../docs/wiring.md)
- [Exact purchased parts and spare quantities](../../docs/parts-sourcing.md)
- [Per-controller BOM](bom.csv)

The circuit guide contains the current functional diagram and electrical pin map. A verified permanent pad placement does not exist yet: dry-fit the actual DIP packages, sockets, passives and connectors on the no-cut ElectroCookie Mini before creating one. Do not mistake a functional diagram for a physical pad layout.

The former five-PhotoMOS JSON/SVG/BOM files are in [hardware/archive](../archive/sony-remote-photomos/README.md). Their generator and tests are retained for history and do not validate this circuit. Never build the current circuit using those drawings or connector allocations.
