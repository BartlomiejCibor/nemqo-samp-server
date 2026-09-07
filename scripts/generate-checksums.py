#!/usr/bin/env python3
"""Generate deterministic SHA-256 checksums for repository artifacts."""
import hashlib
from pathlib import Path

root = Path(__file__).resolve().parent.parent
output = root / "CHECKSUMS.sha256"
ignored_parts = {".git", "__pycache__"}
ignored_names = {"CHECKSUMS.sha256"}
rows = []
for path in sorted(root.rglob("*")):
    if not path.is_file() or path.name in ignored_names or any(part in ignored_parts for part in path.parts):
        continue
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    rows.append(f"{digest}  {path.relative_to(root).as_posix()}")
output.write_text("\n".join(rows) + "\n", encoding="utf-8")
print(f"CHECKSUMS_OK={len(rows)}")
