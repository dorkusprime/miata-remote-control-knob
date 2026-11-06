#!/usr/bin/env python3
"""Upload explicitly, using serial reboot before Picotool when USB serial is present."""
import argparse
import importlib.util
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
                connection.open()
                connection.dtr = True
                connection.write(b"status\n")
                identified = False
                for attempt in range(20):
                    if attempt in (5, 10, 15):
                        connection.write(b"status\n")
                    response = connection.readline()
                    if b"pot_requested=" in response and b"relay_requested=" in response:
                        identified = True
                        break
                if identified:
                    print("Knob firmware identified; requesting watchdog-safe boot.", flush=True)
                    connection.write(b"boot\n")
                    connection.flush()
                    time.sleep(0.3)
                else:
                    print("No interface response; trying 1200-baud reset.", flush=True)
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
        if port and not fallback_requested:
            # Request ROM mode separately so upload does not inherit Picotool's
            # runtime-device tracking (some firmware exposes an empty serial).
            print("Serial reset has not exposed ROM mode; requesting Picotool USB reboot.", flush=True)
            reboot = subprocess.run([picotool, "reboot", "-u", "-f"],
                                    capture_output=True, text=True)
            print(reboot.stdout + reboot.stderr, end="")
            fallback_requested = True
        if not port or time.monotonic() >= deadline:
            print(result.stdout + result.stderr, end="", file=sys.stderr)
            print("Upload failed. Close serial monitors and check USB data connectivity. "
                  "Software recovery requires working USB firmware or BOOTSEL.", file=sys.stderr)
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
