# Firmware environment

The serial-only diagnostic builds for Seeed XIAO RP2040. The user flashed the first build and observed button events but no rotation events. The user reports uploading diagnostic v2; its rotation results are pending. Sony and camera outputs are not configured or driven.

## Pinned dependencies

Versions are recorded in [toolchain.json](toolchain.json): Arduino CLI 1.5.1, Earle Philhower Arduino-Pico core 6.3.1, SparkFun Qwiic Twist Arduino Library 1.0.4. Board FQBN: `rp2040:rp2040:seeed_xiao_rp2040`.

Dependency license review: the SparkFun library's bundled `LICENSE.md` specifies MIT for code, while its source headers contain GPL notices. Resolve that inconsistency before distributing linked firmware binaries. The library is installed as a dependency, not copied into this repository. No code from the separate Sony-controller implementation has been incorporated.

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

The diagnostic starts USB serial at 115200, scans I2C, and looks for a Twist at its default address 0x3F. It polls every 20 ms, reads a checked nine-byte register snapshot and reports software deltas from the absolute count plus button transitions, and retries a missing encoder every five seconds. A five-second heartbeat makes it usable with only the XIAO attached. It starts with the Twist LED off and disables rotation-driven LED changes.

| Serial key | Action |
| --- | --- |
| `h` or `?` | Help |
| `s` | I2C scan |
| `c` | Raw count, difference, status, and Twist firmware version |
| `r`, `g`, `b` | Corresponding LED channel at 8/255 |
| `w` | All channels at 8/255 |
| `o` | LED off |

This milestone reports raw events; gestures, volume/track commands, and camera switching are not implemented. Snapshot reads check the device ID and exact returned byte count. Read failures suspend events and trigger a retry; this diagnostic is not a validated fault-tolerant controller. The difference register is shown for comparison but is no longer cleared or used to derive TURN events.

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
- Diagnostic v2 compilation with `--warnings all` passed: 61,784 bytes flash, 9,140 bytes global RAM; no warnings reported. Compile-time checks cover stationary counts and forward/reverse counter wrap.
- User reports the first upload succeeded after BOOT-mode UF2 copy. Button DOWN/UP events work; turning produces no reported TURN events. Root cause is unconfirmed.
- User reports a subsequent automatic upload failed after the board drive unmounted; unplugging and entering BOOT mode before re-uploading succeeded. Upload failure cause is unconfirmed.
- Diagnostic v2 rotation output and LED test results are pending.

References: [core installation and first upload](https://arduino-pico.readthedocs.io/en/latest/install.html), [SparkFun library](https://github.com/sparkfun/SparkFun_Qwiic_Twist_Arduino_Library).

## First-upload result and BOOT-mode fallback

The original serial upload reported `No drive to deploy`. The user then successfully flashed using BOOT mode: unplug USB, hold B/BOOT, reconnect USB, release B, and copy the compiled UF2 to the mounted drive. When its name is RPI-RP2, from the repository root:

```sh
cp build/diagnostic/diagnostic.ino.uf2 /Volumes/RPI-RP2/
```

Close the serial monitor before uploading. The drive disappears as the board restarts; check the serial port again before reopening the monitor. This is a user-performed hardware operation.

The v2 banner includes `diagnostic v2 (absolute counter)`. Its raw counter report appears every five seconds and after rotation, or on `c`. Compare values before/after several slow clicks each way. A changing absolute count with a stationary difference points to the difference-reading path; both stationary requires further encoder/firmware investigation. These are diagnostic branches, not confirmed causes.

Upload workflow observation (2026-10-27): automatic reset/upload has failed more than once in this setup, including after the first successful flash. Use the user-confirmed unplug/BOOT/reconnect procedure as the current reliable fallback. Do not promise automatic upload works or diagnose the unmount without the full uploader output.
