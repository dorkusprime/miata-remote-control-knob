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
