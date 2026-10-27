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
- Manual rear view without guidelines, actual reverse with guidelines.

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

The initial CD74HCT4051 proposal is superseded by an isolated PhotoMOS-switched resistor ladder. Five AQY212EH switches and seven fixed resistors produce the target values without a mux supply or GPIO connection to the Sony signal. See [the schematic, BOM, placement, and multimeter procedure](sony-interface.md). This is a reviewed prototype design, not a physically validated interface; receiver acceptance remains pending.

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
- [TI CD74HCT4051](https://www.ti.com/product/CD74HCT4051)

References recorded 2026-10-26. Record hardware test results separately with dates and observed configuration.
