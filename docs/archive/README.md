# Superseded design history

The [current through-hole interface](../sony-interface.md) is the build reference. Files here preserve decisions and research, not current assembly instructions.

| Historical document | Why retained |
| --- | --- |
| [AD8400 interface](sony-interface-ad8400.md) | Earlier one-SSR design using an SMD pot with direct 3.3 V SPI |
| [SOIC assembly](smd-assembly.md) | Practice method for that abandoned package choice |
| [Pot research](sony-interface-simplification.md) | Voltage evidence and earlier MCP candidates |
| [Five-PhotoMOS interface](sony-interface-photomos.md) | Previous resistor ladder and isolated control approach |
| [Sourcing history](parts-sourcing-history.md) | Earlier marketplace searches and superseded BOMs |
| [Five-PhotoMOS artifacts](../../hardware/archive/sony-remote-photomos/README.md) | Historical schematic, netlist, placement and generator |

Pin allocations, package pinouts, current ratings and pot codes are not interchangeable between versions. In particular, the current eight-wire harness carries +5 V on contact 2.
