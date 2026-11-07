# Firmware environment

Three sketches are available: the preserved [Twist diagnostic](diagnostic/diagnostic.ino), the [interface bench diagnostic](interface/interface.ino), and the integrated [gesture controller](controller/controller.ino). Build/upload defaults to `controller`; uploading is always explicit and performed by the user.

Controller v4.0 drives the assembled board: turn=volume, click=Play/Pause, held turn=track, stationary hold/release >=700 ms=Mute, double click=Camera 1 request, ten-second stationary hold=learning toggle. It preserves USB maintenance and diagnostics. Sony has accepted the six required audio codes; user testing of the new Mute gesture after upload remains pending. See [current controls](../docs/gestures.md#current-controls--controller-v35).

## Current upload workflow — Arduino CLI

`scripts/knob.sh upload controller` delegates uploading to **Arduino CLI and the installed Arduino-Pico core's Picotool recipe**, matching the IDE configuration `Seeed XIAO RP2040` / `Upload Method: Picotool`. The core recipe executes `picotool load <sketch>.ino.uf2 -f -x`. The wrapper reports memory and selects a matching serial port from Arduino CLI discovery; it sends no `status`/`boot` command, performs no 1200-baud sequence and adds no reboot/retry loop. The retired custom uploader and its tests have been removed; older recovery notes below describe historical attempts.

Close the monitor, then use the existing build:

```sh
scripts/knob.sh upload controller
```

To select a port explicitly, including when more than one matching serial device exists:

```sh
scripts/knob.sh upload controller --port /dev/cu.usbmodem1101
```

With no matching serial port, Arduino CLI runs the same core recipe and Picotool handles USB discovery. The standard Picotool recipe discovers its USB target itself; a serial `--port` does not constrain Picotool's target selection. Connect only the intended RP2040 for this test. Add `--dry-run` to display Arduino CLI's resolved command without executing the upload. Upload is verbose and returns the CLI's exit status directly. It does not rebuild. On macOS the sole upload-property override is `cmd`, selecting the pinned official 2.3.1 executable directory and libusb 1.0.30. The upload pattern itself is unchanged. An IDE using another board core, upload method or executable version would be a different comparison.

This change establishes a standard baseline; it does **not** establish reliable software entry into BOOTSEL. The console installation requires repeated software uploads without opening the fascia or pressing BOOT. User acceptance must include repeat upload and upload after a normal power cycle. No hardware was flashed by the agent.

## macOS Tahoe uploader correction — 2026-11-06

The Arduino package folder `pqt-picotool/5.0.0-9576866` actually contains Picotool **2.0.0**, linked to its adjacent libusb **1.0.27**. On this Mac running **26.6.2**, that library predates the libusb **1.0.30 Tahoe compatibility fix**. `scripts/knob.sh upload` now selects the separate project-local **official Raspberry Pi Picotool 2.3.1 macOS distribution**, which bundles libusb **1.0.30**. The core/compiler and firmware are unchanged by this correction. This removes a confirmed dependency mismatch; the user subsequently retried the custom uploader with these corrected versions and the same reset failure remained. The version upgrade did not resolve it.

The current workspace already has this uploader installed. Fresh setups install it with `setup-toolchain.py`; an existing checkout can install it without rebuilding or touching hardware:

```sh
python3 scripts/picotool-host.py setup
```

The official archive URL, version and SHA-256 are pinned in toolchain.json. Only the executable and its adjacent library are extracted; the runtime checks reject libusb older than 1.0.30 or a different Picotool version. No global Homebrew package or Arduino package file is overwritten. Picotool is BSD-3-Clause and libusb is LGPL-2.1-or-later; they remain local tool dependencies, not vendored firmware sources.

The next controlled test uses Arduino CLI's standard recipe as described above, replacing the failed custom sequence. The opening tool-version line should still read `USB uploader: Picotool 2.3.1 / libusb 1.0.30`.

For research findings and the remaining distinction between an unanswered CDC port and failed ROM re-enumeration, see [USB investigation](../docs/interface-diagnostic.md#macos-host-tool-investigation--2026-11-06).

## Pinned dependencies

Versions are recorded in [toolchain.json](toolchain.json): Arduino CLI 1.5.1, Earle Philhower Arduino-Pico core 6.3.1, SparkFun Qwiic Twist Arduino Library 1.0.4. Board FQBN: `rp2040:rp2040:seeed_xiao_rp2040:uploadmethod=picotool`. The upload method is also a compile option: it adds the USB maintenance interface to the running firmware.

Dependency license review: the SparkFun library's bundled `LICENSE.md` specifies MIT for code, while its source headers contain GPL notices. Resolve that inconsistency before distributing linked firmware binaries. The library is installed as a dependency, not copied into this repository. No code from the separate Sony-controller implementation has been incorporated.

## Setup and build

From the repository root on Apple Silicon macOS:

```sh
python3 scripts/setup-toolchain.py
scripts/knob.sh build diagnostic
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

This diagnostic reports raw events only. The separate [controller sketch](../docs/gestures.md) implements gestures and LED feedback; the current controller implements real requests; Sony learning/recognition remain pending. Snapshot reads check the device ID and exact returned byte count. Read failures suspend events and trigger a retry; this diagnostic is not a validated fault-tolerant controller. The difference register is shown for comparison but is no longer cleared or used to derive TURN events.

## USB updates without board-button access

The default UF2 uploader resets the serial device and searches for a mounted drive. That path has repeatedly failed in this setup. The selected Picotool mode adds a USB reset interface to the firmware and transfers firmware using USB directly. It avoids the mounted-drive discovery step. The original failure cause remains unconfirmed.

One-time transition: close the serial monitor; enter BOOT mode using the procedure that already worked; then copy the **Picotool-enabled** binary from the repository root:

```sh
cp build/diagnostic-picotool/diagnostic.ino.uf2 /Volumes/RPI-RP2/
```

After restart, send `h` in the serial monitor and confirm `USB maintenance: Picotool enabled`. The previous `build/diagnostic` binary does not enable this interface.

For subsequent updates, leave USB connected, close the serial monitor, then run:

```sh
scripts/knob.sh build diagnostic
scripts/knob.sh upload diagnostic
scripts/arduino.sh board list
scripts/knob.sh raw-monitor /dev/cu.usbmodem1412301
```

Use the current port from `board list` for the monitor; Picotool upload itself does not require a serial-port name. Connect only the intended RP2040 target when uploading: Picotool selects a device over USB. The helper does not compile implicitly during upload; build after each source change. With target `diagnostic`, it uploads only from `build/diagnostic-picotool` through the core-provided Picotool recipe (`load -f -x`). The build FQBN in `toolchain.json` must retain `uploadmethod=picotool`.

The user confirmed two successive Picotool uploads without pressing buttons or unplugging USB (2026-10-27). This is user-reported hardware validation, separate from compilation. The `h` command can display the maintenance banner. A firmware/USB failure can still require hardware recovery; Picotool is not a guarantee against every hang. Keep an accessible USB data connection in the installation design.

The user currently performs uploads. An agent must have an explicit instruction before flashing attached hardware, per AGENTS.md. No upload has been performed by the agent.

## Verification recorded 2026-10-27

- USB registry identifies Seeed XIAO RP2040; serial port `/dev/cu.usbmodem1412301` is present. This confirms enumeration, not diagnostic execution.
- The selected core's XIAO variant defines SDA as GPIO6 and SCL as GPIO7.
- Diagnostic v2 compilation with `--warnings all` passed: 61,784 bytes flash, 9,140 bytes global RAM; no warnings reported. Compile-time checks cover stationary counts and forward/reverse counter wrap.
- User reports the first upload succeeded after BOOT-mode UF2 copy. Button DOWN/UP events work; turning produces no reported TURN events. Root cause is unconfirmed.
- User reports a subsequent automatic upload failed after the board drive unmounted; unplugging and entering BOOT mode before re-uploading succeeded. Upload failure cause is unconfirmed.
- User-provided v2 serial output confirms +1/-1 TURN events, BUTTON DOWN/UP, and turns with button=DOWN. Count/difference change together; firmware=1.2. The original getDiff-based diagnostic failure is not fully explained.
- Picotool-enabled build passed: 62,640 bytes flash, 9,316 bytes global RAM. Two missing-initializer warnings originate in the core USB.cpp reset-driver structure (name/deinit); no sketch warnings reported. User confirmed successful Picotool upload and repeat upload without physical board access.

References: [core installation and first upload](https://arduino-pico.readthedocs.io/en/latest/install.html), [SparkFun library](https://github.com/sparkfun/SparkFun_Qwiic_Twist_Arduino_Library).

## Historical default-UF2 upload results

The original serial upload reported `No drive to deploy`. The user successfully flashed by unplugging USB, holding B/BOOT, reconnecting USB, releasing B, and copying the UF2 onto RPI-RP2. Automatic serial-reset uploads failed again after the first flash; manual BOOT mode worked again. The report that the drive unmounted does not establish why the upload failed. Picotool subsequently succeeded twice without physical intervention; retain these observations as the reason for replacing the default UF2 uploader.

## Initial diagnostic complete — RGB confirmation

On 2026-10-26 (client local date), the user reported that RGB worked after the r/g/b/o test instructions. Combined with the supplied rotation/button trace and two successful Picotool uploads, this completes the initial functional diagnostic. Gesture commands, installation checks, electrical fault/reset testing, and Sony/camera operation have not been validated by this milestone. Earlier entries used the execution host date; this entry uses the client local date.

## Assembled interface diagnostic

Build with `scripts/knob.sh build interface`. The user uploads with `scripts/knob.sh upload interface`. See [bench commands and wiring](../docs/interface-diagnostic.md).

## Bulk-volume tuning — v4.0

Quick unpressed L–R–L targets −10 volume steps and R–L–R targets +10, compensating for pulses already sent. Recognition is immediate; final-flick tail input is consumed until a configurable quiet interval, while reversal interrupts immediately. Ordinary turns remain proportional at default 30/30 ms. Use `flick` in `scripts/knob.sh console` to show settings; detent limits, timeouts, quiet interval and target count are adjustable in RAM. See [settings and accounting](../docs/gestures.md#bulk-volume-gestures--controller-v40). Agent builds/tests only; user performs upload and hardware validation.
