# Design and evidence

## Requirements and choices

- Sony XAV-9000ES; XIAO RP2040; SparkFun Qwiic Twist RGB.
- Knob at cigarette-lighter location; minimal footprint; no unnecessary PCB.
- Use a small soldered protoboard near the Sony to support the remote-interface components and connections (user decision, 2026-10-27). Twist and XIAO remain independently mounted. Final board dimensions and layout depend on the validated circuit.
- Unmarked cap, subtle RGB underglow, fixed detents; no haptic motor.
- Opaque knob: use full-brightness idle blue. Gesture colors are temporary acknowledgements; do not treat a local toggle as authoritative Sony camera/mute state.
- Hardware I2C preferred over software I2C.
- Removable wiring and USB access.
- Volume, Play/Pause, Mute and previous/next track control.
- Manual rear view without guidelines, actual reverse with guidelines. Camera 1 control is required: [circuit, harness and firmware plan](camera-control.md).

## Current gestures — controller v3.5

- Turn: volume.
- Short single press: Play/Pause.
- Press and turn: previous/next track.
- Hold still >=700 ms, release before ten seconds: Mute.
- Double press: toggle Camera 1 through EXT IN 1.
- Ten-second stationary hold: maintenance learning-mode toggle, with no Mute on release.

The user verified all six audio gestures with Sony, including hold/release Mute, in the [v4.0 bench test](bench-results-2026-11-12.md). Timing remains 30 ms debounce and a 300 ms double-click window. Held turns suppress click/Mute outputs; camera and mute indications are local feedback only. See [current controls and workflow](gestures.md#current-controls--controller-v35). Arbitrary CarPlay navigation and more capacity exploration are outside the requested scope.

## Sony interface evidence

Sony documents Custom steering-wheel learning and RC-SR1 remote wiring. An independent open-source rotary-controller implementation supports Sony via a digital potentiometer and switched connection.

Its Sony implementation uses these nominal resistances:

| Command | Ohms |
| --- | ---: |
| Volume up | 16800 |
| Volume down | 23600 |
| Mute/ATT | 4400 |
| Next | 8800 |
| Previous | 12100 |

It uses 50 ms command and release intervals. These are external implementation values, not Sony-published specifications or validated requirements for our receiver.

The selected through-hole prototype uses one LCC110, one MCP41050-I/P digital pot, one SN74HCT125N SPI buffer and a 2N3904 relay-input driver. The pot runs at 5 V; HCT input thresholds allow 3.3 V XIAO control while delivering 5 V logic to the pot. This buffer is required for the selected pot and does not drive the relay LED. Logic and Sony grounds are shared. See [the complete circuit](sony-interface.md) and [assembly procedure](assembly-and-test.md).

The user ordered the supporting parts on 2026-10-27, intentionally including spares. The user assembled and bench-tested the circuit; see [assembled results](bench-results-2026-11-05.md). Sony Input 1 was measured around +3.3 V and characterized with a 9.86 kohm load. Camera trigger/release, video, separate-input guidelines and reverse priority were user-verified. See [bench report](bench-results-2026-10-28.md). The assembled system passed functional command learning and operation. Independent supply transitions and fault behavior remain electrical checks. The [archive](archive/README.md) preserves the superseded five-switch, SMD AD8400 and earlier MCP research; none supplies current pin assignments.

No upstream firmware is included here. Review the upstream GPL-3.0 license before code reuse.

## References

- [Seeed pinout](https://wiki.seeedstudio.com/XIAO-RP2040/)
- [Twist hardware](https://github.com/sparkfun/Qwiic_Twist)
- [Twist guide](https://learn.sparkfun.com/tutorials/qwiic-twist-hookup-guide)
- [Sony System / steering-wheel settings](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940274.html)
- [Sony camera settings](https://helpguide.sony.net/ev/xav-9000es/v1/en/contents/TP1000940273.html)
- [Sony RC-SR1 wiring](https://www.sony.com/electronics/support/articles/00168680)
- [Sony installation manual](https://www.sony.com/electronics/support/res/manuals/5046/7d50ce75a9ea9f148dd92ba1db86be98/50462371M.pdf)
- [Existing knob firmware](https://github.com/pounamu-electronics/RE_SWC_FW)
- [Existing Sony implementation](https://github.com/pounamu-electronics/RE_SWC_FW/blob/main/lib/headunit_swc/src/sony/sony_swc.cpp)
- [Current component datasheets](sony-interface.md#sources)

References recorded 2026-10-26. Record hardware test results separately with dates and observed configuration.

## Integrated camera output

The current placement includes Camera 1 control using U2 channel 4, R8/R9 on the existing board and R10 inline in the Sony camera pigtail. This supersedes the second-LCC110 camera proposal. Controller v4.0 drove Camera 1 through the installed buffer and passed the reverse-override bench check. See camera-control.md for electrical budgets and release/priority acceptance.

## Observed steering-remote learning menu

Sony offers Prev Track, Play/Pause, Next Track, Source, Mute, Volume -, Volume +, Voice, Answer Call and Hang Up. The user accepted nine distinct codes during exploratory learning, then stopped the search after confirming ample capacity. The final requested scope is six audio functions (Volume +/-, Play/Pause, Mute, Next/Previous) and Camera 1 via EXT IN 1. See the [bench record](bench-results-2026-11-05.md) for accepted/colliding trial codes; they are not additional gesture assignments.
