# Gesture controller and live monitor

Controller v3.9 provides the six requested audio controls plus Camera 1 through EXT IN 1. The user verified five physical steering gestures in v3.4 and taught Sony the separate Mute code through a serial probe. The new hold/release Mute gesture still needs user testing after upload. Original diagnostics remain available.

## Current controls — controller v3.5

| Gesture | Sony function | Pot code |
| --- | --- | ---: |
| Turn left | Volume - | 0 |
| Turn right | Volume + | 255 |
| Short single press | Play/Pause | 238 |
| Hold and turn left | Previous Track | 215 |
| Hold and turn right | Next Track | 248 |
| Hold still for at least 700 ms, release before 10 s | Mute | 230 |
| Double click | Toggle Camera 1 request through EXT IN 1 | Separate output |
| Hold still for 10 s | Toggle local learning mode | Maintenance gesture |

These are the final requested functions. Source, Voice, Answer Call and Hang Up are outside the requested scope; learning-capacity probing is stopped. The user-reported accepted and colliding probe results are preserved in the bench record. No additional gestures or hardware are needed.

Mute fires **on release**, once. It does not fire while waiting for the ten-second learning toggle. A ten-second stationary hold emits only the learning toggle, even if the first threshold sample is the release. Held rotation consumes the press and suppresses both Play/Pause and Mute. A double click emits only the camera request. Booting/reconnecting with the button held suppresses that press until released.

Sony already learned all six required codes, so the existing assignments can stay after v3.5 upload. `play_pause` now names code 238; `mute_att` now names code 230. In v3.4 the historical `mute_att` name referred to single-click code 238; do not use that older name/code relationship with v3.5. The monitor reports Play/Pause and Mute separately. Both give brief green feedback; no latched mute state is inferred from Sony.

The accepted codes are spaced by estimated input voltage rather than uniform resistance. The measured resistance fit is `Rtotal_kohm ≈ 57.35 − 0.21328125 × code`, with a 3.27 V unloaded input and inferred 1.1–1.2 kohm Sony source resistance. These estimates do not measure recognition thresholds. The lowest endpoint is about 3.0 kohm total and 0.8 mA with this Sony source, below the MCP41050 ±1 mA limit. Keep R3 unchanged. Actual endpoint current and power transitions remain open.

### Live calibration console

After the user builds/uploads controller v3.5, close other serial monitors and run:

```sh
scripts/knob.sh console
```

The console displays translated gestures and sends commands typed with Enter. It reconnects after USB loss and discards commands typed before connection. It does not upload firmware.

| Command | Effect |
| --- | --- |
| `codes` | Show the six current assignments |
| `set mute_att 230` | Change Mute to code 230; release active outputs and discard queued pulses |
| `learn mute_att` | Hold the configured Mute code for six seconds, then release |
| `probe 230` | Hold raw code 230 for six seconds without changing any assignment |
| `enable` | Exit serial calibration and resume normal knob control |
| `release` | Release outputs and suspend normal control |
| `timing` | Show current normal press and release-gap durations |
| `timing 30 30` | Set press=30 ms and release gap=30 ms |

`set` and `learn` accept `volume_up`, `volume_down`, `play_pause`, `mute_att`, `next_track`, or `previous_track`. Codes must be decimal 0–255. Settings are **RAM-only**: reboot restores the table above. A `set` command releases active outputs but preserves the prior enabled/suspended state and the knob's local learning mode. `learn`/`probe` suspend normal gestures until `enable`; keep the console connected during their holds. A detected Twist failure releases outputs. No command reads Sony's recognition state.

To find boundaries, leave the voltage probes fixed on H4/H5 if desired. Select an unassigned function in Sony's learning screen, then issue one `probe N`. Record the code, voltage if measured, and Sony's response (distinct assignment, collision with an existing assignment, or ignored). Wait for release before the next probe. Narrow intervals between different responses by testing intermediate codes. An accepted assignment changes Sony's learning table; record or clear that trial assignment before comparing later probes. This manual procedure maps behavior relative to the current learned assignments, not a guaranteed universal threshold. Do not automatically sweep codes or interpret the green LED as a Sony acknowledgment.

The former v3.3 two-command profile and original 64/96/128/160/192 profile are superseded. The original profile produced collisions; evenly spaced resistance was insufficient.

## Knob-operated learning — controller v3.2

The Sony Custom assignment screen asks for a sustained button press. A normal 55 ms detent pulse did not register in the user's first attempt; the required duration and resistance recognition remain unverified. Learning mode supplies a bounded six-second hold instead.

1. Open Sony Custom steering-control learning.
2. Hold the knob **without turning for ten seconds**. Three amber flashes acknowledge entry. Release the knob; steady amber identifies the local learning mode.
3. Select Volume + on Sony, then turn one positive detent. Two green flashes acknowledge starting that command's six-second resistance hold. It releases automatically. Wait until it ends before assigning the next function.
4. Repeat for Volume - (negative turn), Play/Pause (single click), Mute (hold still >=700 ms then release before 10 s), Next Track (held positive turn), Prev Track (held negative turn). A multi-detent event starts only one hold. Input during an active hold is ignored, so it cannot extend the hold or change resistance.
5. Hold still for ten seconds again. Three blue flashes acknowledge exit, outputs release, and normal operation resumes. Release the button before further gestures.

A long hold fires once while still pressed and cannot also send Mute or a track command on release. Rotation earlier in the same press consumes it as a track gesture and prevents a mode toggle. Booting with the button held does not enter learning. Camera requests are released on entry and are not driven by learning gestures. On disconnect/reset learning mode ends and pending commands are dropped. There is no automatic learning-mode timeout; enter and exit with the ten-second hold as requested. Closing the serial monitor does not cancel knob-operated learning.

Entry/exit flashes are three 150 ms on / 150 ms off cycles; assignment feedback is two green cycles. Idle learning is full-brightness amber; a held button shows purple after transient feedback. These colors acknowledge local mode/requests, never successful Sony learning. The heartbeat includes `learning_mode`; monitor displays LEARNING MODE when active.

Serial fallback: `learn volume_up`, `learn volume_down`, `learn play_pause`, `learn mute_att`, `learn next_track`, `learn previous_track` each release other outputs, hold the named code for six seconds and leave normal knob outputs suspended. `enable` resumes normal mode. `release`, invalid input, boot and detected Twist failures cancel learning. Serial-origin holds also cancel on serial disconnect.

## Real output backend — 2026-11-06

Volume is proportional in v3.9: each detent requests one output pulse at any rotation speed. Three fast detents request three steps, not ten. Same-direction volume steps are stored as counted runs, preserving the count without using one queue slot per detent. There is no speed-triggered fixed burst and no short live-hold conversion. Press and release-gap defaults are **30/30 ms**, reflecting the user's current recognition-producing setting, and `timing PRESS GAP` adjusts them in RAM. Counted volume work has no age expiry; eight distinct queue entries still bound mixed runs and other commands, and non-volume commands retain their 750 ms age limit. Runs preserve order, including changes of direction. Large rolls can still require a drain tail because Sony must receive separate presses; the nominal 30/30 plus 1 ms settling rate is about 16 steps/s. Camera on/off uses D2/GPIO28 separately and persists until the opposite request, reset, explicit release, or a detected input fault. The LED only acknowledges gestures and does not claim Sony state.

Normal operation starts automatically after a two-second USB maintenance window and successful Twist connection. GPIO outputs start low, pot CS high; initialization selects code 128 with the relay open. A five-second watchdog runs after initialization, and I2C has a 25 ms timeout. USB reporting is skipped unless there is space for a complete bounded record. Monitor closure does not suspend standalone operation. A Twist failure releases outputs/queue; reconnect establishes a new gesture baseline and resumes unless explicitly suspended.

Serial commands require Enter: `help`/`h`/`?`, `status`/`s`, `release`, `enable`, `boot`. `release` suspends outputs until `enable` or restart. Invalid/oversized commands also suspend outputs. `boot` releases outputs and enters BOOTSEL with the watchdog disabled, retaining the application-command upload recovery. `status` includes the uploader's `pot_requested` and `relay_requested` signature.

The monitor labels accepted hardware intentions `[REQUEST]`, rejected/unassigned intentions `[DROPPED]`, and historical simulations `[SIM]`. Accepted means queued or locally requested, not Sony recognition. Later queue expiry is not separately acknowledged. Hardware heartbeats show whether outputs are enabled. Camera preview in the protocol means the local request only.

Build with `scripts/knob.sh build controller`, then the user uploads with `scripts/knob.sh upload controller`. Use `scripts/knob.sh monitor` for translated events. First flash and test with Sony disconnected; then learn the six assignments from the current code table. Power the interface before the Sony and switch the Sony off first during controlled bench work. Installation approval still requires power-transition testing.

## Current mapping

Gesture recognition is user-verified. Sony distinguishes the volume endpoints; the three interior assignments remain candidates.

| Input | Intention |
| --- | --- |
| Turn positive / negative | Volume up / down, one step per detent |
| Short click | Play/Pause request |
| Hold button and turn positive / negative | Next / previous track, one step per detent |
| Double click | Toggle manual-camera request |
| Hold for 700 ms to under 10 s, then release without turning | Mute, once on release |
| Hold still for 10 s | Toggle learning mode; fires while held |

Positive/negative follow the Twist's counter direction; check the feel in the final mounting. No volume acceleration or track-repeat timer is implemented. Multiple detents in one sample are reported with a step count.

## Timing and interaction rules

- Button debounce: 30 ms of stable input. Rotation during press debounce is retained as held rotation; rotation during release debounce belongs to the held turn.
- Double-click window: second press begins within 300 ms of the first release. A single click therefore fires after that window expires. Edge timestamps use the start of the stable transition, so debounce does not shorten the window.
- Both clicks must be shorter than 700 ms and contain no held rotation. A valid double click emits only the camera request, never an intermediate Play/Pause or Mute.
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
| Single click or Mute release recognized | Green `(0, 255, 0)` | 180 ms |
| Camera gesture recognized, either toggle direction | Amber `(255, 96, 0)` | 200 ms |

A pulse temporarily takes priority over held/idle color, then returns to the currently observed held/idle state. Track gestures retain the held-button purple; Mute release adds a brief green pulse. Pulses are brief solid-color changes, without blocking delays. Repeated same-color input extends the pulse without repeatedly writing RGB registers. Feedback runs even with the serial monitor closed.

There is no latched camera or mute LED state. Amber acknowledges the local gesture, not the Sony's actual camera view; green acknowledges Play/Pause or Mute input, not confirmed head-unit state. Neither color uses the local camera-preview toggle as persistent truth. Fault-triggered camera-preview cancellation produces no amber acknowledgement.

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

Action names: `volume_up`, `volume_down`, `play_pause`, `mute_att`, `next_track`, `previous_track`, `camera_on`, `camera_off`, `long_press_ignored`. The host renders the firmware's decisions; it does not independently infer gestures from raw input. Firmware keeps processing input if the monitor is closed but does not buffer/replay past commands to a later monitor.

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

## Six-control profile verification — 2026-11-06

Controller v3.5 compiled for XIAO RP2040: full flash image **89,584 / 2,093,056 bytes (4.3%)**, static RAM **10,016 / 262,144 bytes (3.8%)**. Passed 31 gesture scenarios, 13 LED scenarios, 18 Sony-output scenarios, 3 USB-maintenance scenarios and 63 Python tests. Coverage includes Play/Pause versus Mute code separation, Mute once on release at the 700 ms boundary, no Mute after held rotation, no extra Play/Pause on a long second click, cancellation on reconnect, timer wrap and ten-second learning suppression including release as the first threshold sample. The monitor's six-code calibration output and Play/Pause action are covered. `git diff --check` passed. The agent did not upload or reset hardware. The new Mute gesture remains pending user acceptance.

## Output timing trace — controller v3.5.1

The user reports roughly three seconds of continued volume-down activity after a fast turn and confirms normal mode, not learning mode. Normal firmware deliberately serializes each detent into a 1 ms pot settle, 55 ms relay assertion and 55 ms release gap (about nine steps/second). It queues up to eight pending steps, rejects an entire incoming batch that cannot fit, and drops commands older than 750 ms. Button debounce is 30 ms; ordinary turning is not delayed by the 300 ms click window. With a normally serviced loop, stale queued commands should not explain a three-second post-input tail; loop stalls, additional input, and receiver behavior must be distinguished by observations.

v3.5.1 keeps the six-control map and output timing unchanged and adds a bounded diagnostic trace. After user upload, run:

```sh
scripts/knob.sh monitor --outputs
```

Each actual change written by the relay GPIO callback is captured with its original device timestamp, commanded pot code, pending queue count, and age since the last observed audio gesture. The monitor renders `[OUTPUT @1234ms] PRESS/RELEASE code=0 pending=0; 680ms since last audio gesture`. These are GPIO-write observations, not direct measurements of PhotoMOS contacts or Sony's input voltage. Trace reporting occurs after output servicing, with up to 32 stored transitions; a lost-edge counter explicitly flags overflow. With no serial connection, stored records are discarded. Normal monitor output hides the trace unless `--outputs` is selected. Device timestamps, rather than host display timestamps, identify when each edge occurred.

Test: make one quick left roll, release the knob and wait until volume stops. Compare the final RELEASE/input-age against the observed Sony tail; share the trace around that roll. If output edges themselves run late, inspect the controller/input timeline. If GPIO activity ends promptly while Sony continues, investigate the remaining electrical/receiver path. No timing reduction, queue-policy change or Sony-delay diagnosis is claimed from this instrumentation alone.

Verification: 31 gesture, 13 LED, 18 output-driver, 3 USB-maintenance and 3 trace-buffer scenarios passed, plus 64 Python tests. The XIAO build uses 90,040 bytes flash (4.3%) and 10,412 bytes static RAM (4.0%). Two existing USB.cpp initializer warnings remain; no sketch warning. `git diff --check` passed. Agent did not upload or reset hardware.

## Fast-roll retention and pulse-rate tuning — controller v3.6

The user's v3.5.1 trace showed a 33-detent Volume Up roll: 10 steps accepted, 23 rejected at the eight-detent queue limit, eight pulses transmitted, and two accepted steps lost to expiry. The user clarified that the initial “three seconds” was a rough estimate; the concern is unacceptable knob lag, not reproducing that exact duration. Firmware debounce was not responsible for these drops.

v3.6 replaces consecutive volume detents with counted queue runs and removes age expiry for those runs. No heap allocation is used. Individual event counts are bounded to 65,536 (beyond the encoder's actual per-sample delta), total queued steps to uint32 range, and distinct queue entries to eight. Non-volume commands retain their prior bounded/age-limited policy. Reset, fault, maintenance, learning entry and explicit release still clear pending work. The change prevents the measured same-direction overflow pattern; it does not promise that arbitrary mixed-command workloads can never overflow or that all retained steps can be sent instantly.

The user uploaded v3.6 and supplied a normal-mode trace with **17 detents requested and 17 GPIO press/release pairs**, with no drops. The final release was 1,725 ms after the final observed turn. GPIO edges remained spaced about 111 ms apart: the configured 55 ms press + 55 ms gap + settling, rather than a mysterious three-second debounce. This verifies retention for that burst and demonstrates intentional pacing/backlog. It does not measure Sony's own processing delay or establish a usable final rate.

Next test, with no further firmware upload required:

```sh
scripts/knob.sh console --outputs
```

Enter `timing 25 25`, then test a single left detent to see whether Sony accepts one shorter command. Only after recognition is confirmed should a fast roll be compared. Settings may be shortened further if the receiver distinguishes pulses reliably; 25/25 is a trial, not an asserted solution. Enter `timing 55 55` to restore defaults. Each value must be 5–250 ms. Setting timing releases active outputs and clears pending commands while preserving prior enabled/suspended state; it can also release a manual camera request. Six-second learning holds and gesture debounce are unchanged. Timing is RAM-only, so reboot restores 55/55. At 25/25 the nominal rate is about 20 steps/s rather than nine; even this may leave unacceptable lag after a very fast roll. The usability goal is prompt response and stopping after the hand stops, not merely retaining a long backlog.

Verification after the interrupted workspace session was restored: 31 gesture, 13 LED, 23 driver, 3 USB-maintenance and 3 trace-buffer scenarios passed, plus 65 Python tests. Driver tests replay all 33 measured fast-roll steps without overflow/expiry loss, verify direction ordering, bounded mixed-run overflow, full-queue tail merging, adjustable timing, release/cancellation and unchanged six-second learning holds. XIAO compile passed: **91,056 bytes flash (4.4%)**, **10,740 bytes static RAM (4.1%)**. Agent did not flash or reset hardware. Faster timing remains pending user receiver testing.

## Live volume motion — controller v3.7

The user tried 55 ms press / 10 ms release in v3.6 and supplied a trace showing 19 volume detents in about 110 ms, followed by a 1,140 ms output tail. Shortening to 25/10 made Sony recognize only a few of dozens of commands. This does not establish an exact minimum pulse width; it shows that simply speeding up the counted FIFO was not reliable at that setting. An earlier all-DROPPED state cleared with `enable` and was a separate firmware gating issue, not evidence of Sony recognition speed.

The user then tested `learn volume_down` in normal Sony operation. They reported that the held command worked well and reduced volume to zero. This verifies repeat during that six-second test, but not Sony's initial repeat delay or behavior during a brief fast flick.

v3.7 uses this held-command behavior for fast turns and retains normal taps for slow detents. It starts with known-working 55/55 ms normal timing; the learned codes and all six audio/Camera 1 gestures remain the same. Rapid means a multi-detent sample or same-direction input less than 80 ms apart. Each motion sample refreshes a 100 ms idle release. If a same-code volume pulse is already asserted, it stays asserted; no repeated resistance writes are needed. If a release gap is underway, a fresh hold starts after that gap. Old queued rapid motion is not replayed after it has gone idle. This is an intentional change from the v3.6 promise to replay every detent: input describes live motion, and Sony controls repetition. Motion becoming idle, direction reversal, explicit release, fault or reset ends the applicable volume output; other controls retain their separate queued entries.

Reversal opens the relay immediately, removes queued volume motion, observes the normal gap and programs the other code before reasserting. This may truncate an old-direction tap when reversing; it prioritizes stopping the old direction over finishing its old request. Non-volume queued actions survive volume-queue cleanup. The stop target is about 100 ms after the final observed input, with loop/polling overhead, minimum press duration and any intervening gap affecting exact timing. A longer custom normal press setting can extend the minimum duration.

User test after upload: isolated slow detents first, then a short fast roll and stop; compare Sony's response and stopping with `monitor --outputs`. If Sony's initial repeat delay prevents useful response to short flicks, this candidate will need further adjustment. No satisfactory feel or short-flick behavior is claimed before user observation. The user is not asked to relearn Sony assignments.

Host verification: 31 gesture, 13 LED, 29 output-driver, 3 USB-maintenance and 3 trace-buffer scenarios plus 65 Python tests passed. Tests cover promotion of an existing tap to a hold, a replay of the measured fast-input batches producing one hold, idle release without backlog, direction reversal/gap, preservation of queued non-volume commands, stale motion, reset/cancellation, timestamp rollover and unchanged six-second learning holds. XIAO build passed: 91,504 bytes flash (4.4%) and 10,748 bytes static RAM (4.1%). No hardware flashed by the agent.

## Coarse fast flicks and timing command — controller v3.8

The user's v3.7 trace showed isolated 55 ms taps working and a 17-detent fast roll converted into a single **188 ms hold**. Sony responded with one step. Together with successful repetition during the earlier six-second hold, this shows the short live-hold approach does not deliver a useful large quick-flick change. The initial repeat delay itself was not measured. The user clarified that rapid turning should quickly change volume substantially, not preserve fine-grained detent counting. They also reported **25/55 ms did not register**; keep 55 ms presses for the next candidate.

v3.8 replaces brief rapid-turn holds with coarse pulse bursts. Slow motion is one step; fast flicks default to ten steps. Repeated same-flick samples refresh classification without multiplying burst size. The first tap is counted toward ten when a slow tap becomes rapid movement. Sustained fast movement continues at the available pulse rate after the initial burst. Direction reversal cancels queued old-direction volume and releases an old-direction active pulse before the opposite code; deliberate Play/Pause, Mute or track input also cancels remaining coarse work. A new flick after 150 ms quiet supersedes the previous volume request. This is intentional coarse control, not a one-pulse-per-detent promise. Other queued commands are preserved by volume cleanup.

The user asked for tuning through the existing `timing` command:

| Command | Meaning |
| --- | --- |
| `timing` | Display press, release gap, fast-flick steps and enabled/suspended state |
| `timing 55 10 10` | 55 ms press, 10 ms gap, ten-step coarse flick |
| `timing 55 10` | Change press/gap while keeping current coarse size |
| `timing 55 10 15` | Increase coarse flick target to fifteen steps |

Press/gap range is 5–250 ms and coarse size 1–20 steps. These are RAM-only, resetting on reboot to **55/55 ms and ten steps**. Valid changes release active outputs and clear pending work while preserving the prior enabled/suspended state. Invalid `timing` values now report an error and leave settings/output state unchanged, rather than disabling the knob. `timing` explicitly reports suspension and suggests `enable` when applicable. Other maintenance/fault handling remains in place.

At 55/10 plus 1 ms settling, the nominal rate is about 15 steps/s and ten steps take about 650 ms. That rate is the user's previously recognition-producing setting, not a fully measured lossless Sony throughput limit. User confirmation that a whole ten-step burst is recognized and feels useful remains required. The current default gap remains the known 55 ms setting; enter the desired trial timing after upload. No relearning is required.

Host verification passed 31 gesture, 13 LED, 31 driver, 3 USB-maintenance and 3 trace-buffer scenarios, plus 65 Python tests. Coverage checks exact coarse count for measured fast input batches, first-tap inclusion, bounded sustained movement, cancellation on reversal or another audio command, both timing syntax forms, parameter bounds and preserving coarse size with two parameters. XIAO build passed: 91,696 bytes flash (4.4%), 10,752 bytes static RAM (4.1%). Agent did not flash hardware.

## Proportional volume restored — controller v3.9

The user reports that 30/30 timing appears to work well but rejects the fixed-burst response: three quickly spun detents should not request ten steps. v3.9 removes that behavior and restores one pulse per detent for both slow and fast turns, with counted same-direction runs rather than the old eight-detent FIFO. The brief hold strategy is also removed. The six audio assignments, gestures and Camera 1 control are unchanged.

Defaults are now **30 ms press / 30 ms release**, based on the user's trial. Use `timing PRESS GAP` to tune; `timing` reports both values and output state. The optional third burst-size parameter is retired: it produces an error without changing settings or disabling output. Settings remain RAM-only, resetting to 30/30 on reboot. The monitor can still interpret old-version timing records for diagnostic compatibility, but current firmware does not emit a burst size.

This intentionally restores proportionality, not instantaneous delivery of arbitrary counts. At 30/30 plus settling the nominal output rate is about 16 steps/s; three pulses take about 153 ms from first press to final release. Large input bursts can still outpace transmission and produce a tail. Volume runs do not expire; different directions remain ordered rather than automatically cancelling each other. Eight mixed-run/other-command entries remain a finite capacity limit. These are explicit limits of the candidate, not a claim that all prior responsiveness concerns are resolved.

Host verification passed 31 gesture, 13 LED, 26 driver, 3 USB-maintenance and 3 trace-buffer scenarios plus 65 Python tests. New regression tests verify exactly three presses for a three-detent packet in either direction, exactly three for rapid single-detent samples, and actual default 30/30 timing. Prior run-counting tests verify all 33 measured fast-roll steps survive capacity/age handling, release clears pending work, direction order and pulse tuning. Build passed; agent did not flash hardware. User upload and proportional feel/recognition testing remain pending.
