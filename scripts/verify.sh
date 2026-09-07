#!/usr/bin/env bash
set -Eeuo pipefail

ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
SERVICE=nemqo-samp.service
SERVER_DIR=/opt/nemqo-samp/server
HOST=127.0.0.1
PORT=7777

while (($#)); do
  case "$1" in
    --service) SERVICE=${2:?missing service}; shift 2 ;;
    --server-dir) SERVER_DIR=${2:?missing server dir}; shift 2 ;;
    --host) HOST=${2:?missing host}; shift 2 ;;
    --port) PORT=${2:?missing port}; shift 2 ;;
    -h|--help) echo "Usage: $0 [--service UNIT] [--server-dir DIR] [--host HOST] [--port PORT]"; exit 0 ;;
    *) echo "Unknown argument: $1" >&2; exit 2 ;;
  esac
done

systemctl is-active --quiet "$SERVICE"
[[ -s "$SERVER_DIR/gamemodes/nemqo_freeroam_dm.amx" ]]
[[ -s "$SERVER_DIR/npcmodes/ambient_walk.amx" ]]
[[ -f "$SERVER_DIR/scriptfiles/accounts.db" ]]

integrity=$(sqlite3 "$SERVER_DIR/scriptfiles/accounts.db" 'PRAGMA integrity_check;')
[[ "$integrity" == ok ]] || { echo "SQLite integrity failed: $integrity" >&2; exit 1; }
accounts=$(sqlite3 "$SERVER_DIR/scriptfiles/accounts.db" 'SELECT COUNT(*) FROM accounts;')

log="$SERVER_DIR/server_log.txt"
[[ -s "$log" ]] || { echo "Missing server log: $log" >&2; exit 1; }
if grep -Eqi 'FATAL:|Run time error|Unable to execute|Script.*not loaded' "$log"; then
  echo "Fatal marker found in server log" >&2
  exit 1
fi
for marker in \
  'NEMQO PSZ self-test: PASS' \
  'NEMQO Island self-test: PASS' \
  'V4 SAFETY SELFTEST' \
  'ambient NPC movement: PASS'; do
  grep -Fq "$marker" "$log" || { echo "Missing log marker: $marker" >&2; exit 1; }
done

if command -v ss >/dev/null 2>&1; then
  ss -lun | grep -Eq "[:.]${PORT}[[:space:]]" || { echo "UDP port $PORT is not listening" >&2; exit 1; }
fi
if [[ -f "$ROOT/scripts/samp_query.py" ]]; then
  python3 "$ROOT/scripts/samp_query.py" "$HOST" "$PORT"
fi

echo "VERIFY_OK"
echo "SERVICE=$SERVICE"
echo "SQLITE=ok"
echo "ACCOUNTS=$accounts"
