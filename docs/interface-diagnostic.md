# Interface bench diagnostic

The interface sketch controls the assembled MCP41050/HCT125/LCC110 board through the existing eight-wire harness. Use it with the Sony and all external signal sources disconnected.

## Build and upload

Run from the repository root.

```sh
scripts/knob.sh build interface
scripts/knob.sh upload interface
```

The user performs the upload. Picotool maintenance and the upload size report use the same tooling as the existing sketches.

For initial upload, leave the interface harness disconnected. Turn off and disconnect the bench supply from the interface. Then connect the harness per [wiring.md](wiring.md). Power the XIAO and interface together through laptop USB, with interface +5 V supplied from XIAO VBUS. Twist remains on 3V3. Use one 5 V supply path.

Open an interactive 115200-baud serial terminal. The existing raw monitor is available with the actual port:

```sh
scripts/knob.sh raw-monitor /dev/cu.usbmodemYOURPORT
```

Send each command followed by Enter. The gesture monitor parses a different protocol.

## Commands

| Command | Behavior |
| --- | --- |
| help | Print commands and bench conditions |
| status | Print requested pot code, arm state and output states |
| arm bench | Release outputs and enable commands for 60 seconds |
| pot 128 | Release outputs and write a decimal pot code from 0 through 255 |
| relay | Release other output, then assert relay drive for 10 seconds |
| camera | Release other output, then assert camera request for 10 seconds |
| release | Release both outputs and disarm |
| boot | Release outputs, disable watchdog and enter USB BOOTSEL for maintenance |

The diagnostic requests code 128 initially. It initializes SPI and programs that code when first armed. Writes use SPI mode 0 at 100 kHz with command byte 0x11. CS is D0/GPIO26, clock D8/GPIO2, MOSI D10/GPIO3, relay D1/GPIO27 and camera D2/GPIO28. D9/GPIO4 is configured as an unused SPI MISO input.

Invalid commands, input overflow, serial disconnect and arm expiry release both outputs and disarm. A five-second watchdog resets stalled firmware while bench commands are armed. Disarming disables it. Cold startup services USB maintenance before SPI initialization. Hardware pull-downs provide reset defaults. Each output assertion ends after ten seconds. SPI transactions and USB prints are bounded under normal core operation. Physical power-transition behavior still requires electrical acceptance.

Serial status describes software requests. Measure actual outputs and resistance to confirm device operation. MCP41050 has no SPI readback.

## First check

Send `status`. Expect ready=0, armed=0, pot_requested=128, relay_requested=0 and camera_requested=0. Measure H9 to B4 in DC volts and expect approximately 0 V.

Subsequent checks proceed one at a time. Relay contacts and pot resistance can be checked with a known current-limited source and voltage measurements. Check the meter's resistance-mode test voltage/current before using ohms mode on powered analog terminals. Continue to [assembly acceptance](assembly-and-test.md) for voltage, current and power-sequencing limits.

Microchip reference: https://ww1.microchip.com/downloads/en/DeviceDoc/11195c.pdf

## USB upload recovery

The upload helper detects the sole matching RP2040 serial device, requests BOOTSEL using the pinned core’s 1200-baud serial reset sequence, and retries Picotool ROM access for up to 12 seconds while USB re-enumerates. It holds DTR low after selecting 1200 baud and allows USB re-enumeration. If serial reset does not expose ROM mode, it requests a separate Picotool USB reboot before retrying the load. Close serial monitors before uploading. With no serial device, it tries an already-present BOOTSEL device once. Multiple matching serial devices are rejected before reset. This recovery is unit-tested with mocked USB operations. The user verified repeat flashing and flashing after unplug/replug with the interface firmware.

The uploader first sends status at 115200 baud. When the interface diagnostic identifies itself, the uploader sends boot, which disables the watchdog before entering ROM mode. A second upload passed before power cycling, but generic resets failed after unplug/replug. The user subsequently verified cold-start uploading through this application-command path. Controller v3 retains the same commands; repeat with that build.

### Missing serial versus USB recovery — 2026-11-06

The user again saw an upload failure with no serial reboot line. Read-only Arduino board listing showed only the host's Bluetooth/debug ports, and `picotool info -a` reported no accessible BOOTSEL device. The generic close-monitor message did not distinguish this condition. An earlier full power cycle had not restored the Twist; its specific initialization-failure cause is still pending the diagnostic upload.

The uploader now checks a failed Picotool load report for a single runtime RP2040/RP2350 USB device even when no serial port exists, and may request the existing USB reboot path in that case. Multiple reported runtime devices are rejected before reset; absence of both serial and a reported runtime/ROM target stops with an explicit detection error. It does not blindly force-reset devices or claim to repair a host enumeration failure. Eight mocked uploader tests passed. No live reset/upload was performed by the agent. A known data-capable replacement USB cable is the next proposed hardware check.

### Explicit USB detach before controller BOOTSEL — 2026-11-06

The next user upload found `/dev/cu.usbmodem1101` but received no application status response. The 1200-baud reset and Picotool reboot request did not expose ROM mode; Picotool still reported a runtime serial device. Read-only board listing afterward showed that same serial device; `lsof` reported no owner. An attempted agent-side status read was blocked by the session's device-access sandbox, so it supplied no additional evidence about application responsiveness.

Controller v3.3.1 replaces the immediate `reset_usb_boot` command with a deferred request: release outputs/disable watchdog, stop processing other serial commands and peripherals, wait at least 100 ms for the host to close CDC (one-second fallback for a terminal that remains open), call the pinned core's `USB.disconnect()` in loop context, then enter ROM. The core method explicitly detaches USB and, when connected, allows 500 ms for host detection. No USB reset from an interrupt handler or private core patches are added.

The uploader now asserts DTR/RTS before opening, allows 250 ms for line state readiness, collects fragmented status responses across reads for up to roughly four seconds, and closes immediately after flushing `boot` rather than holding the port open for another 300 ms. Re-enumeration retry follows closure. The existing diagnostic sketch retains its original immediate boot behavior; this firmware change applies to the integrated controller.

Rationale: [Arduino-Pico issue 3240](https://github.com/earlephilhower/arduino-pico/issues/3240) reports sensitivity to reset/detach timing on a different Linux/hub configuration and discusses explicit TinyUSB detach. This is supporting evidence for a recovery experiment, not proof that the Mac has the same cause. The software changes need an initial successful recovery upload and then repeated software uploads/cold-start testing. BOOTSEL has consistently worked by user report; the cable/port have previously transferred data. No live reboot/upload was performed by the agent.

Verification for v3.3.1: pinned XIAO build passed (88,496-byte flash image; 10,008-byte static RAM). Passed 24 gesture, 12 LED, 13 output-driver, three USB-maintenance timing scenarios and 58 Python tests, including fragmented status identification. Initial core rebuild emitted its two previously recorded USB.cpp initializer warnings; no sketch warnings. Final `git diff --check` passed. These are software checks, not proof that the intermittent USB failure is fixed on hardware.
