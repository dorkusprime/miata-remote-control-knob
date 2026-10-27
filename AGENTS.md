# Working agreements

- Scope: this knob controller, Sony interface, camera controls, and associated power. Do not modify parent vehicle-maintenance files.
- Preserve the minimal mechanical design. A small soldered protoboard near the Sony is approved for the remote-control interface. No carrier PCB is planned for the Twist and XIAO; justify any further boards.
- The opaque knob needs full-brightness idle RGB. LED feedback acknowledges observed input/gestures; never latch a camera or mute indication based on unverified head-unit state.
- Use hardware I2C on XIAO D4/D5 for the Twist. Verify board labels versus actual RP2040 GPIO numbers.
- Keep proposed wiring distinct from bench-validated wiring. The selected prototype in docs/sony-interface.md uses one LCC110 plus MCP41050-I/P and SN74HCT125N and shares logic/Sony ground. Earlier five-switch, mux, AD8400 and other MCP candidate diagrams/cable allocations are under docs/archive and hardware/archive; do not build from them. The active HCT125-buffered MCP41050 design must not be confused with those candidates.
- Current interface uses through-hole parts; AD8400/SMD assembly and sourcing are historical alternatives.
- Start with USB serial diagnostics; Sony remote and camera outputs remain disabled until the interface is validated.
- Flashing attached hardware or energizing the Sony requires an explicit instruction to perform that hardware action.
- Never wire vehicle 12 V or the Sony remote input directly to XIAO GPIO.
- Design hardware and firmware defaults to release remote commands and manual camera requests on startup/reset/error.
- Actual reverse must take priority over manual view. Guidelines should appear only in reverse; actual Sony behavior remains to be tested.
- Do not commit credentials, unrelated personal/vehicle files, or recordings. This repository is public.
- Never mark hardware tests complete without actual observations.
- Review upstream licenses before incorporating third-party firmware.
