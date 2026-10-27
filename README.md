# Miata remote-control knob

A minimal multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata, using a Seeed Studio XIAO RP2040 and SparkFun Qwiic Twist RGB encoder.

The knob will occupy the cigarette-lighter location. Use removable connections. A small soldered protoboard near the Sony will support the remote-control interface; no carrier board is planned for the Twist and XIAO.

## Status

The XIAO, Twist, Sony receiver, a camera, and the Sony's included steering-remote adapter are on hand. The adapter has leads labeled Input 1, Input 2, and ST Remote GND. The user has verified diagnostic v2 rotation, button events, and rotation while pressed. A Picotool-enabled build and build/upload helper are ready for testing USB updates without board-button access. The electrical interface to the Sony is still a proposal, not a verified schematic.

First milestone: power the XIAO from computer USB, read the Twist using hardware I2C, test its lighting and button, and report gestures over USB serial. Radio and camera-control outputs remain disabled for this milestone.

## Documents

- [Wiring and cable plan](docs/wiring.md)
- [Design decisions and evidence](docs/design.md)
- [Development checklist](docs/tasks.md)
- [Firmware environment](firmware/README.md)
- [Workspace and access](docs/workspace.md)

Hardware tests must be recorded as observations, with date and configuration. Do not treat a proposed pin reservation or external example as confirmation of compatibility.
