# Interface board bench checks — 2026-11-05

The assembled board was tested with the XIAO and Sony disconnected. These measurements were taken before the interface diagnostic firmware was built.

| Check | Result |
| --- | --- |
| R1 | 328 ohm |
| Power-to-ground resistance, chips removed | Settled at 2.79 Mohm |
| R2 | 996 ohm |
| Clock connection | Passed after installing missing W11 |
| Data connection | Passed after moving W12 from J11 to J12 |
| Pot wiper jumper | Continuity passed |
| R3 | 2.692 kohm |
| R4/R5 | 47 kohm each |
| R6 | 46.9 kohm |
| R7/R8 | Reported good |
| R9 | 47 kohm confirmed after an inconsistent pad/probe reading |
| Supply at both IC sockets and C3 | Approximately +5 V |
| Supply with chips installed, idle | 5 V, displayed 0 A |
| Q1 base and camera output at idle | 0 V |
| R1 drop at idle | 0 V |
| Camera request driven from 5 V through temporary 1 kohm resistor | H9 output 4.98 V |
| Camera request removed | H9 output 0 V |
| Relay test Q1 base / collector | 0.726 V / 0.01 V |
| Original LCC110 diode check pins 2–3 | OL in both directions |
| Spare LCC110 diode check pins 2–3 | 1.068 V |
| Replacement LCC110, relay drive asserted | Supply displayed 13 mA at 5 V |
| Replacement LCC110, R1 drop asserted | 3.73 V |
| Relay drive removed, R1 drop | 0 V |

R1 current calculated from the measured drop and resistance is 3.73 / 328 = 11.4 mA. The original LCC110 also had a bent output leg at F17. It was replaced after the diode comparison identified an open internal LED.

Relay LED drive and camera buffer assertion/release passed. Relay contact switching, programmed pot behavior, 3.3 V MCU control, reset/fault behavior and supply-order acceptance remain to be tested.

## MCU-controlled tests — completed through 2026-11-06

The user connected the complete XIAO-to-interface harness and powered it from USB. Twist shares XIAO ground via a Y connection with board B4. B9 is a redundant ground header and remains unused. The Sony stayed disconnected.

| Check | User observation |
| --- | --- |
| USB enumeration after harness connection | Initially missing; changing laptop USB port restored it despite valid 5 V/3.3 V rails |
| Interface serial startup | `ready=0 armed=0 pot_requested=128 relay_requested=0 camera_requested=0` |
| `arm bench` | ready=1, armed=1 |
| GPIO camera test | Assert/release worked |
| GPIO relay test, R1 voltage drop | 3.865 V asserted, then 0 V; about 11.8 mA using 328 ohm R1 |
| Pot 64, relay active, H5–H4 | 43.7 kohm |
| Pot 128, relay active, H5–H4 | Approximately 30 kohm after meter settling |
| Pot 192, relay active, H5–H4 | 16.4 kohm |
| Relay timeout | Output returned to OL after ten seconds |

Resistance measurements used a Fluke 27/FM, red H5 and black H4, in a manually selected high-kohm range. The initial 50 → 28 → 30 kohm display at code 128 was a settling observation, not a measurement of command latency. Actual short-pulse timing is unmeasured. No external voltage source or Sony was connected to the analog port during these resistance tests.

These results verify relay contact switching and SPI-controlled resistance adjustment, in addition to the earlier LED-drive and buffer tests. They do not verify Sony learning, command recognition or power transitions. The two endpoint readings imply approximately 54.6 kohm full pot resistance. Pin 5 is PA and pin 7 PB, correcting earlier swapped terminal names; the physical wiring is unchanged. Output resistance decreases as code increases. Controller candidates 96/160 remain interpolated, not directly measured.

### USB cold-start recovery

The user verified the interface firmware's serial `status`/`boot` path, successful repeated upload, and successful upload after unplugging/replugging. The current controller preserves that maintenance path. That controller build has not yet been uploaded or cold-start-tested. The later host-port enumeration incident has no confirmed firmware cause.

### Next acceptance

User uploads controller v3 with Sony disconnected, checks gestures/LED and software reboot, then performs controlled Sony Custom learning of the five code assignments. Interface must be powered before Sony and remain powered until Sony is switched off for these bench checks. Power-loss orders, reset/fault timing, final camera-buffer loading, final wiring and installation remain pending.

## Integrated controller and first Sony attempt — 2026-11-06

The user uploaded controller v3 and reported the integrated knob working (gesture/LED behavior). Sony was then connected. On the Custom learning screen, Sony requested “press and hold to assign steering button”; a one-detent turn did not register. Normal firmware sends 55 ms pulses. Pulse duration is a plausible explanation, not a confirmed diagnosis; acceptance of the chosen high-resistance codes has not yet been established.

Controller v3.2 adds a ten-second stationary hold to enter/exit learning and bounded three-second assignment holds from the existing gestures. Entry: three amber flashes; idle learning: amber; assignment request: two green flashes; exit: three blue flashes. The original diagnostics remain available. User upload and Sony acceptance of this new build are pending.

## Twist discovery failure after upload — 2026-11-06

Following successful upload by switching laptop USB ports, the user supplied v3.2 monitor output showing USB maintenance running but repeated “Twist unavailable; retrying in 2s”. The controller keeps outputs disabled. Comparing v3 with v3.2 shows no changes to I2C pin selection or Twist initialization. This does not establish a hardware versus firmware cause.

Controller v3.2.1 separates discovery failures: library begin/address response (including a follow-up I2C return code, instantaneous SDA/SCL levels and timeout flag), register-pointer write, short snapshot, unexpected device ID, color setup and count-limit setup. Pin levels are diagnostic observations, not proof of wiring continuity or power. No changed I2C timing, automatic bus scanning, or new recovery mechanism is introduced. The next hardware check is a full XIAO/Twist power cycle with Sony off; diagnosis remains pending.
