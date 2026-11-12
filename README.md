# Miata remote-control knob

A multifunction knob for a Sony XAV-9000ES in a 2005 Mazdaspeed Miata: Seeed Studio XIAO RP2040 plus SparkFun Qwiic Twist RGB, mounted in the cigarette-lighter location.

## Hook up the project box

1. [Open the complete hookup image](hardware/internal-jumpers.svg) for the seven internal jumpers, P1, P2 and USB-C.
2. [Wire the box connectors](docs/wiring.md) for the Twist, Sony, vehicle power and USB-C programming.
3. [Run the assembly and acceptance checks](docs/assembly-and-test.md) before connecting the receiver.

## Current status

The assembled Knob Goblin passed end-to-end bench testing with controller v4.0 and the Sony XAV-9000ES. The user verified all six audio commands, manual Camera 1, reverse override and cold startup on 12 V without the laptop. Controller traces confirm proportional turns, both bulk flicks and cancellation during a burst. The [bench record](docs/bench-results-2026-11-12.md) links the earlier measurements, fixes and remaining electrical and vehicle checks.

## Reference

| Document | Purpose |
| --- | --- |
| [Sony circuit](docs/sony-interface.md) | Electrical pin map, receiver codes and limits |
| [Hardware index](hardware/sony-remote/README.md) | BOM, all-component placement and checked pad map |
| [Camera control](docs/camera-control.md) | Preview circuit, trigger contract and acceptance |
| [Gestures and monitor](docs/gestures.md) | Current gestures, timing, learning, LED behavior and serial protocol |
| [Firmware environment](firmware/README.md) | Pinned toolchain, diagnostics and USB recovery |
| [Parts and order](docs/parts-sourcing.md) | Exact ordered parts, quantities and existing supplies |
| [Design and evidence](docs/design.md) | Requirements, design choices and evidence |
| [Development checklist](docs/tasks.md) | Completed work and open milestones |
| [Workspace](docs/workspace.md) | Repository access and working conventions |
| [Archive](docs/archive/README.md) | Superseded designs and sourcing history |

## Development

```sh
python3 scripts/test.py
scripts/knob.sh build controller
scripts/knob.sh size controller
scripts/knob.sh monitor
```

The user performs flashing: close the monitor, then explicitly run `scripts/knob.sh upload controller` after a successful build. The upload reports flash and static RAM usage, then delegates to Arduino CLI and the core-provided Picotool recipe. It does not rebuild automatically. Add `--port PORT` to select a serial port explicitly. Use target `diagnostic` for the original hardware diagnostic; `raw-monitor PORT` reads its output. See the firmware guide before first upload or recovery.

Host tests never communicate with physical hardware. Historical five-PhotoMOS netlist tests are retained but do not validate the current circuit. Record hardware observations with date and configuration; a passing compile or host test is not receiver acceptance. Read [AGENTS.md](AGENTS.md) before changing this public repository.
