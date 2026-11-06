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
            with serial.Serial(port=None, baudrate=9600, timeout=1, exclusive=True) as connection:
                connection.port = port
                connection.open()
                connection.dtr = True
                time.sleep(0.1)
                connection.dtr = False
                connection.baudrate = 1200
        except serial.SerialException as error:
            print(f"Serial reboot reported {error}; checking USB re-enumeration.", file=sys.stderr)
    # After serial reboot, use ROM access without another forced reboot.
    command = [picotool, "load", firmware, "-v", "-x"]
    deadline = time.monotonic() + 12
    while True:
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode == 0:
            print(result.stdout, end="")
            return 0
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
