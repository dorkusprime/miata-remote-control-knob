# Miata remote-control knob

A minimal multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata, using a Seeed Studio XIAO RP2040 and SparkFun Qwiic Twist RGB encoder.

The knob will occupy the cigarette-lighter location. Use removable connections and no carrier board unless necessary.

## Status

The XIAO, Twist, Sony receiver, and a camera are on hand. No firmware has been implemented or flashed. The electrical interface to the Sony is still a proposal, not a verified schematic.

First milestone: power the XIAO from computer USB, read the Twist using hardware I2C, test its lighting and button, and report gestures over USB serial. Radio and camera-control outputs remain disabled for this milestone.

## Documents

- [Wiring and cable plan](docs/wiring.md)
- [Design decisions and evidence](docs/design.md)
- [Development checklist](docs/tasks.md)
- [Firmware environment](firmware/README.md)
- [Workspace and access](docs/workspace.md)

Hardware tests must be recorded as observations, with date and configuration. Do not treat a proposed pin reservation or external example as confirmation of compatibility.
