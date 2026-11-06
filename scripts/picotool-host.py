#!/usr/bin/env python3
"""Install/select the pinned macOS USB uploader. Never reset or flash a device."""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / '.tools/picotool-host'
LOCK = json.loads((ROOT / 'firmware/toolchain.json').read_text())['macos_picotool']


class UsbVersion(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint16) for name in ('major', 'minor', 'micro', 'nano')]


def usb_version(library):
    lib = ctypes.CDLL(str(library))
    lib.libusb_get_version.restype = ctypes.POINTER(UsbVersion)
    value = lib.libusb_get_version().contents
    return value.major, value.minor, value.micro


def unpack(archive, dest):
    if hashlib.sha256(archive.read_bytes()).hexdigest() != LOCK['sha256']:
        raise RuntimeError('Picotool download checksum mismatch; nothing installed')
    # Extract only these exact members; do not trust archive paths or permissions.
    with zipfile.ZipFile(archive) as source:
        files = {name: source.read('picotool/' + name)
                 for name in ('picotool', 'libusb-1.0.0.dylib')}
    dest.mkdir(parents=True, exist_ok=True)
    for name, data in files.items():
        (dest / name).write_bytes(data)
    (dest / 'picotool').chmod(0o755)


def verify(dest):
    library = dest / 'libusb-1.0.0.dylib'
    version = usb_version(library)
    if version < (1, 0, 30):
        raise RuntimeError('macOS uploader requires libusb >=1.0.30 (Tahoe fixes)')
    result = subprocess.run([str(dest / 'picotool'), 'version'], capture_output=True,
                            text=True, timeout=5, check=True)
    if not result.stdout.startswith('picotool v' + LOCK['version'] + ' '):
        raise RuntimeError('Unexpected Picotool version: ' + result.stdout.strip())
    print('USB uploader: Picotool ' + LOCK['version'] + ' / libusb ' +
          '.'.join(map(str, version)), file=sys.stderr)
    return dest / 'picotool'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('setup', 'path'))
    args = parser.parse_args()
    if platform.system() != 'Darwin':
        parser.error('This host-tool pin is for macOS')
    if args.command == 'setup':
        archive = ROOT / '.tools/downloads/picotool-2.3.1-mac.zip'
        archive.parent.mkdir(parents=True, exist_ok=True)
        if not archive.exists() or hashlib.sha256(archive.read_bytes()).hexdigest() != LOCK['sha256']:
            partial = archive.with_suffix('.partial')
            subprocess.run(['curl', '-fsSL', '--retry', '2', '--max-time', '60',
                            LOCK['url'], '-o', str(partial)], check=True)
            partial.replace(archive)
        unpack(archive, DEST)
    if not (DEST / 'picotool').exists():
        raise RuntimeError('Run python3 scripts/picotool-host.py setup once to install the macOS USB fixes')
    binary = verify(DEST)
    if args.command == 'path':
        print(binary)
    else:
        print('Installed project-local uploader; no hardware reset or flashing occurred.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, subprocess.SubprocessError, zipfile.BadZipFile) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
