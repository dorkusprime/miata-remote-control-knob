#!/bin/sh
# Build and explicit upload helpers. No command uploads implicitly.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
usage() {
  echo 'Usage: scripts/knob.sh build|size [controller|diagnostic|interface]' >&2
  echo '       scripts/knob.sh upload [controller|diagnostic|interface] [--port PORT] [--dry-run]' >&2
  echo '       scripts/knob.sh monitor [--port PORT] [--raw]' >&2
  echo '       scripts/knob.sh console [--port PORT]' >&2
  echo '       scripts/knob.sh raw-monitor PORT' >&2
  exit 1
}
case "${1:-}" in
  build|upload|size)
    ACTION=$1
    shift
    TARGET=controller
    case "${1:-}" in
      --port|'') ;;
      *) TARGET=$1; shift ;;
    esac
    PORT=
    DRY_RUN=false
    while [ "$#" -gt 0 ]; do
      [ "$ACTION" = upload ] || usage
      case "$1" in
        --port)
          [ "$#" -ge 2 ] && [ -n "$2" ] || usage
          PORT=$2; shift 2 ;;
        --dry-run) DRY_RUN=true; shift ;;
        *) usage ;;
      esac
    done
    case "$TARGET" in controller|diagnostic|interface) ;; *) usage ;; esac
    FQBN=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["fqbn"])')
    BUILD="$ROOT/build/$TARGET-picotool"
    if [ "$ACTION" = build ]; then
      exec "$ROOT/scripts/arduino.sh" compile --fqbn "$FQBN" --warnings all \
        --build-path "$BUILD" "$ROOT/firmware/$TARGET"
    fi
    if [ "$ACTION" = size ]; then
      exec python3 "$ROOT/scripts/memory-usage.py" "$BUILD/$TARGET.ino.elf"
    fi
    if [ ! -f "$BUILD/$TARGET.ino.uf2" ]; then
      echo "Run scripts/knob.sh build $TARGET first." >&2
      exit 1
    fi
    VERSION=$(python3 -c 'import json; print(json.load(open("firmware/toolchain.json"))["picotool_package_version"])')
    PICOTOOL="$ROOT/.tools/arduino/data/packages/rp2040/tools/pqt-picotool/$VERSION/picotool"
    # The Arduino package bundles libusb 1.0.27, predating macOS Tahoe fixes.
    if [ "$(uname -s)" = Darwin ]; then
      PICOTOOL=$(python3 "$ROOT/scripts/picotool-host.py" path)
    fi
    if [ ! -x "$PICOTOOL" ]; then
      echo 'Picotool missing; run python3 scripts/setup-toolchain.py.' >&2
      exit 1
    fi
    python3 "$ROOT/scripts/memory-usage.py" "$BUILD/$TARGET.ino.elf"
    if [ -z "$PORT" ]; then
      # Discovery only: Arduino CLI lists ports; no application serial commands.
      PORT=$("$ROOT/scripts/arduino.sh" board list --json | python3 -c '
import json, sys
board = ":".join(sys.argv[1].split(":")[:3])
ports = sorted({item["port"]["address"] for item in json.load(sys.stdin).get("detected_ports", [])
                if item.get("port", {}).get("protocol") == "serial"
                and any(match.get("fqbn") == board for match in item.get("matching_boards", []))})
if len(ports) > 1:
    sys.exit("Multiple matching serial ports; select one with --port PORT.")
if ports:
    print(ports[0])
' "$FQBN")
    fi
    # Delegate reset, mode transition and upload to the core-provided recipe.
    # CLI upload overrides use the flattened tool key (cmd), not tools.picotool.cmd.
    # This changes only the executable directory, not the recipe.
    set -- upload --fqbn "$FQBN" --input-dir "$BUILD" --verbose \
      --upload-property "cmd=$(dirname "$PICOTOOL")"
    if [ "$DRY_RUN" = true ]; then
      set -- "$@" --dry-run
    fi
    if [ -n "$PORT" ]; then
      echo "Arduino CLI upload via $PORT (core Picotool recipe)"
      set -- "$@" --port "$PORT"
    else
      echo 'Arduino CLI upload (no matching serial port; core Picotool handles USB discovery)'
    fi
    exec "$ROOT/scripts/arduino.sh" "$@" "$ROOT/firmware/$TARGET"
    ;;
  monitor)
    shift
    # Preserve the previous positional-port spelling too.
    case "${1:-}" in /dev/*) exec python3 "$ROOT/scripts/gesture-monitor.py" --port "$@" ;; esac
    exec python3 "$ROOT/scripts/gesture-monitor.py" "$@"
    ;;
  console)
    shift
    exec python3 "$ROOT/scripts/gesture-monitor.py" --interactive "$@"
    ;;
  raw-monitor)
    [ "$#" -eq 2 ] || usage
    exec "$ROOT/scripts/arduino.sh" monitor --port "$2" --config baudrate=115200
    ;;
  *) usage ;;
esac
