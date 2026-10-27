# Miata remote-control knob

A minimal multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata, using a Seeed Studio XIAO RP2040 and SparkFun Qwiic Twist RGB encoder.

The knob will occupy the cigarette-lighter location. Use removable connections. A small soldered protoboard near the Sony will support the remote-control interface; no carrier board is planned for the Twist and XIAO.

## Status

The XIAO, Twist, Sony receiver, a camera, and the Sony's included steering-remote adapter are on hand. The adapter has leads labeled Input 1, Input 2, and ST Remote GND. The initial functional diagnostic is complete: the user verified diagnostic v2 rotation, button events, rotation while pressed, and RGB tests. The user has confirmed two consecutive Picotool uploads without button access or unplugging USB. The selected prototype uses one LCC110 and an AD8400ARZ50 digital pot. The user will hand-solder its SOIC-8 package onto a small adapter after practicing first; circuit, pinout and assembly steps are documented. It has not been assembled or verified against the Sony.

The separate gesture controller and live translation monitor are host-tested and user bench-verified: the user reports all gestures functional. Radio and camera-control outputs remain disabled until the Sony interface is validated.

Controller v2 adds full-brightness blue idle lighting and brief gesture feedback. Camera gestures pulse amber, with no latched camera-state color. This LED update is compiled, host-tested, and user bench-verified.

```sh
scripts/knob.sh build controller
scripts/knob.sh upload controller
scripts/knob.sh monitor
```

Close any running monitor before upload. The monitor detects the USB port automatically; actions are simulations. The original diagnostic is preserved: use `build diagnostic`, `upload diagnostic`, and `raw-monitor PORT` with the same helper. Run host tests with `python3 scripts/test.py`.

Upload prints the selected build's flash-image and static RAM usage before flashing. Preview the report without uploading with `scripts/knob.sh size` or `scripts/knob.sh size diagnostic`.

## Documents

- [Wiring and cable plan](docs/wiring.md)
- [Sony interface circuit, parts, pinout, and bench procedure](docs/sony-interface.md)
- [Digital-potentiometer simplification and voltage evidence](docs/sony-interface-simplification.md)
- [Amazon component sourcing](docs/parts-sourcing.md)
- [SOIC-8 practice and assembly](docs/smd-assembly.md)
- [Design decisions and evidence](docs/design.md)
- [Development checklist](docs/tasks.md)
- [Firmware environment](firmware/README.md)
- [Gesture mapping and live monitor](docs/gestures.md)
- [Workspace and access](docs/workspace.md)

Hardware tests must be recorded as observations, with date and configuration. Do not treat a proposed pin reservation or external example as confirmation of compatibility.
