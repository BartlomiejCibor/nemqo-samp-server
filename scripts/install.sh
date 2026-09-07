#!/usr/bin/env bash
set -Eeuo pipefail

ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
RUNTIME=
INSTALL_DIR=/opt/nemqo-samp
SERVICE_NAME=nemqo-samp
BIND_IP=
PORT=7777
FORCE=0

usage() {
  cat <<'USAGE'
Usage: sudo ./scripts/install.sh --runtime DIR [options]
  --runtime DIR       extracted official SA-MP 0.3.7-R2 Linux directory
  --install-dir DIR   installation prefix (default: /opt/nemqo-samp)
  --service-name NAME systemd unit basename (default: nemqo-samp)
  --bind-ip ADDRESS   optional local address for server.cfg
  --port PORT         UDP port (default: 7777)
  --force             replace an existing installation after making a backup
USAGE
}

while (($#)); do
  case "$1" in
    --runtime) RUNTIME=${2:?missing runtime path}; shift 2 ;;
    --install-dir) INSTALL_DIR=${2:?missing install path}; shift 2 ;;
    --service-name) SERVICE_NAME=${2:?missing service name}; shift 2 ;;
    --bind-ip) BIND_IP=${2:?missing bind address}; shift 2 ;;
    --port) PORT=${2:?missing port}; shift 2 ;;
    --force) FORCE=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown argument: $1" >&2; usage; exit 2 ;;
  esac
done

[[ $EUID -eq 0 ]] || { echo "Run this installer with sudo/root." >&2; exit 1; }
[[ -n "$RUNTIME" ]] || { echo "--runtime is required" >&2; exit 2; }
[[ "$SERVICE_NAME" =~ ^[A-Za-z0-9_.@-]+$ ]] || { echo "Invalid service name" >&2; exit 2; }
[[ "$PORT" =~ ^[0-9]+$ ]] && ((PORT >= 1 && PORT <= 65535)) || { echo "Invalid UDP port" >&2; exit 2; }
for file in samp03svr samp-npc announce; do
  [[ -f "$RUNTIME/$file" ]] || { echo "Missing official runtime file: $RUNTIME/$file" >&2; exit 1; }
done
for file in \
  "$ROOT/server-files/gamemodes/nemqo_freeroam_dm.amx" \
  "$ROOT/server-files/npcmodes/ambient_walk.amx" \
  "$ROOT/database/accounts.clean.db" \
  "$ROOT/config/server.cfg.example" \
  "$ROOT/config/samp.service.example"; do
  [[ -s "$file" ]] || { echo "Missing package artifact: $file" >&2; exit 1; }
done

SERVER_DIR="$INSTALL_DIR/server"
UNIT="/etc/systemd/system/$SERVICE_NAME.service"
if [[ -e "$SERVER_DIR" ]]; then
  [[ $FORCE -eq 1 ]] || { echo "$SERVER_DIR exists; use --force after reviewing the target." >&2; exit 1; }
  stamp=$(date +%Y%m%d-%H%M%S)
  systemctl stop "$SERVICE_NAME.service" 2>/dev/null || true
  tar -C "$INSTALL_DIR" -czf "$INSTALL_DIR/server-before-$stamp.tar.gz" server
  rm -rf "$SERVER_DIR"
fi

getent group samp >/dev/null || groupadd --system samp
id samp >/dev/null 2>&1 || useradd --system --gid samp --home-dir "$INSTALL_DIR" --shell /usr/sbin/nologin samp
install -d -o samp -g samp -m 0750 "$SERVER_DIR"
cp -a "$RUNTIME"/. "$SERVER_DIR"/
install -d -o samp -g samp -m 0750 "$SERVER_DIR/gamemodes" "$SERVER_DIR/npcmodes" "$SERVER_DIR/scriptfiles"
install -m 0644 "$ROOT/server-files/gamemodes/nemqo_freeroam_dm.amx" "$SERVER_DIR/gamemodes/"
install -m 0644 "$ROOT/server-files/npcmodes/ambient_walk.amx" "$SERVER_DIR/npcmodes/"
install -m 0640 "$ROOT/database/accounts.clean.db" "$SERVER_DIR/scriptfiles/accounts.db"

if command -v openssl >/dev/null 2>&1; then
  RCON=$(openssl rand -hex 24)
else
  RCON=$(python3 -c 'import secrets; print(secrets.token_hex(24))')
fi
python3 - "$ROOT/config/server.cfg.example" "$SERVER_DIR/server.cfg" "$RCON" "$BIND_IP" "$PORT" <<'PY'
import sys
from pathlib import Path
src,dst,password,bind_ip,port=sys.argv[1:]
text=Path(src).read_text()
text=text.replace("CHANGE_ME_TO_A_LONG_RANDOM_PASSWORD",password)
text=text.replace("port 7777",f"port {port}")
if bind_ip:
    text=f"bind {bind_ip}\n"+text
Path(dst).write_text(text)
PY

python3 - "$ROOT/config/samp.service.example" "$UNIT" "$SERVER_DIR" <<'PY'
import sys
from pathlib import Path
src,dst,server_dir=sys.argv[1:]
text=Path(src).read_text().replace("/opt/nemqo-samp/server",server_dir)
Path(dst).write_text(text)
PY

chown -R samp:samp "$SERVER_DIR"
chmod 0750 "$SERVER_DIR" "$SERVER_DIR/samp03svr" "$SERVER_DIR/samp-npc" "$SERVER_DIR/announce"
chmod 0640 "$SERVER_DIR/server.cfg" "$SERVER_DIR/scriptfiles/accounts.db"
systemctl daemon-reload
systemctl enable --now "$SERVICE_NAME.service"
sleep 2
systemctl is-active --quiet "$SERVICE_NAME.service"

echo "INSTALL_OK"
echo "SERVICE=$SERVICE_NAME.service"
echo "SERVER_DIR=$SERVER_DIR"
echo "PORT=$PORT/udp"
echo "RCON password was generated locally and stored only in $SERVER_DIR/server.cfg"
