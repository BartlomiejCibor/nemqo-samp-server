#!/usr/bin/env bash
set -Eeuo pipefail

ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
COMPILER_DIR=${PAWN_COMPILER_DIR:-}
SDK_INCLUDE=${SAMP_INCLUDE_DIR:-}

while (($#)); do
  case "$1" in
    --compiler-dir) COMPILER_DIR=${2:?missing compiler path}; shift 2 ;;
    --sdk-include) SDK_INCLUDE=${2:?missing SDK include path}; shift 2 ;;
    -h|--help)
      echo "Usage: $0 --compiler-dir DIR --sdk-include DIR"
      exit 0 ;;
    *) echo "Unknown argument: $1" >&2; exit 2 ;;
  esac
done

[[ -x "$COMPILER_DIR/pawncc" ]] || { echo "Missing executable: $COMPILER_DIR/pawncc" >&2; exit 1; }
[[ -f "$COMPILER_DIR/libpawnc.so" ]] || { echo "Missing: $COMPILER_DIR/libpawnc.so" >&2; exit 1; }
[[ -f "$SDK_INCLUDE/a_samp.inc" ]] || { echo "Missing official SDK include: $SDK_INCLUDE/a_samp.inc" >&2; exit 1; }
[[ -f "$SDK_INCLUDE/a_npc.inc" ]] || { echo "Missing official SDK include: $SDK_INCLUDE/a_npc.inc" >&2; exit 1; }

BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT
mkdir -p "$BUILD/include" "$ROOT/server-files/gamemodes" "$ROOT/server-files/npcmodes"
cp "$ROOT/gamemodes/nemqo_freeroam_dm.pwn" "$BUILD/"
cp "$ROOT/npcmodes/ambient_walk.pwn" "$BUILD/"
# Copy the external SDK first. Project-owned modules must win if an operator's
# SDK directory accidentally contains stale NEMQO includes from an older build.
cp "$SDK_INCLUDE"/*.inc "$BUILD/include/"
cp "$ROOT"/include/*.inc "$BUILD/include/"
cp "$ROOT/include/samp_empty_prefix.inc" "$BUILD/samp_empty_prefix.inc"

compile_one() {
  local source=$1 output=$2 log=$3
  (
    cd "$BUILD"
    LD_LIBRARY_PATH="$COMPILER_DIR" "$COMPILER_DIR/pawncc" "$source" \
      -i"$BUILD/include" -psamp_empty_prefix.inc -o"$output"
  ) >"$log" 2>&1 || { cat "$log"; exit 1; }
  cat "$log"
  if grep -Eqi '(^|[[:space:]])(warning|error)[[:space:]][0-9]+:' "$log"; then
    echo "Build rejected: compiler emitted a warning or error." >&2
    exit 1
  fi
  [[ -s "$output" ]] || { echo "Build produced no artifact: $output" >&2; exit 1; }
}

compile_one nemqo_freeroam_dm.pwn "$BUILD/nemqo_freeroam_dm.amx" "$BUILD/gamemode.log"
compile_one ambient_walk.pwn "$BUILD/ambient_walk.amx" "$BUILD/npc.log"
install -m 0644 "$BUILD/nemqo_freeroam_dm.amx" "$ROOT/server-files/gamemodes/nemqo_freeroam_dm.amx"
install -m 0644 "$BUILD/ambient_walk.amx" "$ROOT/server-files/npcmodes/ambient_walk.amx"

sha256sum "$ROOT/server-files/gamemodes/nemqo_freeroam_dm.amx" \
          "$ROOT/server-files/npcmodes/ambient_walk.amx"
echo "BUILD_OK"
