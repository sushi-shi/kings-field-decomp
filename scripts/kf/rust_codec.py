"""Binary transport to the std host driver of the independent no_std codecs."""

from __future__ import annotations

import struct
import subprocess
from pathlib import Path
from typing import Sequence

from scripts.kf.paths import REPO


DEFAULT_DRIVER = REPO / "tools/target/debug/kf-codec-oracle"


def encode_blocks(blocks: Sequence[bytes]) -> bytes:
    return struct.pack("<I", len(blocks)) + b"".join(
        struct.pack("<I", len(block)) + block for block in blocks
    )


def decode_blocks(data: bytes) -> tuple[bytes, ...]:
    if len(data) < 4:
        raise ValueError("Rust driver response has no block count")
    count = struct.unpack_from("<I", data)[0]
    if count > 128:
        raise ValueError(f"Rust driver response has {count} blocks (maximum 128)")
    at = 4
    result = []
    for _ in range(count):
        if at + 4 > len(data):
            raise ValueError("Rust driver response has a truncated block header")
        size = struct.unpack_from("<I", data, at)[0]
        at += 4
        if at + size > len(data):
            raise ValueError("Rust driver response has a truncated block")
        result.append(data[at:at + size])
        at += size
    if at != len(data):
        raise ValueError("Rust driver response has trailing bytes")
    return tuple(result)


def build_driver() -> Path:
    subprocess.run(
        ["cargo", "build", "--offline", "--manifest-path", str(REPO / "tools/Cargo.toml"),
         "--bin", "kf-codec-oracle"],
        cwd=REPO, check=True,
    )
    return DEFAULT_DRIVER


class RustCodec:
    def __init__(self, executable: Path = DEFAULT_DRIVER):
        self.executable = executable

    def call(self, operation: str, *blocks: bytes) -> tuple[bytes, ...]:
        process = subprocess.run(
            [str(self.executable), operation], input=encode_blocks(blocks),
            capture_output=True, check=False, timeout=30,
        )
        if process.returncode:
            raise RuntimeError(
                f"Rust {operation}: {process.stderr.decode('utf-8', errors='replace').strip()}"
            )
        return decode_blocks(process.stdout)
