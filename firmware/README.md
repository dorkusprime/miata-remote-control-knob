# Firmware environment

The serial-only diagnostic builds for Seeed XIAO RP2040. User-provided v2 output confirms rotation in both directions, button transitions, and rotation while held. Twist firmware is 1.2. The Picotool-enabled build compiles but its upload path has not yet been hardware-validated. Sony and camera outputs are not configured or driven.

## Pinned dependencies

Versions are recorded in [toolchain.json](toolchain.json): Arduino CLI 1.5.1, Earle Philhower Arduino-Pico core 6.3.1, SparkFun Qwiic Twist Arduino Library 1.0.4. Board FQBN: `rp2040:rp2040:seeed_xiao_rp2040:uploadmethod=picotool`. The upload method is also a compile option: it adds the USB maintenance interface to the running firmware.

Dependency license review: the SparkFun library's bundled `LICENSE.md` specifies MIT for code, while its source headers contain GPL notices. Resolve that inconsistency before distributing linked firmware binaries. The library is installed as a dependency, not copied into this repository. No code from the separate Sony-controller implementation has been incorporated.

## Setup and build

From the repository root on Apple Silicon macOS:

```sh
python3 scripts/setup-toolchain.py
scripts/knob.sh build
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

## USB updates without board-button access

The default UF2 uploader resets the serial device and searches for a mounted drive. That path has repeatedly failed in this setup. The selected Picotool mode adds a USB reset interface to the firmware and transfers firmware using USB directly. It avoids the mounted-drive discovery step. The original failure cause remains unconfirmed.

One-time transition: close the serial monitor; enter BOOT mode using the procedure that already worked; then copy the **Picotool-enabled** binary from the repository root:

```sh
cp build/picotool/diagnostic.ino.uf2 /Volumes/RPI-RP2/
```

After restart, send `h` in the serial monitor and confirm `USB maintenance: Picotool enabled`. The previous `build/diagnostic` binary does not enable this interface.

For subsequent updates, leave USB connected, close the serial monitor, then run:

```sh
scripts/knob.sh build
scripts/knob.sh upload
scripts/arduino.sh board list
scripts/knob.sh monitor /dev/cu.usbmodem1412301
```

Use the current port from `board list` for the monitor; Picotool upload itself does not require a serial-port name. Connect only the intended RP2040 target when uploading: Picotool selects a device over USB. The helper does not compile implicitly during upload; build after each source change. It uploads only from `build/picotool` using the installed Picotool with forced USB reset, read-back verification, and execution (`load -v -f -x`). The build FQBN in `toolchain.json` must retain `uploadmethod=picotool`.

Before mounting the board, verify the Picotool banner and perform two successive uploads without pressing buttons or unplugging USB, checking the diagnostic still runs afterward. These checks are pending. Do not present compilation as proof of remote upload success. A firmware/USB failure can still require hardware recovery; Picotool is not a guarantee against every hang. Keep an accessible USB data connection in the installation design.

The user currently performs uploads. An agent must have an explicit instruction before flashing attached hardware, per AGENTS.md. No upload has been performed by the agent.

## Verification recorded 2026-10-27

- USB registry identifies Seeed XIAO RP2040; serial port `/dev/cu.usbmodem1412301` is present. This confirms enumeration, not diagnostic execution.
- The selected core's XIAO variant defines SDA as GPIO6 and SCL as GPIO7.
- Diagnostic v2 compilation with `--warnings all` passed: 61,784 bytes flash, 9,140 bytes global RAM; no warnings reported. Compile-time checks cover stationary counts and forward/reverse counter wrap.
- User reports the first upload succeeded after BOOT-mode UF2 copy. Button DOWN/UP events work; turning produces no reported TURN events. Root cause is unconfirmed.
- User reports a subsequent automatic upload failed after the board drive unmounted; unplugging and entering BOOT mode before re-uploading succeeded. Upload failure cause is unconfirmed.
- User-provided v2 serial output confirms +1/-1 TURN events, BUTTON DOWN/UP, and turns with button=DOWN. Count/difference change together; firmware=1.2. The original getDiff-based diagnostic failure is not fully explained.
- Picotool-enabled build passed: 62,640 bytes flash, 9,316 bytes global RAM. Two missing-initializer warnings originate in the core USB.cpp reset-driver structure (name/deinit); no sketch warnings reported. USB upload validation and LED tests remain pending.

References: [core installation and first upload](https://arduino-pico.readthedocs.io/en/latest/install.html), [SparkFun library](https://github.com/sparkfun/SparkFun_Qwiic_Twist_Arduino_Library).

## Historical default-UF2 upload results

The original serial upload reported `No drive to deploy`. The user successfully flashed by unplugging USB, holding B/BOOT, reconnecting USB, releasing B, and copying the UF2 onto RPI-RP2. Automatic serial-reset uploads failed again after the first flash; manual BOOT mode worked again. The report that the drive unmounted does not establish why the upload failed. Retain these observations until the Picotool workflow is verified.
