# Gesture controller and live monitor

Controller v3 connects the existing gesture engine and LED feedback to the assembled Sony interface. The original Twist diagnostic and the separate interface bench diagnostic remain available. The user uploaded v3.4 and reports all five Sony assignments working: Volume +, Volume -, Play/Pause, Next Track, and Previous Track. See [current code assignments](sony-interface.md#calibration-model-and-firmware-contract).

## Active five-command calibration — controller v3.4

Sony now distinguishes the two endpoint assignments, Volume + at code 255 and Volume - at code 0 (user observation, 2026-11-06). These endpoints stay unchanged. Three interior candidates are spaced by estimated **input voltage**, rather than evenly spaced pot codes, because the Sony input's voltage divider is nonlinear.

| Gesture / Sony assignment | Code | Estimated loaded Input 1 voltage |
| --- | ---: | ---: |
| Right turn / Volume + | 255 | 2.4 V |
| Held right turn / Next Track | 248 | 2.6 V |
| Single click / Play/Pause (user assignment) | 238 | 2.8 V |
| Held left turn / Prev Track | 215 | 3.0 V |
| Left turn / Volume - | 0 | 3.2 V |

The user reports **all five assignments working** (2026-11-06), including Next Track at 248 and Previous Track at 215. These codes are therefore receiver-verified for this assembled interface and learned configuration. All voltages in the table remain estimates, not measured recognition thresholds. The firmware/console still calls the single-click action `mute_att`; that is a local name, while Sony learns the chosen function. Use `learn mute_att` or `set mute_att N` for this same single-click output. No firmware or wiring change is needed to assign it Play/Pause. Camera remains separate. The maximum distinct-command count is still unknown. A sixth candidate is raw code **230**, between Play/Pause 238 and Previous Track 215 (estimated voltage about 2.88 V). Select an unused Sony function such as Mute and send `probe 230` through the console. Wait for its six-second release; `enable` resumes the existing knob assignments. This probe does not change the five configured codes or add a gesture. Sony may accept it, collide with an existing assignment, or ignore it; record the response before another trial.

The 64/192 resistance measurements fit `Rtotal_kohm ≈ 57.35 − 0.21328125 × code`. With the measured 3.27 V open input and an inferred 1.1–1.2 kohm source resistance, `Vinput ≈ 3.27 × Rtotal / (Rsource + Rtotal)`. Endpoint totals are approximately 3.0 and 57.4 kohm. Estimated low-end current is about 0.8 mA, below the MCP41050 ±1 mA limit but above the original conservative 0.5 mA prototype target. This estimate applies to this measured Sony input; keep R3 unchanged. Actual endpoint voltage/current and power-transition acceptance remain open.

### Live calibration console

After the user builds/uploads controller v3.4, close other serial monitors and run:

```sh
scripts/knob.sh console
```

The console displays translated gestures and sends commands typed with Enter. It reconnects after USB loss and discards commands typed before connection. It does not upload firmware.

| Command | Effect |
| --- | --- |
| `codes` | Show the five current assignments |
| `set mute_att 230` | Change Mute to code 230; release active outputs and discard queued pulses |
| `learn mute_att` | Hold the configured Mute code for six seconds, then release |
| `probe 230` | Hold raw code 230 for six seconds without changing any assignment |
| `enable` | Exit serial calibration and resume normal knob control |
| `release` | Release outputs and suspend normal control |

`set` and `learn` accept `volume_up`, `volume_down`, `mute_att`, `next_track`, or `previous_track`. Codes must be decimal 0–255. Settings are **RAM-only**: reboot restores the table above. A `set` command releases active outputs but preserves the prior enabled/suspended state and the knob's local learning mode. `learn`/`probe` suspend normal gestures until `enable`; keep the console connected during their holds. A detected Twist failure releases outputs. No command reads Sony's recognition state.

To find boundaries, leave the voltage probes fixed on H4/H5 if desired. Select an unassigned function in Sony's learning screen, then issue one `probe N`. Record the code, voltage if measured, and Sony's response (distinct assignment, collision with an existing assignment, or ignored). Wait for release before the next probe. Narrow intervals between different responses by testing intermediate codes. An accepted assignment changes Sony's learning table; record or clear that trial assignment before comparing later probes. This manual procedure maps behavior relative to the current learned assignments, not a guaranteed universal threshold. Do not automatically sweep codes or interpret the green LED as a Sony acknowledgment.

The former v3.3 two-command profile and original 64/96/128/160/192 profile are superseded. The original profile produced collisions; evenly spaced resistance was insufficient.

## Knob-operated learning — controller v3.2

The Sony Custom assignment screen asks for a sustained button press. A normal 55 ms detent pulse did not register in the user's first attempt; the required duration and resistance recognition remain unverified. Learning mode supplies a bounded six-second hold instead.

1. Open Sony Custom steering-control learning.
2. Hold the knob **without turning for ten seconds**. Three amber flashes acknowledge entry. Release the knob; steady amber identifies the local learning mode.
3. Select Volume + on Sony, then turn one positive detent. Two green flashes acknowledge starting that command's six-second resistance hold. It releases automatically. Wait until it ends before assigning the next function.
4. Repeat for Volume - (negative turn), Mute (single click), Next Track (held positive turn), Prev Track (held negative turn). A multi-detent event starts only one hold. Input during an active hold is ignored, so it cannot extend the hold or change resistance.
5. Hold still for ten seconds again. Three blue flashes acknowledge exit, outputs release, and normal operation resumes. Release the button before further gestures.

A long hold fires once while still pressed and cannot also send Mute or a track command on release. Rotation earlier in the same press consumes it as a track gesture and prevents a mode toggle. Booting with the button held does not enter learning. Camera requests are released on entry and are not driven by learning gestures. On disconnect/reset learning mode ends and pending commands are dropped. There is no automatic learning-mode timeout; enter and exit with the ten-second hold as requested. Closing the serial monitor does not cancel knob-operated learning.

Entry/exit flashes are three 150 ms on / 150 ms off cycles; assignment feedback is two green cycles. Idle learning is full-brightness amber; a held button shows purple after transient feedback. These colors acknowledge local mode/requests, never successful Sony learning. The heartbeat includes `learning_mode`; monitor displays LEARNING MODE when active.

Serial fallback: `learn volume_up`, `learn volume_down`, `learn mute_att`, `learn next_track`, `learn previous_track` each release other outputs, hold the named code for six seconds and leave normal knob outputs suspended. `enable` resumes normal mode. `release`, invalid input, boot and detected Twist failures cancel learning. Serial-origin holds also cancel on serial disconnect.

## Real output backend — 2026-11-06

Each steering detent queues one 55 ms relay pulse, with a 1 ms pot settling interval and 55 ms minimum release gap. The queue holds eight pending detents; an oversized event is rejected in full, and queued work older than 750 ms is discarded. Camera on/off uses D2/GPIO28 separately and persists until the opposite request, reset, explicit release, or a detected input fault. The LED only acknowledges gestures and does not claim Sony state.

Normal operation starts automatically after a two-second USB maintenance window and successful Twist connection. GPIO outputs start low, pot CS high; initialization selects code 128 with the relay open. A five-second watchdog runs after initialization, and I2C has a 25 ms timeout. USB reporting is skipped unless there is space for a complete bounded record. Monitor closure does not suspend standalone operation. A Twist failure releases outputs/queue; reconnect establishes a new gesture baseline and resumes unless explicitly suspended.

Serial commands require Enter: `help`/`h`/`?`, `status`/`s`, `release`, `enable`, `boot`. `release` suspends outputs until `enable` or restart. Invalid/oversized commands also suspend outputs. `boot` releases outputs and enters BOOTSEL with the watchdog disabled, retaining the application-command upload recovery. `status` includes the uploader's `pot_requested` and `relay_requested` signature.

The monitor labels accepted hardware intentions `[REQUEST]`, rejected/unassigned intentions `[DROPPED]`, and historical simulations `[SIM]`. Accepted means queued or locally requested, not Sony recognition. Later queue expiry is not separately acknowledged. Hardware heartbeats show whether outputs are enabled. Camera preview in the protocol means the local request only.

Build with `scripts/knob.sh build controller`, then the user uploads with `scripts/knob.sh upload controller`. Use `scripts/knob.sh monitor` for translated events. First flash and test with Sony disconnected; then learn the five assignments from the current code table. Power the interface before the Sony and switch the Sony off first during controlled bench work. Installation approval still requires power-transition testing.

## Current mapping

Gesture recognition is user-verified. Sony distinguishes the volume endpoints; the three interior assignments remain candidates.

| Input | Intention |
| --- | --- |
| Turn positive / negative | Volume up / down, one step per detent |
| Short click | Mute / ATT request |
| Hold button and turn positive / negative | Next / previous track, one step per detent |
| Double click | Toggle manual-camera request |
| Hold for 700 ms to under 10 s, then release without turning | Ignored; reported as unassigned |
| Hold still for 10 s | Toggle learning mode; fires while held |

Positive/negative follow the Twist's counter direction; check the feel in the final mounting. No volume acceleration or track-repeat timer is implemented. Multiple detents in one sample are reported with a step count.

## Timing and interaction rules

- Button debounce: 30 ms of stable input. Rotation during press debounce is retained as held rotation; rotation during release debounce belongs to the held turn.
- Double-click window: second press begins within 300 ms of the first release. A single click therefore fires after that window expires. Edge timestamps use the start of the stable transition, so debounce does not shorten the window.
- Both clicks must be shorter than 700 ms and contain no held rotation. A valid double click emits only the camera request, never an intermediate mute.
- Turning consumes pending clicks. Held rotation produces track intentions and suppresses a click when released. A long second press cancels the first pending click too.
- A button held at startup/reconnect is ignored until released. Existing count is taken as the baseline, so reconnect does not replay old movement.
- A checked I2C read failure cancels pending clicks, releases the camera request, and suspends input. Reconnect establishes a new baseline. Restart also starts with camera preview off.
- Counts and timestamps wrap safely. There is no intentional output on idle.

The pure C++ state machine is [gesture_engine.h](../firmware/controller/gesture_engine.h). The hardware polling/JSON output is [controller.ino](../firmware/controller/controller.ino).

## LED feedback

The controller provides automatic RGB feedback for the opaque knob. Idle is full-intensity blue `(0, 0, 255)`, with no global dimmer. Palette and durations are in [led_feedback.h](../firmware/controller/led_feedback.h).

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

`--raw` also displays input samples. Repeated heartbeats are suppressed unless state changes. Labels distinguish simulations, hardware requests and dropped requests; none claims that the radio or camera actually changed.

## Keep using the original diagnostic

The original `firmware/diagnostic/diagnostic.ino` remains unchanged, including its low-brightness manual RGB tests. Build and explicitly upload it when hardware tests are needed:

```sh
scripts/knob.sh build diagnostic
scripts/knob.sh upload diagnostic
scripts/arduino.sh board list
scripts/knob.sh raw-monitor /dev/cu.usbmodem1412301
```

Use the current serial port. The diagnostic retains its `h`, `s`, `c`, `r`, `g`, `b`, `w`, `o` commands. The controller's commands are listed above; manual RGB testing stays in the diagnostic while the controller supplies automatic gesture feedback.

## Protocol

One JSON object per newline, protocol `v=1`; each record has `type`, `seq`, and device uptime `ms`. Types are `status` (fixed message), `input` (raw count/button), `heartbeat` (connection/camera-preview state), and `gesture` (action and positive step count). Controller v3 gesture records include `simulated=false` and a boolean `accepted`. Older simulation records use `simulated=true`.

```json
{"v":1,"type":"gesture","seq":12,"ms":2500,"action":"volume_up","steps":2,"simulated":true}
```

Action names: `volume_up`, `volume_down`, `mute_att`, `next_track`, `previous_track`, `camera_on`, `camera_off`, `long_press_ignored`. The host renders the firmware's decisions; it does not independently infer gestures from raw input. Firmware keeps processing input if the monitor is closed but does not buffer/replay past commands to a later monitor.

## Historical simulation verification

Run `python3 scripts/test.py`. It builds the exact gesture engine for the host with address/undefined-behavior sanitizers and runs Python monitor tests. Tests exercise timing, bounce, click/turn conflicts, startup-held behavior, counter/millis rollover, disconnect cancellation, malformed protocol data, framing, CLI stdin, and a virtual serial port. It never opens the physical XIAO.

On 2026-10-26 (client local date): 19 gesture scenarios and 15 monitor tests passed. Both controller and diagnostic compile for XIAO with Picotool. The controller uses 62,968 bytes flash and 9,360 bytes global RAM. The core emits the two previously observed missing-initializer warnings in USB.cpp; no controller warnings were reported. The controller has not been flashed or physically tested by the agent. The user subsequently reported all gestures functional through the live monitor.

Memory-report verification: four additional tests passed, including ELF-versus-generated-binary comparison for both sketches. The full controller flash image is 79,760 bytes (3.8% of its configured flash partition); static RAM is 9,360 bytes (3.6%). The 62,968-byte figure above is the narrower Arduino compiler summary. No hardware upload was performed to test this reporting change.

Bench acceptance: check both volume directions; isolated click fires once after the window; double click produces camera only; held turns produce tracks without mute/camera on release; long holds produce no mapped command. Confirm repeated Picotool updates still work with this sketch. Keep Sony disconnected throughout.

## User bench acceptance — 2026-10-26

The user reports all controller gestures functional in the live monitor. This verifies the serial simulation on the XIAO/Twist; it does not validate Sony electrical commands or camera switching. Repeat Picotool upload with this controller sketch remains a separate unchecked item.

USB observation during a change of physical location: initially neither USB serial nor a normal application USB device appeared. Manual BOOT mode presented RPI-RP2; after the user pressed R to exit bootloader mode, the application and monitor worked. The user initially had not pressed R when the monitor remained waiting. No firmware changes were made for this incident, and its initial connection failure has no confirmed cause.

## LED implementation verification — 2026-10-26

19 gesture scenarios, 10 LED scenarios, and 19 Python monitor/memory tests passed. LED tests cover full-brightness idle, press/release restoration, pulse expiry/retrigger, both camera directions returning to idle, fault clearing, and timestamp rollover. Controller v2 compiles: Arduino summary 63,528 bytes flash / 9,376 bytes static RAM; full flash image 80,320 bytes (3.8%). The two existing USB.cpp core warnings remain; no controller warnings were reported. The agent did not upload this LED build. After performing the upload, the user reported that all feedback works (2026-10-26 client local date). Gesture and LED feedback are now user bench-verified; actual Sony and camera behavior remain untested.

## Controller v3 implementation verification — 2026-11-06

`python3 scripts/test.py` passed 19 gesture, 10 LED and 10 Sony-output host scenarios, plus 55 Python tests. New coverage includes pot-before-relay ordering, pulse/release timing, repeated detents, bounded capacity, expiry (including stalled settling), camera independence, release/reenable without replay, timer rollover, gesture-to-output dispatch, hardware monitor labels and serial status retry before boot. These tests use callbacks/mocks or virtual serial ports, not physical Sony hardware.

XIAO controller compilation passed with the pinned toolchain. Full flash image: 86,640 / 2,093,056 bytes (4.1%); static RAM: 9,984 / 262,144 bytes (3.8%). Arduino section summary: 69,572 bytes flash. The initial rebuild reported the same two existing USB.cpp missing-initializer warnings; no sketch warnings. Current placement generators/checks and `git diff --check` passed. No hardware was flashed or energized by the agent. The user must upload and verify integrated gestures, software reboot/cold startup, actual Sony learning and pulse recognition.

## Learning-mode implementation verification — 2026-11-06

Passed 24 gesture scenarios, 12 LED scenarios, 13 output-driver scenarios and 55 Python tests. Added checks cover the exact ten-second threshold, one toggle per press, no release click/turn after a toggle, reentry on a fresh hold, held-turn suppression, boot-held suppression, clearing a prior camera request, timer rollover, entry/exit/assignment flash timing, bounded learning holds, input suppression during a hold, release cancellation and restoration of ordinary 55 ms pulses. No physical hardware was accessed by these tests.

Controller v3.2 compiles for XIAO RP2040: full flash image 87,976 bytes (4.2%); static RAM 10,000 bytes (3.8%). No sketch warnings were emitted. `git diff --check` passed. The agent did not flash the board. User verification of mode entry/exit, visual feedback and Sony assignment recognition remains pending.

## Learning hold duration update — 2026-11-06

At the user's request, learning holds are now six seconds for both knob and serial initiation, providing more time for voltage measurements. Ten-second mode entry/exit, LED patterns and normal 55 ms command pulses are unchanged. Driver tests verify the hold remains asserted past three seconds, releases at six seconds, and handles timestamp rollover. Sony currently groups the tested gestures as Volume Down; whether loaded output voltages differ is still awaiting measurement.

## Endpoint-test build verification — 2026-11-06

Controller v3.3 compiles for the XIAO (Arduino section summary 71,236 bytes; static RAM 10,000 bytes). Passed 24 gesture, 12 LED, 13 output-driver scenarios and 57 Python tests. Driver coverage checks exact 0/255 volume assignments and rejection of Mute/Next/Previous in both ordinary and learning paths, plus pulse/hold release behavior. `git diff --check` passed. User upload and physical receiver endpoint validation remain pending.

## Controller v3.3.1 USB maintenance change

The active two-command test profile is unchanged. The serial `boot` command now releases outputs and schedules a USB detach followed by ROM entry after allowing the uploader to close its serial handle. No gesture processing resumes once boot is pending. See [upload recovery details](interface-diagnostic.md#explicit-usb-detach-before-controller-bootsel--2026-11-06). This change is intended to improve software update reliability; actual repeat-upload acceptance remains pending.

## Five-command calibration verification — 2026-11-06

Controller v3.4 compiled for XIAO RP2040: full flash image **89,496 / 2,093,056 bytes (4.3%)**, static RAM **10,012 / 262,144 bytes (3.8%)**. Passed 24 gesture, 12 LED, 17 Sony-output and 3 USB-maintenance scenarios, plus 60 Python tests. New coverage checks runtime assignment changes, release/queue clearing on reconfiguration, strict code parsing, all five learning assignments, bounded raw probes without altering assignments, calibration rendering and interactive command transmission over a virtual serial port. `git diff --check` passed. The agent did not flash hardware. Interior-code recognition and physical repeat-upload reliability remain pending user observations.
