#!/bin/sh
# Project commands. Upload is always an explicit user action.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
FQBN=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["fqbn"])')
case "${1:-}" in
  build)
    exec "$ROOT/scripts/arduino.sh" compile --fqbn "$FQBN" --warnings all \
      --build-path "$ROOT/build/picotool" "$ROOT/firmware/diagnostic"
    ;;
  upload)
    if [ ! -f "$ROOT/build/picotool/diagnostic.ino.uf2" ]; then
      echo 'Run scripts/knob.sh build first.' >&2
      exit 1
    fi
    VERSION=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["picotool_package_version"])')
    PICOTOOL="$ROOT/.tools/arduino/data/packages/rp2040/tools/pqt-picotool/$VERSION/picotool"
    if [ ! -x "$PICOTOOL" ]; then
      echo 'Picotool missing; run python3 scripts/setup-toolchain.py.' >&2
      exit 1
    fi
    # Same USB upload path as the core's Picotool recipe, plus read-back verification.
    exec "$PICOTOOL" load "$ROOT/build/picotool/diagnostic.ino.uf2" -v -f -x
    ;;
  monitor)
    if [ "$#" -ne 2 ]; then
      echo 'Usage: scripts/knob.sh monitor /dev/cu.usbmodem...' >&2
      exit 1
    fi
    exec "$ROOT/scripts/arduino.sh" monitor --port "$2" --config baudrate=115200
    ;;
  *)
    echo 'Usage: scripts/knob.sh build | upload | monitor PORT' >&2
    exit 1
    ;;
esac
