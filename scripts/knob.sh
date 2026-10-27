#!/bin/sh
# Build and explicit upload helpers. No command uploads implicitly.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
usage() {
  echo 'Usage: scripts/knob.sh build|upload [controller|diagnostic]' >&2
  echo '       scripts/knob.sh monitor [--port PORT] [--raw]' >&2
  echo '       scripts/knob.sh raw-monitor PORT' >&2
  exit 1
}
case "${1:-}" in
  build|upload)
    [ "$#" -le 2 ] || usage
    TARGET=${2:-controller}
    case "$TARGET" in controller|diagnostic) ;; *) usage ;; esac
    FQBN=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["fqbn"])')
    BUILD="$ROOT/build/$TARGET-picotool"
    if [ "$1" = build ]; then
      exec "$ROOT/scripts/arduino.sh" compile --fqbn "$FQBN" --warnings all \
        --build-path "$BUILD" "$ROOT/firmware/$TARGET"
    fi
    if [ ! -f "$BUILD/$TARGET.ino.uf2" ]; then
      echo "Run scripts/knob.sh build $TARGET first." >&2
      exit 1
    fi
    VERSION=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["picotool_package_version"])')
    PICOTOOL="$ROOT/.tools/arduino/data/packages/rp2040/tools/pqt-picotool/$VERSION/picotool"
    if [ ! -x "$PICOTOOL" ]; then
      echo 'Picotool missing; run python3 scripts/setup-toolchain.py.' >&2
      exit 1
    fi
    exec "$PICOTOOL" load "$BUILD/$TARGET.ino.uf2" -v -f -x
    ;;
  monitor)
    shift
    # Preserve the previous positional-port spelling too.
    case "${1:-}" in /dev/*) exec python3 "$ROOT/scripts/gesture-monitor.py" --port "$@" ;; esac
    exec python3 "$ROOT/scripts/gesture-monitor.py" "$@"
    ;;
  raw-monitor)
    [ "$#" -eq 2 ] || usage
    exec "$ROOT/scripts/arduino.sh" monitor --port "$2" --config baudrate=115200
    ;;
  *) usage ;;
esac
