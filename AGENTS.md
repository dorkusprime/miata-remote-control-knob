# Working agreements

- Scope: this knob controller, Sony interface, camera controls, and associated power. Do not modify parent vehicle-maintenance files.
- Preserve the minimal mechanical design. No extra carrier PCB unless necessary and justified.
- Use hardware I2C on XIAO D4/D5 for the Twist. Verify board labels versus actual RP2040 GPIO numbers.
- Keep proposed wiring distinct from bench-validated wiring.
- Start with USB serial diagnostics; Sony remote and camera outputs remain disabled until the interface is validated.
- Flashing attached hardware or energizing the Sony requires an explicit instruction to perform that hardware action.
- Never wire vehicle 12 V or the Sony remote input directly to XIAO GPIO.
- Design hardware and firmware defaults to release remote commands and manual camera requests on startup/reset/error.
- Actual reverse must take priority over manual view. Guidelines should appear only in reverse; actual Sony behavior remains to be tested.
- Do not commit credentials, unrelated personal/vehicle files, or recordings. This repository is public.
- Never mark hardware tests complete without actual observations.
- Review upstream licenses before incorporating third-party firmware.
