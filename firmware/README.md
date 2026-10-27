# Firmware environment

The serial-only diagnostic builds for Seeed XIAO RP2040. It has not been flashed or tested against the Twist yet. Sony and camera outputs are not configured or driven.

## Pinned dependencies

Versions are recorded in [toolchain.json](toolchain.json): Arduino CLI 1.5.1, Earle Philhower Arduino-Pico core 6.3.1, SparkFun Qwiic Twist Arduino Library 1.0.4. Board FQBN: `rp2040:rp2040:seeed_xiao_rp2040`.

The SparkFun library's source header identifies GPL licensing; its bundled license is GPL-3.0. It is installed as a dependency, not copied into this repository. Firmware distribution must comply with dependency licenses. No code from the separate Sony-controller implementation has been incorporated.

## Setup and build

From the repository root on Apple Silicon macOS:

```sh
python3 scripts/setup-toolchain.py
scripts/arduino.sh compile --fqbn rp2040:rp2040:seeed_xiao_rp2040 --warnings all --build-path build/diagnostic firmware/diagnostic
scripts/arduino.sh board list
```

The setup script downloads official packages using curl, verifies published archive checksums, and installs inside the ignored `.tools` directory. It requires Python 3, curl, tar, and network access. It does not flash hardware or change a global Arduino installation. The wrapper uses a generated configuration containing this checkout's absolute path; rerun setup if the checkout moves. Build products are ignored under `build/`.

The bootstrap currently supports Apple Silicon macOS only. On other hosts, install the versions in `toolchain.json` using Arduino CLI and the additional board-manager URL listed there; use the same FQBN and compile command with your CLI.

## Wiring and operation

Unplug USB before wiring: Twist GND to XIAO GND, 3.3V to 3V3, SDA to D4, SCL to D5. INT and RST remain disconnected. Keep Sony and interface wiring disconnected. See [the cable plan](../docs/wiring.md).

The diagnostic starts USB serial at 115200, scans I2C, and looks for a Twist at its default address 0x3F. It polls every 20 ms, reports signed rotation deltas and button transitions, and retries a missing encoder every five seconds. A five-second heartbeat makes it usable with only the XIAO attached. It starts with the Twist LED off and disables rotation-driven LED changes.

| Serial key | Action |
| --- | --- |
| `h` or `?` | Help |
| `s` | I2C scan |
| `r`, `g`, `b` | Corresponding LED channel at 8/255 |
| `w` | All channels at 8/255 |
| `o` | LED off |

This milestone reports raw events; gestures, volume/track commands, and camera switching are not implemented. The SparkFun library does not expose every individual read failure; connectivity checks reduce false events but this diagnostic is not a validated fault-tolerant controller.

## Upload and observation — requires explicit instruction

Only after the user explicitly instructs an upload, select the currently verified XIAO port (it can change), then run:

```sh
scripts/arduino.sh upload --fqbn rp2040:rp2040:seeed_xiao_rp2040 --port /dev/cu.usbmodem1412301 --input-dir build/diagnostic firmware/diagnostic
scripts/arduino.sh monitor --port /dev/cu.usbmodem1412301 --config baudrate=115200
```

The first upload may require manually entering BOOT mode; report the upload result before assuming it ran. Opening a serial monitor can miss the startup banner; send `h` and `s` to repeat help and scanning. Record actual observations separately from a successful compilation.

## Verification recorded 2026-10-27

- USB registry identifies Seeed XIAO RP2040; serial port `/dev/cu.usbmodem1412301` is present. This confirms enumeration, not diagnostic execution.
- The selected core's XIAO variant defines SDA as GPIO6 and SCL as GPIO7.
- Compilation with `--warnings all` passed: 61,536 bytes flash, 9,132 bytes global RAM; no warnings reported.
- No firmware upload, I2C detection, rotation/button observation, or LED test has occurred.

References: [core installation and first upload](https://arduino-pico.readthedocs.io/en/latest/install.html), [SparkFun library](https://github.com/sparkfun/SparkFun_Qwiic_Twist_Arduino_Library).
