import importlib.util
from pathlib import Path
import subprocess
import unittest
from unittest.mock import MagicMock, patch

spec = importlib.util.spec_from_file_location("uploader", Path(__file__).resolve().parents[1] / "scripts/upload-interface.py")
uploader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(uploader)


class UploadTests(unittest.TestCase):
    @patch.object(uploader.subprocess, "run")
    def test_bootsel_without_serial(self, run):
        serial, ports = MagicMock(), MagicMock()
        ports.comports.return_value = []
        run.return_value = subprocess.CompletedProcess([], 0, "loaded", "")
        self.assertEqual(uploader.upload("tool", "file.uf2", serial, ports), 0)
        serial.Serial.assert_not_called()
        self.assertNotIn("-f", run.call_args.args[0])

    @patch.object(uploader.time, "sleep")
    @patch.object(uploader.subprocess, "run")
    def test_serial_reboot_and_retry(self, run, sleep):
        serial, ports = MagicMock(), MagicMock()
        ports.comports.return_value = [MagicMock(device="/dev/cu.usbmodem1", vid=0x2E8A, pid=0x000A)]
        run.side_effect = [subprocess.CompletedProcess([], 1, "", "waiting"),
                           subprocess.CompletedProcess([], 0, "rebooted", ""),
                           subprocess.CompletedProcess([], 0, "loaded", "")]
        self.assertEqual(uploader.upload("tool", "file.uf2", serial, ports), 0)
        connection = serial.Serial.return_value.__enter__.return_value
        self.assertEqual(connection.baudrate, 1200)
        self.assertFalse(connection.dtr)
        self.assertEqual(run.call_count, 3)
        self.assertEqual(run.call_args_list[1].args[0], ["tool", "reboot", "-u", "-f"])

    @patch.object(uploader.subprocess, "run")
    def test_ambiguous_devices_do_not_reset(self, run):
        serial, ports = MagicMock(), MagicMock()
        ports.comports.return_value = [
            MagicMock(device=f"/dev/port{i}", vid=0x2E8A, pid=0x000A) for i in (1, 2)]
        with self.assertRaises(RuntimeError):
            uploader.upload("tool", "file.uf2", serial, ports)
        serial.Serial.assert_not_called()
        run.assert_not_called()

    @patch.object(uploader.subprocess, "run")
    def test_no_usb_fails_without_reset(self, run):
        serial, ports = MagicMock(), MagicMock()
        ports.comports.return_value = []
        run.return_value = subprocess.CompletedProcess([], 1, "", "missing")
        self.assertEqual(uploader.upload("tool", "file.uf2", serial, ports), 1)
        serial.Serial.assert_not_called()
