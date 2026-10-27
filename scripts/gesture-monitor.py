#!/usr/bin/env python3
"""Render controller JSONL events live. Does not flash or reset the board."""
import argparse
import datetime
import json
import pathlib
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
MAX_LINE = 4096
LABELS = {
    "volume_up": "Volume up",
    "volume_down": "Volume down",
    "mute_att": "Mute / ATT requested",
    "next_track": "Next track",
    "previous_track": "Previous track",
    "camera_on": "Manual camera request: ON",
    "camera_off": "Manual camera request: OFF",
    "long_press_ignored": "Long press ignored (unassigned)",
}


def clean(text):
    return "".join(c for c in str(text) if c.isprintable())[:500]


class Lines:
    """Bounded framing; discard oversized messages through their next newline."""
    def __init__(self):
        self.buffer = bytearray()
        self.dropping = False

    def feed(self, chunk):
        for byte in chunk:
            if byte == 10:
                if not self.dropping and self.buffer:
                    yield self.buffer.decode("utf-8", errors="replace").rstrip("\r")
                self.buffer.clear()
                self.dropping = False
            elif not self.dropping:
                self.buffer.append(byte)
                if len(self.buffer) > MAX_LINE:
                    self.buffer.clear()
                    self.dropping = True


class Renderer:
    def __init__(self, raw=False):
        self.raw = raw
        self.last_state = None
        self.warned_legacy = False

    def render(self, line):
        try:
            event = json.loads(line)
        except (ValueError, TypeError):
            if self.raw:
                return "[raw] " + clean(line)
            if not self.warned_legacy:
                self.warned_legacy = True
                return "Non-JSON output received; flash the controller sketch for gesture translation."
            return None
        if not isinstance(event, dict) or type(event.get("v")) is not int or event.get("v") != 1:
            return "Unsupported device message."
        kind = event.get("type")
        if kind == "gesture":
            action = event.get("action")
            if not isinstance(action, str) or action not in LABELS:
                return "Unknown gesture action."
            steps = event.get("steps")
            if type(steps) is not int or not 1 <= steps <= 65536:
                return "Invalid gesture step count."
            if event.get("simulated") is not True:
                return "Gesture message missing simulation marker; ignored."
            label = LABELS[action]
            if action in ("volume_up", "volume_down", "next_track", "previous_track"):
                label += f" × {steps}"
            return "[SIM] " + label
        if kind == "status":
            return clean(event.get("message", ""))
        if kind == "heartbeat":
            state = (event.get("connected"), event.get("camera_preview"))
            if any(type(value) is not bool for value in state):
                return "Invalid device state."
            if state == self.last_state:
                return None
            self.last_state = state
            return ("Twist " + ("connected" if state[0] else "missing") +
                    "; camera preview " + ("ON" if state[1] else "OFF") + "; outputs simulated")
        if kind == "input" and self.raw:
            return "[input] " + clean(json.dumps(event, separators=(",", ":")))
        return None


def output(message):
    if message:
        stamp = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
        print(f"{stamp}  {message}", flush=True)


def serial_modules():
    lock = json.loads((ROOT / "firmware/toolchain.json").read_text())
    bundled = (ROOT / ".tools/arduino/data/packages/rp2040/hardware/rp2040" /
               lock["core_version"] / "tools/pyserial")
    if not bundled.is_dir():
        raise RuntimeError("Toolchain missing; run python3 scripts/setup-toolchain.py first.")
    sys.path.insert(0, str(bundled))
    import serial
    from serial.tools import list_ports
    return serial, list_ports


def choose_port(explicit, list_ports):
    if explicit:
        return explicit
    candidates = [p.device for p in list_ports.comports()
                  if p.vid == 0x2E8A and p.pid is not None and (p.pid & 0x00FF) == 0x000A]
    if len(candidates) > 1:
        raise RuntimeError("Multiple RP2040 serial devices; select one with --port.")
    return candidates[0] if candidates else None


def monitor(args):
    serial, ports = serial_modules()
    announced_wait = False
    while True:
        port = choose_port(args.port, ports)
        if port is None:
            if not announced_wait:
                output("Waiting for XIAO USB serial port; Ctrl+C exits.")
                announced_wait = True
            time.sleep(1)
            continue
        try:
            # Configure before opening; never use the 1200-baud reset sequence.
            with serial.Serial(port=None, baudrate=115200, timeout=0.2,
                               write_timeout=1, exclusive=True) as connection:
                connection.port = port
                connection.dtr = True
                connection.rts = True
                connection.open()
                announced_wait = False
                output(f"Connected to {port}; all actions are simulations. Ctrl+C exits.")
                connection.write(b"h\n")
                renderer, lines = Renderer(args.raw), Lines()
                while True:
                    chunk = connection.read(min(connection.in_waiting or 1, 512))
                    if not chunk and not pathlib.Path(port).exists():
                        raise serial.SerialException("USB serial port disappeared")
                    for line in lines.feed(chunk):
                        output(renderer.render(line))
        except (serial.SerialException, OSError) as error:
            output(f"Serial connection unavailable: {clean(error)}; retrying. Close other monitors.")
            time.sleep(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="Serial port; otherwise detect the sole matching XIAO/RP2040")
    parser.add_argument("--raw", action="store_true", help="Also show raw input samples and legacy text")
    parser.add_argument("--stdin", action="store_true", help="Render JSONL from stdin instead of a device")
    args = parser.parse_args()
    try:
        if args.stdin:
            renderer, lines = Renderer(args.raw), Lines()
            while chunk := sys.stdin.buffer.read1(512):
                for line in lines.feed(chunk):
                    output(renderer.render(line))
        else:
            monitor(args)
    except KeyboardInterrupt:
        print("\nMonitor closed.", flush=True)
    except RuntimeError as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
