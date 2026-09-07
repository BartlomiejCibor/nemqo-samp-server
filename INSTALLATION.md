# NEMQO SA-MP Server Installation Guide

This guide installs the NEMQO gamemode on a clean Debian 12/Ubuntu-compatible Linux server. It also explains how to build from source, create a fresh database, configure systemd and verify the actual SA-MP UDP protocol.

## 1. Requirements

### Hardware

- 1 x86-64 CPU core
- 512 MB RAM minimum; 1 GB recommended
- 2 GB free disk space
- UDP port 7777 reachable by players

### Software

- Debian 12 Bookworm or a compatible x86-64 Linux distribution
- official SA-MP 0.3.7-R2 Linux server package
- Pawn compiler compatible with `3.2.3664.samp` when building from source
- 32-bit runtime libraries
- SQLite 3 and Python 3 for database creation/verification

This gamemode requires no MySQL server, Docker, Wine, third-party plugins or filterscripts.

## 2. Install operating-system packages

Automated Debian installation:

```bash
sudo ./scripts/install-dependencies-debian.sh
```

Equivalent manual command:

```bash
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  ca-certificates curl tar python3 sqlite3 \
  libc6-i386 lib32gcc-s1 lib32stdc++6
```

Optional but recommended for a virtual machine:

```bash
sudo apt-get install -y qemu-guest-agent
sudo systemctl enable --now qemu-guest-agent
```

Verify that 32-bit programs can run:

```bash
file /path/to/samp03svr
ldd /path/to/samp03svr
```

The server should be reported as an `ELF 32-bit` executable and `ldd` must not show `not found`.

## 3. Obtain the official SA-MP runtime

Obtain a legitimate Linux SA-MP **0.3.7-R2** server package. Extract it outside this repository. The extracted directory must contain at least:

```text
samp03svr
samp-npc
announce
include/a_samp.inc
include/a_npc.inc
gamemodes/
npcmodes/
scriptfiles/
```

These third-party files are intentionally not committed to this repository.

## 4. Fast installation using the prebuilt overlay

The repository contains a `server-files/` overlay with:

```text
gamemodes/nemqo_freeroam_dm.amx
npcmodes/ambient_walk.amx
scriptfiles/accounts.db
server.cfg.example
```

After the release artifacts have been built, run:

```bash
sudo ./scripts/install.sh \
  --runtime /path/to/extracted/samp03 \
  --install-dir /opt/nemqo-samp \
  --service-name nemqo-samp
```

The installer:

1. validates the official runtime;
2. creates an unprivileged `samp` system account if needed;
3. copies the official runtime to the target directory;
4. applies the NEMQO AMX/NPC/database overlay;
5. generates a new random RCON password locally;
6. installs a hardened systemd service;
7. enables and starts the service;
8. prints the installed path and service name.

To bind to the original private LAN address, add:

```bash
sudo ./scripts/install.sh \
  --runtime /path/to/extracted/samp03 \
  --install-dir /opt/nemqo-samp \
  --service-name nemqo-samp \
  --bind-ip 10.0.30.123
```

Do not use that bind address unless it is assigned to the target server. Without `--bind-ip`, SA-MP listens on all available addresses, which is more portable.

## 5. Manual installation

### 5.1 Create a service account

```bash
sudo useradd --system --home-dir /opt/nemqo-samp --shell /usr/sbin/nologin samp 2>/dev/null || true
sudo install -d -o samp -g samp -m 0750 /opt/nemqo-samp/server
```

### 5.2 Copy the official runtime

```bash
sudo cp -a /path/to/extracted/samp03/. /opt/nemqo-samp/server/
```

### 5.3 Apply the NEMQO overlay

```bash
sudo cp -a server-files/gamemodes/. /opt/nemqo-samp/server/gamemodes/
sudo cp -a server-files/npcmodes/. /opt/nemqo-samp/server/npcmodes/
sudo cp -a server-files/scriptfiles/. /opt/nemqo-samp/server/scriptfiles/
sudo cp config/server.cfg.example /opt/nemqo-samp/server/server.cfg
```

Edit the RCON placeholder before starting:

```bash
sudo editor /opt/nemqo-samp/server/server.cfg
```

Replace:

```text
rcon_password CHANGE_ME_TO_A_LONG_RANDOM_PASSWORD
```

Generate a strong value locally if OpenSSL is installed:

```bash
openssl rand -hex 24
```

The server configuration does not require the original `10.0.30.123` address. To bind explicitly, add:

```text
bind 10.0.30.123
```

### 5.4 Set permissions

```bash
sudo chown -R samp:samp /opt/nemqo-samp/server
sudo chmod 0750 /opt/nemqo-samp/server
sudo chmod 0750 /opt/nemqo-samp/server/samp03svr /opt/nemqo-samp/server/samp-npc /opt/nemqo-samp/server/announce
sudo chmod 0640 /opt/nemqo-samp/server/server.cfg
sudo chmod 0640 /opt/nemqo-samp/server/scriptfiles/accounts.db
```

### 5.5 Install systemd service

```bash
sudo cp config/samp.service.example /etc/systemd/system/nemqo-samp.service
sudo sed -i 's|/opt/nemqo-samp/server|/opt/nemqo-samp/server|g' /etc/systemd/system/nemqo-samp.service
sudo systemctl daemon-reload
sudo systemctl enable --now nemqo-samp.service
```

Check it:

```bash
systemctl status nemqo-samp.service --no-pager
journalctl -u nemqo-samp.service -n 100 --no-pager
```

## 6. Build from source

### 6.1 Prepare the legacy compiler and official SDK

You need:

- Pawn compiler executable `pawncc` compatible with `3.2.3664.samp`;
- its matching `libpawnc.so`;
- official SA-MP include directory containing `a_samp.inc` and `a_npc.inc`.

Example environment:

```bash
export PAWN_COMPILER_DIR=/opt/pawn-3.2.3664
export SAMP_INCLUDE_DIR=/opt/samp-sdk/include
```

### 6.2 Run the build

```bash
./scripts/build.sh
```

Or specify paths explicitly:

```bash
./scripts/build.sh \
  --compiler-dir /opt/pawn-3.2.3664 \
  --sdk-include /opt/samp-sdk/include
```

Expected outputs:

```text
server-files/gamemodes/nemqo_freeroam_dm.amx
server-files/npcmodes/ambient_walk.amx
```

The build is rejected if the compiler emits an error or warning.

## 7. Create or reset the clean database

To create a brand-new database from the reviewed schema:

```bash
python3 scripts/create-clean-db.py \
  --schema database/schema.sql \
  --output database/accounts.clean.db
```

Install it as a new runtime database:

```bash
install -m 0640 database/accounts.clean.db server-files/scriptfiles/accounts.db
```

Never overwrite a live `accounts.db` unless you intentionally want to delete all accounts and progression. Back it up first.

Verify the clean database:

```bash
sqlite3 database/accounts.clean.db 'PRAGMA integrity_check;'
sqlite3 database/accounts.clean.db 'SELECT COUNT(*) FROM accounts;'
sqlite3 database/accounts.clean.db 'SELECT COUNT(*) FROM admin_logs;'
```

Expected output:

```text
ok
0
0
```

The five houses and three businesses are catalogue rows and intentionally exist with empty owners.

## 8. Network and firewall

SA-MP uses **UDP**, not TCP.

Allow the default port with the firewall used by your environment:

```bash
sudo ufw allow 7777/udp
```

Or configure the equivalent rule in nftables, OPNsense, a cloud security list or your hosting provider's firewall.

For the original homelab deployment:

```text
IP: 10.0.30.123/24
Gateway: 10.0.30.1
VLAN: 30
Port: UDP 7777
```

A private `10.x.x.x` address is only reachable from the LAN/VPN unless it is deliberately port-forwarded.

## 9. First account and administrator access

1. Join the server and register a normal account.
2. Stop the service before editing the database:

```bash
sudo systemctl stop nemqo-samp.service
```

3. Grant an administrator level to the exact player name:

```bash
sudo -u samp sqlite3 /opt/nemqo-samp/server/scriptfiles/accounts.db \
  "UPDATE accounts SET admin=3 WHERE name='YourPlayerName';"
```

4. Confirm exactly one row changed and start the service:

```bash
sudo -u samp sqlite3 /opt/nemqo-samp/server/scriptfiles/accounts.db \
  "SELECT name,admin FROM accounts WHERE name='YourPlayerName';"
sudo systemctl start nemqo-samp.service
```

RCON login remains available through the normal SA-MP command, but the RCON password must never be committed to Git.

## 10. Verification

Run the included verification script:

```bash
./scripts/verify.sh \
  --service nemqo-samp.service \
  --server-dir /opt/nemqo-samp/server \
  --host 127.0.0.1 \
  --port 7777
```

It verifies:

- systemd service state;
- UDP listener;
- SQLite integrity and empty/real row counts;
- required startup self-test markers;
- absence of fatal runtime errors;
- native SA-MP UDP query when the query helper is available.

Manual checks:

```bash
systemctl is-active nemqo-samp.service
sudo ss -lunp | grep ':7777'
sudo -u samp sqlite3 /opt/nemqo-samp/server/scriptfiles/accounts.db 'PRAGMA integrity_check;'
tail -n 100 /opt/nemqo-samp/server/server_log.txt
```

Required log markers include successful vehicle, label, fuel-marker, island and safety self-tests plus six connected ambient NPCs.

## 11. Backups and updates

Before replacing the gamemode or database:

```bash
sudo systemctl stop nemqo-samp.service
sudo tar -C /opt/nemqo-samp -czf \
  "/opt/nemqo-samp-backup-$(date +%Y%m%d-%H%M%S).tar.gz" server
sudo systemctl start nemqo-samp.service
```

For a code-only update, preserve:

```text
server.cfg
scriptfiles/accounts.db
```

Replace the AMX/source files, verify checksums, restart once and inspect only the new startup-log section.

## 12. Troubleshooting

### `No such file or directory` when `samp03svr` exists

The 32-bit ELF interpreter is missing. Install:

```bash
sudo apt-get install libc6-i386 lib32gcc-s1 lib32stdc++6
```

### `Run time error 18`

The AMX was built by an incompatible/newer Pawn compiler. Use the verified `3.2.3664.samp` toolchain and `samp_empty_prefix.inc`.

### `Unable to open accounts.db`

Check that the service user can write to `scriptfiles/`:

```bash
sudo chown -R samp:samp /opt/nemqo-samp/server/scriptfiles
sudo chmod 0750 /opt/nemqo-samp/server/scriptfiles
```

### NPCs do not connect

Verify:

```text
maxnpc 6
npcmodes/ambient_walk.amx
samp-npc executable permissions
```

### The socket exists but players cannot join

Check the UDP firewall, server log and a real SA-MP query. A TCP port check is not valid for SA-MP.

### Server exits during startup

Read the latest startup window in `server_log.txt`. The gamemode intentionally exits if critical safety, storage or content self-tests fail.
