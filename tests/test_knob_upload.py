"""Exercise the shell wrapper with a fake CLI; never access hardware."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(sys.platform == "darwin", "macOS wrapper integration")
class StandardUploadTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='knob upload ')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        scripts = self.root / 'scripts'; scripts.mkdir()
        shutil.copy(ROOT / 'scripts/knob.sh', scripts / 'knob.sh')
        (self.root / 'firmware').mkdir()
        shutil.copy(ROOT / 'firmware/toolchain.json', self.root / 'firmware/toolchain.json')
        for target in ('controller', 'diagnostic', 'interface'):
            (self.root / 'firmware' / target).mkdir()
            folder = self.root / 'build' / (target + '-picotool'); folder.mkdir(parents=True)
            (folder / (target + '.ino.uf2')).write_bytes(b'fake')
        tool = self.root / 'fake tool'; tool.mkdir()
        (tool / 'picotool').write_text('#!/bin/sh\nexit 99\n'); (tool / 'picotool').chmod(0o755)
        # Host tool verification is independently tested; fake only its path lookup.
        (scripts / 'picotool-host.py').write_text('from pathlib import Path\nprint(Path(__file__).resolve().parents[1]/"fake tool/picotool")\n')
        (scripts / 'memory-usage.py').write_text('print("Memory report")\n')
        (scripts / 'arduino.sh').write_text('#!/bin/sh\nexec python3 "$(dirname "$0")/fake_cli.py" "$@"\n')
        (scripts / 'arduino.sh').chmod(0o755)
        (scripts / 'fake_cli.py').write_text('''import json, os, sys
from pathlib import Path
root = Path(__file__).resolve().parents[1]
if sys.argv[1:3] == ['board', 'list']:
    print((root / 'ports.json').read_text())
else:
    (root / 'cli-args.json').write_text(json.dumps(sys.argv[1:]))
    sys.exit(int(os.environ.get('FAKE_CLI_EXIT', '0')))
''')
        self.ports(['/dev/cu.usbmodemTEST'])

    def ports(self, addresses):
        records = [dict(port=dict(address=a, protocol='serial'), matching_boards=[
            dict(fqbn='rp2040:rp2040:seeed_xiao_rp2040')]) for a in addresses]
        (self.root / 'ports.json').write_text(json.dumps(dict(detected_ports=records)))

    def run_wrapper(self, *args, **env):
        return subprocess.run(['sh', str(self.root / 'scripts/knob.sh'), *args],
                              capture_output=True, text=True, timeout=5,
                              env=dict(os.environ, **env))

    def arguments(self):
        return json.loads((self.root / 'cli-args.json').read_text())

    def test_standard_recipe_and_discovered_port(self):
        result = self.run_wrapper('upload', 'controller')
        self.assertEqual(result.returncode, 0, result.stderr)
        args = self.arguments()
        self.assertEqual(args[0], 'upload')
        self.assertEqual(args[args.index('--fqbn')+1], 'rp2040:rp2040:seeed_xiao_rp2040:uploadmethod=picotool')
        self.assertEqual(args[args.index('--input-dir')+1], str(self.root / 'build/controller-picotool'))
        self.assertEqual(args[args.index('--port')+1], '/dev/cu.usbmodemTEST')
        self.assertIn('--verbose', args)
        self.assertEqual(args[args.index('--upload-property')+1], 'tools.picotool.cmd='+str((self.root/'fake tool').resolve()))
        self.assertIn('Memory report', result.stdout)

    def test_explicit_port_and_default_target(self):
        self.ports(['/dev/a', '/dev/b'])
        result = self.run_wrapper('upload', '--port', '/dev/chosen')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.arguments()[-3:], ['--port', '/dev/chosen', str(self.root/'firmware/controller')])

    def test_multiple_ports_require_selection(self):
        self.ports(['/dev/a', '/dev/b'])
        result = self.run_wrapper('upload')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Multiple matching', result.stderr)
        self.assertFalse((self.root/'cli-args.json').exists())

    def test_no_serial_defers_usb_discovery_to_core(self):
        self.ports([])
        result = self.run_wrapper('upload')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn('--port', self.arguments())

    def test_cli_failure_is_returned_without_retry(self):
        result = self.run_wrapper('upload', FAKE_CLI_EXIT='7')
        self.assertEqual(result.returncode, 7)

    def test_other_sketches_use_matching_artifact(self):
        for target in ('diagnostic', 'interface'):
            result = self.run_wrapper('upload', target)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(self.arguments()[-1], str(self.root/'firmware'/target))
            self.assertIn(str(self.root/'build'/(target+'-picotool')), self.arguments())

    def test_bad_arguments_do_not_invoke_cli(self):
        for args in [('build', '--port', '/dev/fake'), ('upload', 'bogus'), ('upload', '--port')]:
            self.assertNotEqual(self.run_wrapper(*args).returncode, 0)
        self.assertFalse((self.root/'cli-args.json').exists())
