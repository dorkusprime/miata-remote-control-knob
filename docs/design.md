# Design and evidence

## Requirements and choices

- Sony XAV-9000ES; XIAO RP2040; SparkFun Qwiic Twist RGB.
- Knob at cigarette-lighter location; minimal footprint; no unnecessary PCB.
- Use a small soldered protoboard near the Sony to support the remote-interface components and connections (user decision, 2026-10-27). Twist and XIAO remain independently mounted. Final board dimensions and layout depend on the validated circuit.
- Unmarked cap, subtle RGB underglow, fixed detents; no haptic motor.
- Opaque knob: use full-brightness idle blue. Gesture colors are temporary acknowledgements; do not treat a local toggle as authoritative Sony camera/mute state.
- Hardware I2C preferred over software I2C.
- Removable wiring and USB access.
- Volume, mute/ATT, track control.
- Manual rear view without guidelines, actual reverse with guidelines. Camera 1 control is required: [circuit, harness and firmware plan](camera-control.md).

## Implemented gesture defaults — user bench-verified

- Turn: volume.
- Short press: mute/ATT.
- Press and turn: previous/next track.
- Double press: toggle manual camera.

These are serial-only simulated intentions in the separate controller sketch. Initial timing: 30 ms debounce, 300 ms double-click window, 700 ms long-hold threshold (unassigned). Held rotation suppresses clicks; a double click does not emit mute. See [gesture rules and monitor workflow](gestures.md). The user reports all gestures functional in the serial simulation. Exact Sony command behavior remains untested. Arbitrary CarPlay menu navigation is not promised.

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

The user ordered the supporting parts on 2026-10-27, intentionally including spares. No circuit assembly has been reported. Sony Input 1 was measured around +3.3 V and characterized with a 9.86 kohm load. Camera trigger/release, video, separate-input guidelines and reverse priority were user-verified. See [bench report](bench-results-2026-10-28.md). Assembled-interface power transitions, ATT current margin and command learning remain acceptance gates. The [archive](archive/README.md) preserves the superseded five-switch, SMD AD8400 and earlier MCP research; none supplies current pin assignments.

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

The current placement includes Camera 1 control using U2 channel 4, R8/R9 on the existing board and R10 inline in the Sony camera pigtail. This supersedes the second-LCC110 camera proposal. No camera output firmware has been enabled. See camera-control.md for electrical budgets and release/priority acceptance.
