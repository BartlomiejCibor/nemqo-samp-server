#!/usr/bin/env python3
"""Create a brand-new NEMQO SQLite database from reviewed SQL."""
import argparse
import os
import sqlite3
import tempfile
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--schema", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    if not args.schema.is_file():
        parser.error(f"schema does not exist: {args.schema}")
    if args.output.exists() and not args.force:
        parser.error(f"output exists (use --force): {args.output}")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix="accounts-clean-", suffix=".db", dir=args.output.parent)
    os.close(fd)
    temp = Path(temp_name)
    try:
        con = sqlite3.connect(temp)
        try:
            con.executescript(args.schema.read_text(encoding="utf-8"))
            result = con.execute("PRAGMA integrity_check").fetchone()[0]
            if result != "ok":
                raise RuntimeError(f"integrity_check returned {result!r}")
            expected = {
                "accounts": 0,
                "houses": 5,
                "businesses": 3,
                "reports": 0,
                "bans": 0,
                "admin_logs": 0,
            }
            for table, count in expected.items():
                actual = con.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
                if actual != count:
                    raise RuntimeError(f"{table}: expected {count}, got {actual}")
            if con.execute("SELECT COUNT(*) FROM houses WHERE owner<>''").fetchone()[0]:
                raise RuntimeError("clean database contains a house owner")
            if con.execute("SELECT COUNT(*) FROM businesses WHERE owner<>''").fetchone()[0]:
                raise RuntimeError("clean database contains a business owner")
            con.execute("VACUUM")
        finally:
            con.close()
        os.replace(temp, args.output)
        os.chmod(args.output, 0o640)
    finally:
        temp.unlink(missing_ok=True)

    print(f"CLEAN_DB_OK={args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
