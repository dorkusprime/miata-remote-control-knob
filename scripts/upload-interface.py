#!/usr/bin/env python3
"""Upload explicitly, using serial reboot before Picotool when USB serial is present."""
import argparse
import importlib.util
import re
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("gesture_monitor", ROOT / "scripts/gesture-monitor.py")
monitor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(monitor)


def upload(picotool, firmware, serial, ports):
    port = monitor.choose_port(None, ports)
    if port:
        print(f"Requesting BOOTSEL through {port}", flush=True)
        try:
            with serial.Serial(port=None, baudrate=115200, timeout=0.1,
                               write_timeout=1, exclusive=True) as connection:
                connection.port = port
                connection.dtr = True
                connection.rts = True
                connection.open()
                time.sleep(0.25)  # Let CDC line-state changes reach the firmware.
                connection.write(b"status\n")
                identified = False
                received = b""
                for attempt in range(40):
                    if attempt and attempt % 5 == 0:
                        connection.write(b"status\n")
                    received = (received + connection.readline())[-4096:]
                    if b"pot_requested=" in received and b"relay_requested=" in received:
                        identified = True
                        break
                if identified:
                    print("Knob firmware identified; requesting watchdog-safe boot.", flush=True)
                    connection.write(b"boot\n")
                    connection.flush()
                    # Close the host serial handle now. New controller firmware
                    # waits for this, then explicitly detaches USB before ROM boot.
                else:
                    print(f"No interface status signature ({len(received)} bytes received); trying 1200-baud reset.", flush=True)
                    if received:
                        print("Serial response sample: " + repr(received[-300:]), flush=True)
                    connection.baudrate = 1200
                    time.sleep(0.1)
                    connection.dtr = False
                    time.sleep(0.3)
        except serial.SerialException as error:
            print(f"Serial reboot reported {error}; checking USB re-enumeration.", file=sys.stderr)
    if port:
        time.sleep(1)  # Allow macOS to enumerate the ROM USB device.
    # After serial reboot, use ROM access without another forced reboot.
    command = [picotool, "load", firmware, "-v", "-x"]
    deadline = time.monotonic() + 12
    fallback_requested = False
    while True:
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode == 0:
            print(result.stdout, end="")
            return 0
        details = result.stdout + result.stderr
        # Picotool can see a runtime USB maintenance interface even when macOS
        # has not created a serial device. Count actual reports before reset;
        # never blindly reset an unidentified or ambiguous collection.
        runtime_devices = re.findall(r"Device at bus [^\n]*appears to be a RP(?:2040|2350) device", details)
        if not port and len(runtime_devices) > 1:
            print(details, end="", file=sys.stderr)
            print("Multiple runtime RP devices; connect only the intended XIAO before uploading.", file=sys.stderr)
            return result.returncode
        if (port or len(runtime_devices) == 1) and not fallback_requested:
            # Request ROM mode separately so upload does not inherit Picotool's
            # runtime-device tracking (some firmware exposes an empty serial).
            print("Requesting Picotool USB reboot into BOOTSEL.", flush=True)
            reboot = subprocess.run([picotool, "reboot", "-u", "-f"],
                                    capture_output=True, text=True)
            print(reboot.stdout + reboot.stderr, end="")
            fallback_requested = True
            if not port and reboot.returncode != 0:
                print("Runtime USB reboot failed; no XIAO serial port is available.", file=sys.stderr)
                return reboot.returncode
        if (not port and not fallback_requested) or time.monotonic() >= deadline:
            print(result.stdout + result.stderr, end="", file=sys.stderr)
            if not port and not fallback_requested:
                print("Upload stopped: no matching XIAO serial port, no accessible BOOTSEL device, "
                      "and no runtime USB device reported by Picotool. No reboot or flashing occurred. "
                      "Restore USB data detection, then rerun; closing monitors alone does not create a missing port.",
                      file=sys.stderr)
            else:
                print("Upload failed after requesting USB reboot. The device did not become accessible "
                      "in BOOTSEL; inspect USB detection before retrying.", file=sys.stderr)
            return result.returncode
        time.sleep(0.5)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("picotool")
    parser.add_argument("firmware")
    args = parser.parse_args()
    try:
        serial, ports = monitor.serial_modules()
        return upload(args.picotool, args.firmware, serial, ports)
    except (RuntimeError, OSError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
