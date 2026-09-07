#!/usr/bin/env bash
set -Eeuo pipefail
[[ $EUID -eq 0 ]] || { echo "Run with sudo/root." >&2; exit 1; }
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
  ca-certificates curl tar python3 sqlite3 \
  libc6-i386 lib32gcc-s1 lib32stdc++6
echo "DEPENDENCIES_OK"
