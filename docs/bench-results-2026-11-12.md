# Knob Goblin bench record — 2026-11-12

The user tested the assembled Knob Goblin with the Twist, XIAO RP2040, MCP41050/HCT125/LCC110 interface, Sony XAV-9000ES and rear camera. Controller v4.0 ran with nominal 30 ms presses and 30 ms release gaps. The functional bench test passed. Vehicle installation and electrical characterization are separate checks.

## Earlier tests and fixes

| Stage | Result or fix | Detailed record |
| --- | --- | --- |
| Twist bring-up | Initial sketch saw button events only. Updated snapshot diagnostic confirmed both directions, held rotation, button events and RGB. Subsequent gesture and LED tests passed. | [Diagnostic checklist](tasks.md#encoder-diagnostics-and-follow-up-sony-disconnected) |
| Sony and camera alone | A 12 V bench supply powered the Sony after two earlier supplies sagged. Input 1 was approximately 3.3 V unloaded and 2.939 V with a measured 9.86 kΩ load. External triggering worked. Disabling the camera's own lines and using Sony's rear guidelines gave the intended two views. A later passive RCA split showed no visible degradation. | [October Sony bench record](bench-results-2026-10-28.md) |
| Interface assembly | Installed missing W11, corrected W12 from J11 to J12 and replaced the original LCC110 after finding a bent F17 leg and an open input LED. Verified continuity, 5 V rails, relay drive, contact switching and camera buffer operation. The replacement drew about 11.4 mA at the relay LED from the measured R1 drop. SPI produced 43.7, approximately 30 and 16.4 kΩ at codes 64, 128 and 192. The meter's settling time was observed separately from command timing. | [November board bench record](bench-results-2026-11-05.md) |
| Encoder discovery after flash | Twist returned an I²C address NACK until its loose ground connector was secured. | [November board bench record](bench-results-2026-11-05.md#sony-learning-collision-and-endpoint-test-request--2026-11-06) |
| Sony learning | Short normal pulses did not register on the initial assignment screen. Knob-operated learning uses a ten-second stationary hold to enter or exit, and six-second assignment holds. The first resistance profile collided. Endpoint codes 0/255 proved the two volume commands distinct, followed by the four interior codes below. Nine total codes were accepted during exploration. The maximum was left undetermined. | [Learning results](bench-results-2026-11-05.md#learning-exploration-concluded-final-scope--2026-11-06) |
| Fast volume turns | Early firmware dropped detents when its queue filled. Counted runs fixed that loss in the retest, but 55/55 ms and 55/10 ms still left a long output tail. Shorter 25 ms trials missed commands, and a 188 ms hold gave one Sony step despite a six-second hold repeating successfully. The user selected 30/30 ms with one pulse per ordinary detent and a separate alternating flick for bulk changes. | [Timing trials and firmware history](bench-results-2026-11-05.md#fast-turn-volume-tail-observation--2026-11-06) |
| USB programming | Manual BOOT recovery worked during repeated software BOOTSEL failures. Picotool/libusb updates and the standard Arduino CLI upload path did not eliminate the intermittent behavior. Moving laptop ports restored detection on multiple occasions. During box bring-up, bypassing the panel lead restored detection and reconnecting it also worked. The user successfully flashed through the installed panel path. A later monitor session logged one USB disconnect and reconnected automatically. The underlying USB fault and repeatability after cold start remain unconfirmed. | [Firmware and USB recovery](../firmware/README.md) |

## Assembled-box electrical checks

After assembly into the box, the user initially measured approximately 30 kΩ during the six-second learning holds and OL after release. Checking the SPI hookups and unplugging/reseating the connections restored distinct readings. The exact loose or misconnected contact was undetermined.

The following readings were taken across the steering output with the Sony disconnected. Camera output measured **0 V off and 5 V on**, also with the Sony disconnected. Earlier, the user identified the camera as a PMD2A-S and measured 0.037 A at 12 V on the bench. That reading describes the camera while powered in the tested configuration.

| Gesture | Sony assignment | Pot code | Measured resistance |
| --- | --- | ---: | ---: |
| Turn left | Volume − | 0 | 57.3 kΩ |
| Turn right | Volume + | 255 | 3.047 kΩ |
| Hold and turn left | Previous Track | 215 | 11.57 kΩ |
| Hold and turn right | Next Track | 248 | 4.53 kΩ |
| Single press | Play/Pause | 238 | 6.63 kΩ |
| Stationary hold and release | Mute | 230 | 8.37 kΩ |

## End-to-end results

The user reported that all six audio commands and the manual Camera 1 toggle worked on the Sony. The normal audio trace shows relay GPIO press/release pairs with 30 ms press widths and first presses about 1–2 ms after isolated requests. The trace records GPIO writes. Receiver response comes from user observation.

| Check | Result |
| --- | --- |
| Quick ordinary turns | Four input detents and four output pulses each way, with no bulk trigger. Final release followed the last detent by 145 ms up and 104 ms down. |
| Right–left–right | Opening pulses canceled to net zero. Firmware then sent 10 up pulses. Final-flick tail added no pulses. |
| Left–right–left | Opening pulses netted one step down. Firmware sent nine more down pulses for a net −10. |
| Reversal after a burst | In two attempts the reverse detent arrived after the queue drained and produced a normal opposite-direction step. |
| Reversal during a burst | A bulk-down flick started from net +1 and queued 11 down pulses. Seven were sent before the right detent canceled the remaining four. The up pulse started 7 ms after that detent, for a net −5 from the gesture start. |
| Camera preview and reverse priority | Double-click opened and closed Camera 1. With the preview open, applying bench +12 V to Sony REVERSE IN successfully exercised reverse override. Manual view uses no lines and reverse uses Sony lines. The user confirmed the test worked; the exact screen shown immediately after releasing reverse was not recorded. |
| Cold start without laptop | The user unplugged the laptop and cycled the 12 V bench supply. The setup came up with Camera 1 closed and responded to the first volume turn and camera double-click. |

Exact volume counts above come from the controller trace. The Sony's numeric volume readout was not recorded for those trials.

## Open checks

- Vehicle fit, cable routing, strain relief, switched camera power, and operation from the Miata's actual reverse-light signal.
- Loaded EXT IN 1 voltage/current, steering-input endpoint current, and independent supply-loss/restoration and fault behavior. The shared 12 V power cycle verifies ordinary startup.
- Sony touchscreen Back/retrigger and the specific screen immediately after reverse release with a standing Camera 1 request.
- Repeat software-only uploads through the installed panel jack, including after cold power-up.
