import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('picotool_host', ROOT / 'scripts/picotool-host.py')
host = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(host)


class HostToolTests(unittest.TestCase):
    def test_bad_checksum_does_not_install(self):
        with tempfile.TemporaryDirectory() as folder:
            archive = Path(folder) / 'bad.zip'
            archive.write_bytes(b'not a trusted download')
            dest = Path(folder) / 'installed'
            with self.assertRaisesRegex(RuntimeError, 'checksum'):
                host.unpack(archive, dest)
            self.assertFalse(dest.exists())

    def test_extracts_only_pinned_binary_and_library(self):
        with tempfile.TemporaryDirectory() as folder:
            archive = Path(folder) / 'test.zip'
            with zipfile.ZipFile(archive, 'w') as z:
                z.writestr('picotool/picotool', b'binary')
                z.writestr('picotool/libusb-1.0.0.dylib', b'library')
                z.writestr('../unwanted', b'never extracted')
            lock = dict(host.LOCK, sha256=host.hashlib.sha256(archive.read_bytes()).hexdigest())
            dest = Path(folder) / 'installed'
            with patch.object(host, 'LOCK', lock): host.unpack(archive, dest)
            self.assertEqual(sorted(p.name for p in dest.iterdir()), ['libusb-1.0.0.dylib', 'picotool'])
            self.assertFalse((Path(folder) / 'unwanted').exists())
            self.assertTrue((dest / 'picotool').stat().st_mode & 0o111)

    @patch.object(host, 'usb_version', return_value=(1, 0, 27))
    @patch.object(host.subprocess, 'run')
    def test_rejects_pre_tahoe_library(self, run, version):
        with self.assertRaisesRegex(RuntimeError, 'Tahoe'):
            host.verify(Path('/unused'))
        run.assert_not_called()

    @patch.object(host, 'usb_version', return_value=(1, 0, 30))
    @patch.object(host.subprocess, 'run')
    def test_verification_only_runs_version_command(self, run, version):
        run.return_value = subprocess.CompletedProcess([], 0, 'picotool v2.3.1 (Darwin)\n', '')
        self.assertEqual(host.verify(Path('/unused')), Path('/unused/picotool'))
        self.assertEqual(run.call_args.args[0], ['/unused/picotool', 'version'])
        run.return_value.stdout = 'picotool v2.0.0 (Darwin)\n'
        with self.assertRaisesRegex(RuntimeError, 'Unexpected'): host.verify(Path('/unused'))
