# Miata remote-control knob

A multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata: Seeed Studio XIAO RP2040 plus SparkFun Qwiic Twist RGB, mounted in the cigarette-lighter location.

## Current status

The user has bench-verified rotation, button events, gestures and RGB feedback over USB. Idle lighting is full-brightness blue for the opaque cap; gesture colors are brief acknowledgements, not confirmed radio state. The original diagnostic is preserved. Earlier Picotool updates worked without physical button access, but repeated controller uploads have failed; reliable software-only updating remains an installation requirement.

The selected Sony interface is **MCP41050-I/P + SN74HCT125N + one existing LCC110**, with a 2N3904 driver, all through-hole. The assembled board passed user bench checks of relay switching, GPIO camera output and three programmed resistances; see [results](docs/bench-results-2026-11-05.md). Controller v3 now connects gestures/LEDs to real output requests. Sony distinguishes the two volume endpoint assignments. Controller v3.4 adds three interior candidates and a live calibration console; expanded learning and installed power-transition acceptance remain pending; see [current code assignments](docs/sony-interface.md#calibration-model-and-firmware-contract).

A small protoboard near the Sony is approved. Twist and XIAO need no carrier board. Final enclosure, no-cut permanent board placement, cable length, vehicle power protection and assembled camera interface remain unfinished. Camera 1 trigger/release, live video, guideline separation and reverse priority passed separate-input bench tests; a passive RCA splitter subsequently passed reverse/Camera 1 switching with the intended guideline separation and no visible image degradation (2026-10-31).

## Start here

| Document | Purpose |
| --- | --- |
| [Sony bench results](docs/bench-results-2026-10-28.md) | Measured values, observed camera behavior, calculations and remaining tests |
| [Camera control](docs/camera-control.md) | Required preview circuit, trigger contract and acceptance |
| [Design and evidence](docs/design.md) | Requirements, choices, what is and is not verified |
| [Sony circuit](docs/sony-interface.md) | Current schematic overview, complete pin map, limits and backend contract |
| [Assembly and test](docs/assembly-and-test.md) | Staged permanent assembly, calibration and acceptance gates |
| [Wiring](docs/wiring.md) | Twist/XIAO pins, eight-wire harness, power and camera plan |
| [Parts and order](docs/parts-sourcing.md) | Exact ordered parts, quantities, existing supplies |
| [Hardware index](hardware/sony-remote/README.md) | BOM, all-component placement and checked pad map |
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

The user performs flashing: close the monitor, then explicitly run `scripts/knob.sh upload controller` after a successful build. The upload reports flash and static RAM usage, then delegates to Arduino CLI and the core-provided Picotool recipe. It does not rebuild automatically. Add `--port PORT` to select a serial port explicitly. Use target `diagnostic` for the original hardware diagnostic; `raw-monitor PORT` reads its output. See the firmware guide before first upload or recovery.

Host tests never communicate with physical hardware. Historical five-PhotoMOS netlist tests are retained but do not validate the current circuit. Record hardware observations with date and configuration; a passing compile or host test is not receiver acceptance. Read [AGENTS.md](AGENTS.md) before changing this public repository.
