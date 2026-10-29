# Sony bench results — 2026-10-28

User-performed tests reported in the project conversation. This records observations and calculations; the agent did not operate hardware. No photos, scope captures, calibrated instrument specification or firmware-driven Sony commands were supplied.

## Configuration and scope

- Sony XAV-9000ES powered successfully from the newly available bench supply set to nominal 12 V. Supply model, current limit, startup current and loaded rail voltage were not recorded. Previous supply-collapse cause remains unknown.
- Steering tests used the included adapter's labeled Input 1 and ST Remote GND, with Custom steering-wheel mode requested in the procedure. No button-learning success was reported.
- XIAO/protoboard disconnected throughout Sony measurements and camera-trigger tests. Ordered interface components had not arrived at the last user update; no circuit assembly or calibration occurred.
- Camera tests used an independent nominal 5 V supply sharing Sony power ground, with a nominal 1 kohm series resistor feeding EXT IN 1. Steering-remote wiring was disconnected for camera testing.
- Actual camera supplied live video. Model, supply voltage/current and cut-wire identities were not recorded. Default video mode worked; do not infer AHD support from this.
- No RCA splitter available. Camera RCA moved between CAMERA IN 1 and REAR CAMERA IN for separate tests.

## Steering measurements

| Item | User report | Interpretation |
| --- | --- | --- |
| Initial Input 1 measurements | -3.27 V, later -3.3 V | Meter leads were reversed; not a negative Sony output |
| ST Remote GND versus power ground | -0.02 V with reversed leads | Grounds approximately equal; corrected magnitude about 20 mV; not a precision bonding measurement |
| Unloaded Input 1 | Approximately +3.3 V after polarity correction | Use 3.30 V as an estimate, not a separately recorded high-precision reading |
| Test resistor measured unpowered | 9.86 kohm | Use measured resistance for calculation |
| Input 1 with test resistor to ST Remote GND | 2.939 V | Loaded input measurement |

Assuming a steady Thevenin pull-up: I = 2.939 / 9860 = 0.2981 mA. Rsource = 9860 × (3.30 / 2.939 − 1) = 1211 ohm. Using 3.27 V instead gives about 1110 ohm. Thus roughly 1.1–1.2 kohm is an estimate from one load point, not a Sony specification or a characterized range over operating conditions.

Estimated command currents using 3.30 V and 1211 ohm source resistance:

| Intention | Nominal total resistance | Estimated current |
| --- | ---: | ---: |
| Mute/ATT | 4.4 kohm | 0.588 mA |
| Next | 8.8 kohm | 0.330 mA |
| Previous | 12.1 kohm | 0.248 mA |
| Volume up | 16.8 kohm | 0.183 mA |
| Volume down | 23.6 kohm | 0.133 mA |

All are estimates below MCP41050's ±1 mA wiper limit. Mute exceeds the previous <=0.5 mA prototype target. This is not a completed acceptance gate: keep that target visible, confirm actual command resistance/current and receiver learning, then explicitly resolve the margin or revise the design before enabling hardware. At the present source estimate, <=0.5 mA would require >=5.39 kohm total command resistance; whether Sony can learn that for ATT is untested. Do not change firmware calibration values based only on this calculation. Power-loss analog-terminal limits remain unresolved.

## Camera observations

| Test | User-observed result |
| --- | --- |
| EXT IN 1 disconnected, relative to Sony black power ground | 0.019 V |
| Nominal 5 V source through nominal 1 kohm, EXT IN 1 active | 4.87 V |
| Trigger assertion with no video attached | Camera 1 selected; video-signal-not-detected message |
| Trigger release | Previous screen restored |
| Powered camera connected to CAMERA IN 1 | Live picture displayed |
| Camera-generated guidelines disabled, Camera 1 preview | No lines; user confirmed proposed test sequence worked |
| Camera moved to REAR CAMERA IN; REVERSE IN at nominal +12 V | Rear picture with configurable Sony guidelines; user confirmed sequence worked |
| REVERSE IN active while EXT IN 1 also asserted | RearCam retained priority; user confirmed sequence worked |

If source voltage was exactly 5.00 V and series resistance exactly 1000 ohm, trigger load was (5.00 − 4.87) / 1000 = 0.13 mA. Neither exact value was recorded, so this is only a nominal estimate. Active trigger voltage 4.87 V was measured and camera operation was observed. The assembled HCT125 output still requires loaded voltage/current verification; an external supply test is not a buffer test.

## Decisions supported

- Keep manual preview on CAMERA IN 1 / EXT IN 1 and actual reverse on REAR CAMERA IN / REVERSE IN.
- Double-click may toggle the local trigger request: assertion/release is supported by observed Sony behavior. This is not feedback of actual display state; retain acknowledgement-only LEDs.
- Keep reverse trigger independent of MCU. Sony's priority was observed with both trigger signals present.
- Disable camera-generated lines and use configurable Sony lines for reverse only; desired separation was observed with one input at a time.
- Keep compact spare-HCT125-channel design pending its own electrical acceptance. No evidence requires another relay, larger board or video amplifier at this point.
- Keep R8 input pull-down. Retain R9 in the checked candidate for now; it is redundant while U2 is powered/enabled and does not prove safe behavior unpowered. R10 limits current; it is not isolation or comprehensive transient protection.
- Head unit can be packed away while waiting for components/splitter. No head-unit-only bench test is a prerequisite to continued firmware planning or staged interface assembly.

## Remaining acceptance, grouped by required equipment

### Components and assembled interface

1. Inspect parts, socket fit, formed-lead clearance, connector orientations, insulation and strain relief; use current placement, not old drawings.
2. Assemble permanent board in stages; a duplicate solderless build is optional. Verify rails and no shorts with chips removed where practical.
3. Implement a separately gated driver/calibration diagnostic. No automatic flashing or live-output activation.
4. Measure command resistances, relay LED current, simulated-source current and both supply-loss orders. Resolve ATT current target explicitly.
5. Verify camera default low, assertion/release, >=3.5 V loaded trigger and <=0.25 mA trigger load; record actual source and resistor values. Test reset/unplug/watchdog and Sony-only/controller-only power states.

### Sony plus assembled interface

1. Learn/recognize all five steering commands; measure actual loaded currents and calibrate pulse timing/repeat behavior.
2. Repeat camera operation using the actual buffer rather than bench 5 V. Verify touchscreen Back while request remains high and retrigger semantics.
3. Remove reverse while manual request remains asserted; record whether Camera 1 resumes. Do not assume automatic request clearing without a reverse-sensing input.

### RCA splitter and installation

1. Compare direct connection with both Sony camera inputs connected simultaneously; check brightness, color, sharpness and sync in both modes, including inactive-input loading.
2. Repeat live-video reverse/manual transitions and guideline separation with both branches attached.
3. Add video switching/buffering only if observed loading requires it.
4. Confirm camera model, supply requirements and accessory-switched power available during manual preview; reverse-only camera power will not suffice.
5. Final vehicle mounting, fusing, converter/transient behavior and cable checks remain separate from bench acceptance.

## Source references

- [Sony camera operation and priority](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940244.html)
- [Sony camera settings](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940273.html)
- [Sony steering-wheel settings](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940274.html)
- [Sony operating instructions, connection section](https://www.retailspecs.com/files/pdf/attachment/121511/XAV9000ES_O.pdf): positive 3.3–12 V EXT IN signal.
- Component datasheets linked in [circuit guide](sony-interface.md#sources).

The camera's AHD-select loop was discussed. Crutchfield describes composite inputs; Sony manuals reviewed did not explicitly establish AHD capability or exclusion. Leave format selection as in the working bench configuration and do not report an AHD test as passed.

## User-observed steering-remote learning options

The user subsequently supplied the exact available function list from this receiver's learning screen (observation date not separately recorded):

- Prev Track
- Play/Pause
- Next Track
- Source
- Mute
- Volume -
- Volume +
- Voice
- Answer Call
- Hang Up

These are observed assignable functions, not successfully learned commands. Camera 1 is absent, supporting the separate EXT IN 1 trigger plan. The selected initial gesture mapping remains turn = Volume -/+, short press = Mute, held turn = Prev/Next Track, double press = separate camera request. Use Sony's observed label Mute; earlier ATT terminology came from the external resistor implementation, not this menu. Actual mute behavior, resistance recognition and timing still require testing. Play/Pause, Source, Voice and call functions are available for future mappings; no gestures or resistor values are assigned to them by this observation.
