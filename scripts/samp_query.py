#!/usr/bin/env python3
"""Query a SA-MP server with the native UDP information protocol."""
from __future__ import annotations
import argparse
import json
import socket
import struct
import sys


def read_u32_string(packet: bytes, offset: int) -> tuple[str, int]:
    if offset + 4 > len(packet):
        raise ValueError("truncated string length")
    length = struct.unpack_from("<I", packet, offset)[0]
    offset += 4
    if offset + length > len(packet):
        raise ValueError("truncated string payload")
    value = packet[offset:offset + length].decode("latin-1", errors="replace")
    return value, offset + length


def query(host: str, port: int, timeout: float) -> dict[str, object]:
    ip = socket.gethostbyname(host)
    request = b"SAMP" + socket.inet_aton(ip) + struct.pack("<H", port) + b"i"
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(timeout)
        sock.sendto(request, (ip, port))
        packet, source = sock.recvfrom(65535)
    if len(packet) < 16 or packet[:4] != b"SAMP":
        raise ValueError("invalid or truncated SA-MP information response")
    offset = 11
    passworded = bool(packet[offset])
    offset += 1
    players, max_players = struct.unpack_from("<HH", packet, offset)
    offset += 4
    hostname, offset = read_u32_string(packet, offset)
    gamemode, offset = read_u32_string(packet, offset)
    language, offset = read_u32_string(packet, offset)
    return {
        "source": f"{source[0]}:{source[1]}",
        "passworded": passworded,
        "players": players,
        "max_players": max_players,
        "hostname": hostname,
        "gamemode": gamemode,
        "language": language,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("host")
    parser.add_argument("port", nargs="?", type=int, default=7777)
    parser.add_argument("--timeout", type=float, default=3.0)
    args = parser.parse_args()
    if not 1 <= args.port <= 65535:
        parser.error("port must be between 1 and 65535")
    try:
        result = query(args.host, args.port, args.timeout)
    except (OSError, ValueError, struct.error) as exc:
        print(f"SA-MP query failed: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
