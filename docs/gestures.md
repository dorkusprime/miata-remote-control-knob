# Gesture controller and live monitor

The controller is a separate sketch from the preserved hardware diagnostic. It translates input into intentions and streams JSON lines over USB. Sony and camera pins are never configured or driven. Every action is marked simulated; a camera request changes only the controller's preview state.

## Current mapping

These defaults are user bench-verified through the live monitor; Sony behavior remains untested.

| Input | Intention |
| --- | --- |
| Turn positive / negative | Volume up / down, one step per detent |
| Short click | Mute / ATT request |
| Hold button and turn positive / negative | Next / previous track, one step per detent |
| Double click | Toggle simulated manual-camera request |
| Hold for 700 ms or longer, then release without turning | Ignored; reported as unassigned |

Positive/negative follow the Twist's counter direction; check the feel in the final mounting. No volume acceleration or track-repeat timer is implemented. Multiple detents in one sample are reported with a step count.

## Timing and interaction rules

- Button debounce: 30 ms of stable input. Rotation during press debounce is retained as held rotation; rotation during release debounce belongs to the held turn.
- Double-click window: second press begins within 300 ms of the first release. A single click therefore fires after that window expires. Edge timestamps use the start of the stable transition, so debounce does not shorten the window.
- Both clicks must be shorter than 700 ms and contain no held rotation. A valid double click emits only the camera request, never an intermediate mute.
- Turning consumes pending clicks. Held rotation produces track intentions and suppresses a click when released. A long second press cancels the first pending click too.
- A button held at startup/reconnect is ignored until released. Existing count is taken as the baseline, so reconnect does not replay old movement.
- A checked I2C read failure cancels pending clicks, releases the simulated camera request, and suspends input. Reconnect establishes a new baseline. Restart also starts with camera preview off.
- Counts and timestamps wrap safely. There is no intentional output on idle.

The pure C++ state machine is [gesture_engine.h](../firmware/controller/gesture_engine.h). The hardware polling/JSON output is [controller.ino](../firmware/controller/controller.ino).

## LED feedback

Controller v2 adds automatic RGB feedback for the opaque knob. Idle is full-intensity blue `(0, 0, 255)`, with no global dimmer. Palette and durations are in [led_feedback.h](../firmware/controller/led_feedback.h).

| Observed input / recognized gesture | Color | Duration |
| --- | --- | --- |
| Idle | Blue `(0, 0, 255)` | Continuous |
| Debounced button held | Purple `(255, 0, 255)` | While held |
| Volume turn | White `(255, 255, 255)` | 120 ms from latest turn |
| Single click recognized | Green `(0, 255, 0)` | 180 ms |
| Camera gesture recognized, either toggle direction | Amber `(255, 96, 0)` | 200 ms |

A pulse temporarily takes priority over held/idle color, then returns to the currently observed held/idle state. Track gestures retain the held-button purple; unassigned long holds add no pulse. Pulses are brief solid-color changes, without blocking delays. Repeated same-color input extends the pulse without repeatedly writing RGB registers. Feedback runs even with the serial monitor closed.

There is no latched camera or mute LED state. Amber acknowledges the local gesture, not the Sony's actual camera view; green acknowledges click recognition, not a confirmed mute state. Neither color uses the local camera-preview toggle as persistent truth. Fault-triggered camera-preview cancellation produces no amber acknowledgement.

On a read failure, software cancels LED pulses and attempts to switch the LED off once. If communication itself has failed, LED-off cannot be guaranteed and the Twist may retain its last color. LED-write failures suspend input and trigger reconnection too. Reconnect clears old pulses and writes the observed held/idle color again.

## Build, user upload, and monitor

From the repository root:

```sh
scripts/knob.sh build controller
# Close any running serial monitor before the explicit upload:
scripts/knob.sh upload controller
scripts/knob.sh monitor
```

Build/upload default to `controller` if the target is omitted. Build artifacts are separate under `build/controller-picotool` and `build/diagnostic-picotool`. Upload does not rebuild; build after changing source. The controller requires the Picotool compile option, preserving the maintenance interface already verified with the diagnostic. Connect only the intended RP2040 when uploading. The user performs uploads; agents must follow AGENTS.md before touching attached firmware.

Before invoking Picotool, upload prints flash image and static RAM usage for the selected build. Preview it without touching the board with `scripts/knob.sh size` (or `size diagnostic`). The report reads load segments and static RAM sections from the ELF, and capacity from that build's generated linker script. Flash includes boot metadata and the initialized-data load image, which the Arduino compiler's section summary omits. Remaining RAM is not a measurement of peak runtime heap/stack use. Missing report artifacts stop the upload and request a rebuild.

The monitor detects the sole matching XIAO/RP2040 USB serial port, opens it at 115200, requests help, and prints translated actions immediately as events arrive. It reconnects after a USB disconnect; close it with Ctrl+C before uploading. It uses pyserial already bundled with the project toolchain; no pip installation is required. It never initiates an upload or sends a reboot command.

Example display (illustrative, not a hardware test result):

```text
21:40:02.100  [SIM] Volume up × 1
21:40:03.200  [SIM] Next track × 1
21:40:04.300  [SIM] Mute / ATT requested
21:40:05.400  [SIM] Manual camera request: ON
```

Options:

```sh
scripts/knob.sh monitor --port /dev/cu.usbmodem1412301
scripts/knob.sh monitor --raw
python3 scripts/gesture-monitor.py --stdin < recorded-events.jsonl
```

`--raw` also displays input samples. Repeated heartbeats are suppressed unless state changes. All action labels are marked `[SIM]`; the monitor never claims the radio or camera actually changed.

## Keep using the original diagnostic

The original `firmware/diagnostic/diagnostic.ino` remains unchanged, including its low-brightness manual RGB tests. Build and explicitly upload it when hardware tests are needed:

```sh
scripts/knob.sh build diagnostic
scripts/knob.sh upload diagnostic
scripts/arduino.sh board list
scripts/knob.sh raw-monitor /dev/cu.usbmodem1412301
```

Use the current serial port. The diagnostic retains its `h`, `s`, `c`, `r`, `g`, `b`, `w`, `o` commands. The controller's commands are only `h`/`?` for help and `s` for state; manual RGB testing stays in the diagnostic while the controller supplies automatic gesture feedback.

## Protocol

One JSON object per newline, protocol `v=1`; each record has `type`, `seq`, and device uptime `ms`. Types are `status` (fixed message), `input` (raw count/button), `heartbeat` (connection/camera-preview state), and `gesture` (action and positive step count). Gesture records include `simulated=true`.

```json
{"v":1,"type":"gesture","seq":12,"ms":2500,"action":"volume_up","steps":2,"simulated":true}
```

Action names: `volume_up`, `volume_down`, `mute_att`, `next_track`, `previous_track`, `camera_on`, `camera_off`, `long_press_ignored`. The host renders the firmware's decisions; it does not independently infer gestures from raw input. Firmware keeps processing input if the monitor is closed but does not buffer/replay past commands to a later monitor.

## Verification

Run `python3 scripts/test.py`. It builds the exact gesture engine for the host with address/undefined-behavior sanitizers and runs Python monitor tests. Tests exercise timing, bounce, click/turn conflicts, startup-held behavior, counter/millis rollover, disconnect cancellation, malformed protocol data, framing, CLI stdin, and a virtual serial port. It never opens the physical XIAO.

On 2026-10-26 (client local date): 19 gesture scenarios and 15 monitor tests passed. Both controller and diagnostic compile for XIAO with Picotool. The controller uses 62,968 bytes flash and 9,360 bytes global RAM. The core emits the two previously observed missing-initializer warnings in USB.cpp; no controller warnings were reported. The controller has not been flashed or physically tested by the agent. The user subsequently reported all gestures functional through the live monitor.

Memory-report verification: four additional tests passed, including ELF-versus-generated-binary comparison for both sketches. The full controller flash image is 79,760 bytes (3.8% of its configured flash partition); static RAM is 9,360 bytes (3.6%). The 62,968-byte figure above is the narrower Arduino compiler summary. No hardware upload was performed to test this reporting change.

Bench acceptance: check both volume directions; isolated click fires once after the window; double click produces camera only; held turns produce tracks without mute/camera on release; long holds produce no mapped command. Confirm repeated Picotool updates still work with this sketch. Keep Sony disconnected throughout.

## User bench acceptance — 2026-10-26

The user reports all controller gestures functional in the live monitor. This verifies the serial simulation on the XIAO/Twist; it does not validate Sony electrical commands or camera switching. Repeat Picotool upload with this controller sketch remains a separate unchecked item.

USB observation during a change of physical location: initially neither USB serial nor a normal application USB device appeared. Manual BOOT mode presented RPI-RP2; after the user pressed R to exit bootloader mode, the application and monitor worked. The user initially had not pressed R when the monitor remained waiting. No firmware changes were made for this incident, and its initial connection failure has no confirmed cause.

## LED implementation verification — 2026-10-26

19 gesture scenarios, 10 LED scenarios, and 19 Python monitor/memory tests passed. LED tests cover full-brightness idle, press/release restoration, pulse expiry/retrigger, both camera directions returning to idle, fault clearing, and timestamp rollover. Controller v2 compiles: Arduino summary 63,528 bytes flash / 9,376 bytes static RAM; full flash image 80,320 bytes (3.8%). The two existing USB.cpp core warnings remain; no controller warnings were reported. This LED build has not been uploaded by the agent or bench-verified by the user yet. Confirm colors under the opaque knob, return to blue after either camera gesture, standalone feedback with the monitor closed, and unchanged gesture behavior after user upload.
