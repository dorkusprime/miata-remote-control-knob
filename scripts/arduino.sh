#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CLI="$ROOT/.tools/bin/arduino-cli"
CONFIG="$ROOT/.tools/arduino/config.yaml"
if [ ! -x "$CLI" ] || [ ! -f "$CONFIG" ]; then
  echo 'Project toolchain missing; see firmware/README.md.' >&2
  exit 1
fi
exec "$CLI" --config-file "$CONFIG" "$@"
