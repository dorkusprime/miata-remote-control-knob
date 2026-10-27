# Miata remote-control knob

A multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata: Seeed Studio XIAO RP2040 plus SparkFun Qwiic Twist RGB, mounted in the cigarette-lighter location.

## Current status

The user has bench-verified rotation, button events, gestures and RGB feedback over USB. Idle lighting is full-brightness blue for the opaque cap; gesture colors are brief acknowledgements, not confirmed radio state. The original diagnostic is preserved. Picotool updates were user-verified without physical button access.

The selected Sony interface is **MCP41050-I/P + SN74HCT125N + one existing LCC110**, with a 2N3904 optical-input driver. All parts are through-hole. Supporting components were ordered from DigiKey on 2026-10-27, with intentional spares. The circuit is not assembled, calibrated or receiver-validated. Firmware currently simulates commands; Sony and camera outputs remain disabled.

A small protoboard near the Sony is approved. Twist and XIAO need no carrier board. Final enclosure, no-cut permanent board placement, cable length, vehicle power protection and camera switching remain unfinished.

## Start here

| Document | Purpose |
| --- | --- |
| [Design and evidence](docs/design.md) | Requirements, choices, what is and is not verified |
| [Sony circuit](docs/sony-interface.md) | Current schematic overview, complete pin map, limits and backend contract |
| [Assembly and test](docs/assembly-and-test.md) | Breadboard order, calibration and acceptance gates |
| [Wiring](docs/wiring.md) | Twist/XIAO pins, six-wire harness, power and camera plan |
| [Parts and order](docs/parts-sourcing.md) | Exact ordered parts, quantities, existing supplies |
| [Hardware index](hardware/sony-remote/README.md) | Current BOM and layout status |
| [Development checklist](docs/tasks.md) | Completed work and next milestones |
| [Gestures and monitor](docs/gestures.md) | Mapping, timing, LED behavior and serial protocol |
| [Firmware environment](firmware/README.md) | Pinned toolchain, diagnostics and USB recovery |
| [Workspace](docs/workspace.md) | Repository access and working conventions |
| [Archive](docs/archive/README.md) | Superseded designs; not build instructions |

## Development

```sh
python3 scripts/test.py
scripts/knob.sh build controller
scripts/knob.sh size controller
scripts/knob.sh monitor
```

The user performs flashing: close the monitor, then explicitly run `scripts/knob.sh upload controller` after a successful build. The upload reports flash and static RAM usage. It does not rebuild automatically. Use target `diagnostic` for the original hardware diagnostic; `raw-monitor PORT` reads its output. See the firmware guide before first upload or recovery.

Host tests never communicate with physical hardware. Historical five-PhotoMOS netlist tests are retained but do not validate the current circuit. Record hardware observations with date and configuration; a passing compile or host test is not receiver acceptance. Read [AGENTS.md](AGENTS.md) before changing this public repository.
