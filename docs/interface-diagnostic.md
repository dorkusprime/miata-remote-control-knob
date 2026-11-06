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

The pot starts at code 128. Writes use SPI mode 0 at 100 kHz with command byte 0x11. CS is D0/GPIO26, clock D8/GPIO2, MOSI D10/GPIO3, relay D1/GPIO27 and camera D2/GPIO28. D9/GPIO4 is configured as an unused SPI MISO input.

Invalid commands, input overflow, serial disconnect and arm expiry release both outputs and disarm. A one-second watchdog resets stalled firmware. Hardware pull-downs provide reset defaults. Each output assertion ends after ten seconds. SPI transactions and USB prints are bounded under normal core operation. Physical power-transition behavior still requires electrical acceptance.

Serial status describes software requests. Measure actual outputs and resistance to confirm device operation. MCP41050 has no SPI readback.

## First check

Send `status`. Expect armed=0, pot_requested=128, relay_requested=0 and camera_requested=0. Measure H9 to B4 in DC volts and expect approximately 0 V.

Subsequent checks proceed one at a time. Relay contacts and pot resistance can be checked with a known current-limited source and voltage measurements. Check the meter's resistance-mode test voltage/current before using ohms mode on powered analog terminals. Continue to [assembly acceptance](assembly-and-test.md) for voltage, current and power-sequencing limits.

Microchip reference: https://ww1.microchip.com/downloads/en/DeviceDoc/11195c.pdf

## USB upload recovery

The upload helper detects the sole matching RP2040 serial device, requests BOOTSEL using the pinned core’s 1200-baud serial reset sequence, and retries Picotool ROM access for up to 12 seconds while USB re-enumerates. It holds DTR low after selecting 1200 baud and allows USB re-enumeration. If serial reset does not expose ROM mode, it requests a separate Picotool USB reboot before retrying the load. Close serial monitors before uploading. With no serial device, it tries an already-present BOOTSEL device once. Multiple matching serial devices are rejected before reset. This recovery is unit-tested with mocked USB operations. Repeat flashing on the actual XIAO remains to be verified.
