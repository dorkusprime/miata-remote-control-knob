import importlib.util
import json
import os
import pathlib
import pty
import select
import subprocess
import sys
import time
import types
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("gesture_monitor", ROOT / "scripts/gesture-monitor.py")
monitor = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(monitor)


def gesture(action="volume_up", steps=1, **changes):
    return json.dumps(dict(v=1, type="gesture", action=action, steps=steps, simulated=True, **changes))


class MonitorTests(unittest.TestCase):
    def test_all_actions(self):
        for action in monitor.LABELS:
            with self.subTest(action=action):
                self.assertIn(monitor.LABELS[action], monitor.Renderer().render(gesture(action)))

    def test_detent_count(self):
        self.assertEqual(monitor.Renderer().render(gesture(steps=4)), "[SIM] Volume up × 4")

    def test_reject_bad_steps(self):
        for value in (True, 0, -1, "2", 999999):
            self.assertEqual(monitor.Renderer().render(gesture(steps=value)), "Invalid gesture step count.")

    def test_missing_simulation_marker(self):
        data = json.loads(gesture()); del data["simulated"]
        self.assertIn("ignored", monitor.Renderer().render(json.dumps(data)))

    def test_hardware_requests_and_rejections(self):
        data = json.loads(gesture())
        data.update(simulated=False, accepted=True)
        self.assertEqual(monitor.Renderer().render(json.dumps(data)), "[REQUEST] Volume up × 1")
        data["accepted"] = False
        self.assertEqual(monitor.Renderer().render(json.dumps(data)), "[DROPPED] Volume up × 1")
        del data["accepted"]
        self.assertIn("ignored", monitor.Renderer().render(json.dumps(data)))

    def test_hardware_heartbeat(self):
        data = dict(v=1, type="heartbeat", connected=True, camera_preview=False,
                    simulated=False, outputs_enabled=True)
        renderer = monitor.Renderer()
        self.assertIn("hardware outputs enabled", renderer.render(json.dumps(data)))
        data["outputs_enabled"] = False
        self.assertIn("hardware outputs disabled", renderer.render(json.dumps(data)))
        data["learning_mode"] = True
        self.assertIn("LEARNING MODE", renderer.render(json.dumps(data)))

    def test_calibration_table_and_request_code(self):
        codes = dict(volume_up=255, volume_down=0, mute_att=238, next_track=248, previous_track=215)
        line = json.dumps(dict(v=1, type="calibration", codes=codes))
        self.assertIn("mute_att=238", monitor.Renderer().render(line))
        codes["mute_att"] = 256
        self.assertEqual(monitor.Renderer().render(json.dumps(dict(v=1, type="calibration", codes=codes))),
                         "Invalid calibration message.")
        data = json.loads(gesture())
        data.update(simulated=False, accepted=True, pot_code=255)
        self.assertIn("(code 255)", monitor.Renderer().render(json.dumps(data)))

    def test_unknown_action(self):
        data = json.loads(gesture()); data["action"] = ["bad"]
        self.assertEqual(monitor.Renderer().render(json.dumps(data)), "Unknown gesture action.")

    def test_bad_json_does_not_crash(self):
        renderer = monitor.Renderer()
        self.assertIn("Non-JSON", renderer.render("ALIVE; Twist=connected"))
        self.assertIsNone(renderer.render("not JSON"))
        self.assertIn("Volume up", renderer.render(gesture()))

    def test_wrong_protocol(self):
        for line in ("null", "[]", '{"v":true}', '{"v":2}'):
            self.assertEqual(monitor.Renderer().render(line), "Unsupported device message.")

    def test_heartbeat_deduplication_and_recovery(self):
        renderer = monitor.Renderer()
        data = dict(v=1, type="heartbeat", connected=True, camera_preview=False)
        self.assertIn("Twist connected", renderer.render(json.dumps(data)))
        self.assertIsNone(renderer.render(json.dumps(data)))
        data["connected"] = False
        self.assertIn("Twist missing", renderer.render(json.dumps(data)))
        data["connected"] = True
        self.assertIn("Twist connected", renderer.render(json.dumps(data)))

    def test_raw_samples_opt_in(self):
        line = json.dumps(dict(v=1, type="input", count=3, pressed=False))
        self.assertIsNone(monitor.Renderer().render(line))
        self.assertIn("[input]", monitor.Renderer(raw=True).render(line))

    def test_terminal_control_characters_removed(self):
        text = monitor.Renderer().render(json.dumps(dict(v=1, type="status", message="Hi\x1b[31m\nthere")))
        self.assertNotIn("\x1b", text)
        self.assertNotIn("\n", text)

    def test_chunked_framing(self):
        lines = monitor.Lines()
        self.assertEqual(list(lines.feed(b'{"v":')), [])
        self.assertEqual(list(lines.feed(b'1}\r\nnext\n')), ['{"v":1}', "next"])

    def test_oversize_frame_recovery(self):
        lines = monitor.Lines()
        self.assertEqual(list(lines.feed(b"x" * (monitor.MAX_LINE + 1))), [])
        self.assertEqual(list(lines.feed(b"still oversized\nok\n")), ["ok"])

    def test_port_selection(self):
        item = lambda name: types.SimpleNamespace(device=name, vid=0x2E8A, pid=0x010A)
        ports = types.SimpleNamespace(comports=lambda: [item("/dev/fake")])
        self.assertEqual(monitor.choose_port(None, ports), "/dev/fake")
        ports.comports = lambda: [item("/dev/a"), item("/dev/b")]
        with self.assertRaisesRegex(RuntimeError, "Multiple"):
            monitor.choose_port(None, ports)
        self.assertEqual(monitor.choose_port("/dev/explicit", ports), "/dev/explicit")

    def test_stdin_cli(self):
        process = subprocess.run([sys.executable, str(ROOT / "scripts/gesture-monitor.py"), "--stdin"],
                                 input=gesture("previous_track", 2) + "\n", text=True,
                                 capture_output=True, timeout=3)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("[SIM] Previous track × 2", process.stdout)

    def test_interactive_console_with_virtual_port(self):
        try:
            monitor.serial_modules()
        except RuntimeError:
            self.skipTest("Project toolchain not installed")
        master, slave = pty.openpty()
        process = subprocess.Popen([sys.executable, str(ROOT / "scripts/gesture-monitor.py"),
                                    "--interactive", "--port", os.ttyname(slave)],
                                   stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            self.assertTrue(select.select([master], [], [], 3)[0], "No help request")
            self.assertIn(b"h\n", os.read(master, 1024))
            process.stdin.write(b"set mute_att 230\n"); process.stdin.flush()
            self.assertTrue(select.select([master], [], [], 3)[0], "No calibration command")
            self.assertEqual(os.read(master, 1024), b"set mute_att 230\n")
            codes = dict(volume_up=255, volume_down=0, mute_att=230, next_track=248, previous_track=215)
            os.write(master, (json.dumps(dict(v=1, type="calibration", codes=codes)) + "\n").encode())
            collected = b""
            deadline = time.monotonic() + 3
            while b"mute_att=230" not in collected and time.monotonic() < deadline:
                if select.select([process.stdout], [], [], 0.1)[0]:
                    collected += os.read(process.stdout.fileno(), 4096)
            self.assertIn(b"mute_att=230", collected)
        finally:
            process.terminate(); process.communicate(timeout=3)
            os.close(master); os.close(slave)

    def test_serial_monitor_with_virtual_port(self):
        # Exercise actual pyserial and the process without opening the physical XIAO.
        try:
            monitor.serial_modules()
        except RuntimeError:
            self.skipTest("Project toolchain not installed")
        master, slave = pty.openpty()
        process = subprocess.Popen([sys.executable, str(ROOT / "scripts/gesture-monitor.py"),
                                    "--port", os.ttyname(slave)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            self.assertTrue(select.select([master], [], [], 3)[0], "Monitor did not send help request")
            self.assertIn(b"h", os.read(master, 1024))
            os.write(master, (gesture("next_track", 2) + "\n").encode())
            collected = b""
            deadline = time.monotonic() + 3
            while b"Next track" not in collected and time.monotonic() < deadline:
                if select.select([process.stdout], [], [], 0.1)[0]:
                    collected += os.read(process.stdout.fileno(), 4096)
            self.assertIn("[SIM] Next track × 2", collected.decode())
        finally:
            process.terminate()
            process.communicate(timeout=3)
            os.close(master); os.close(slave)


if __name__ == "__main__":
    unittest.main()
