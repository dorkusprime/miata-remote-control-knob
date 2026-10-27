# Gesture controller and live monitor

The controller is a separate sketch from the preserved hardware diagnostic. It translates input into intentions and streams JSON lines over USB. Sony and camera pins are never configured or driven. Every action is marked simulated; a camera request changes only the controller's preview state.

## Current mapping

These are implemented defaults for user testing, not validated Sony behavior.

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

## Build, user upload, and monitor

From the repository root:

```sh
scripts/knob.sh build controller
# Close any running serial monitor before the explicit upload:
scripts/knob.sh upload controller
scripts/knob.sh monitor
```

Build/upload default to `controller` if the target is omitted. Build artifacts are separate under `build/controller-picotool` and `build/diagnostic-picotool`. Upload does not rebuild; build after changing source. The controller requires the Picotool compile option, preserving the maintenance interface already verified with the diagnostic. Connect only the intended RP2040 when uploading. The user performs uploads; agents must follow AGENTS.md before touching attached firmware.

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

The original `firmware/diagnostic/diagnostic.ino` remains unchanged. Build and explicitly upload it when hardware tests are needed:

```sh
scripts/knob.sh build diagnostic
scripts/knob.sh upload diagnostic
scripts/arduino.sh board list
scripts/knob.sh raw-monitor /dev/cu.usbmodem1412301
```

Use the current serial port. The diagnostic retains its `h`, `s`, `c`, `r`, `g`, `b`, `w`, `o` commands. The controller's commands are only `h`/`?` for help and `s` for state; RGB testing stays in the diagnostic.

## Protocol

One JSON object per newline, protocol `v=1`; each record has `type`, `seq`, and device uptime `ms`. Types are `status` (fixed message), `input` (raw count/button), `heartbeat` (connection/camera-preview state), and `gesture` (action and positive step count). Gesture records include `simulated=true`.

```json
{"v":1,"type":"gesture","seq":12,"ms":2500,"action":"volume_up","steps":2,"simulated":true}
```

Action names: `volume_up`, `volume_down`, `mute_att`, `next_track`, `previous_track`, `camera_on`, `camera_off`, `long_press_ignored`. The host renders the firmware's decisions; it does not independently infer gestures from raw input. Firmware keeps processing input if the monitor is closed but does not buffer/replay past commands to a later monitor.

## Verification

Run `python3 scripts/test.py`. It builds the exact gesture engine for the host with address/undefined-behavior sanitizers and runs Python monitor tests. Tests exercise timing, bounce, click/turn conflicts, startup-held behavior, counter/millis rollover, disconnect cancellation, malformed protocol data, framing, CLI stdin, and a virtual serial port. It never opens the physical XIAO.

On 2026-10-26 (client local date): 19 gesture scenarios and 15 monitor tests passed. Both controller and diagnostic compile for XIAO with Picotool. The controller uses 62,968 bytes flash and 9,360 bytes global RAM. The core emits the two previously observed missing-initializer warnings in USB.cpp; no controller warnings were reported. The controller has not been flashed or physically tested by the agent. User testing of gesture timing and monitor behavior is pending.

Bench acceptance: check both volume directions; isolated click fires once after the window; double click produces camera only; held turns produce tracks without mute/camera on release; long holds produce no mapped command. Confirm repeated Picotool updates still work with this sketch. Keep Sony disconnected throughout.
