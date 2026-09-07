#!/usr/bin/env python3
"""Fail closed if the public export contains secrets or runtime data."""
from __future__ import annotations
import hashlib
import re
import sqlite3
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
errors: list[str] = []
allowed_ips = {"10.0.30.123", "10.0.30.1"}
placeholder = "CHANGE_ME_TO_A_LONG_RANDOM_PASSWORD"
forbidden_names = {"id_rsa", "id_ed25519", "authorized_keys", ".env", "server.cfg"}
forbidden_fragments = [
    "-----BEGIN OPENSSH PRIVATE KEY-----",
    "-----BEGIN RSA PRIVATE KEY-----",
    "/home/sysadmin",
    "/opt/samp/backups",
    "PACKAGE_TEST_ONLY_NOT_PUBLISHED",
]

files = [p for p in root.rglob("*") if p.is_file() and ".git" not in p.parts and "__pycache__" not in p.parts]
for path in files:
    rel = path.relative_to(root).as_posix()
    if path.name in forbidden_names:
        errors.append(f"forbidden filename: {rel}")
    if path.suffix.lower() in {".pem", ".key", ".log"}:
        errors.append(f"forbidden extension: {rel}")
    data = path.read_bytes()
    if b"\0" in data[:8192]:
        continue
    # The scanner contains the forbidden signatures as detection rules.
    if path.resolve() == Path(__file__).resolve():
        continue
    text = data.decode("utf-8", errors="replace")
    for fragment in forbidden_fragments:
        if fragment in text:
            errors.append(f"forbidden content in {rel}: {fragment}")
    for line in text.splitlines():
        clean = line.strip().strip("`")
        if clean.startswith("rcon_password ") and clean != f"rcon_password {placeholder}":
            errors.append(f"non-placeholder RCON value in {rel}")
    for match in re.findall(r"\b(?:10|172\.(?:1[6-9]|2\d|3[01])|192\.168)(?:\.\d{1,3}){2,3}\b", text):
        if match not in allowed_ips:
            # Gameplay coordinates are not dotted IPv4 values; any other private IP is operator data.
            errors.append(f"unexpected private IP in {rel}: {match}")

markdown = sorted(p.relative_to(root).as_posix() for p in files if p.suffix.lower() == ".md")
if markdown != ["INSTALLATION.md", "README.md"]:
    errors.append(f"expected exactly two Markdown documents, got {markdown}")

amx = sorted(p.relative_to(root).as_posix() for p in files if p.suffix.lower() == ".amx")
expected_amx = [
    "server-files/gamemodes/nemqo_freeroam_dm.amx",
    "server-files/npcmodes/ambient_walk.amx",
]
if amx != expected_amx:
    errors.append(f"unexpected AMX set: {amx}")

dbs = sorted(p for p in files if p.suffix.lower() == ".db")
expected_db = [root / "database/accounts.clean.db", root / "server-files/scriptfiles/accounts.db"]
if dbs != expected_db:
    errors.append(f"unexpected database set: {[p.relative_to(root).as_posix() for p in dbs]}")
else:
    digests = {hashlib.sha256(p.read_bytes()).hexdigest() for p in dbs}
    if len(digests) != 1:
        errors.append("clean database copies are not identical")
    for path in dbs:
        con = sqlite3.connect(f"file:{path}?mode=ro", uri=True)
        try:
            if con.execute("PRAGMA integrity_check").fetchone()[0] != "ok":
                errors.append(f"SQLite integrity failed: {path}")
            expected = {"accounts": 0, "houses": 5, "businesses": 3, "reports": 0, "bans": 0, "admin_logs": 0}
            for table, count in expected.items():
                actual = con.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
                if actual != count:
                    errors.append(f"{path.name}:{table} expected {count}, got {actual}")
            if con.execute("SELECT COUNT(*) FROM houses WHERE owner<>''").fetchone()[0]:
                errors.append(f"house owner found in {path}")
            if con.execute("SELECT COUNT(*) FROM businesses WHERE owner<>''").fetchone()[0]:
                errors.append(f"business owner found in {path}")
        finally:
            con.close()

if errors:
    print("SECURITY_SCAN_FAILED", file=sys.stderr)
    for error in errors:
        print(f"- {error}", file=sys.stderr)
    raise SystemExit(1)
print(f"SECURITY_SCAN_OK files={len(files)} markdown=2 amx=2 clean_db=2 allowed_private_ips={','.join(sorted(allowed_ips))}")
