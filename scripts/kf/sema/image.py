"""Verified random access to one selected retail PS-X executable."""

from __future__ import annotations

import struct
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.retail import IMAGE_LAYOUTS, ImageLayout, parse_psx_exe


@dataclass(frozen=True)
class RetailImage:
    image: str
    layout: ImageLayout
    data: bytes
    path: Path | None = None

    @classmethod
    def load(cls, image: str) -> "RetailImage":
        layout = IMAGE_LAYOUTS[image]
        path = configured_retail_dir() / image
        if parse_psx_exe(path) != layout:
            raise ValueError(f"{path}: retail identity/layout mismatch")
        return cls(image, layout, path.read_bytes(), path)

    @classmethod
    def synthetic(
        cls,
        image: str,
        load_address: int,
        payload: bytes,
        *,
        entry: int | None = None,
    ) -> "RetailImage":
        """Construct an in-memory image for semantic-tooling tests."""
        header = bytearray(0x800)
        header[:8] = b"PS-X EXE"
        struct.pack_into(
            "<4I",
            header,
            0x10,
            load_address if entry is None else entry,
            0,
            load_address,
            len(payload),
        )
        layout = ImageLayout(
            image,
            "synthetic",
            len(header) + len(payload),
            load_address if entry is None else entry,
            load_address,
            len(payload),
        )
        return cls(image, layout, bytes(header) + payload)

    @property
    def load_start(self) -> int:
        return self.layout.load_address

    @property
    def load_end(self) -> int:
        return self.layout.load_end

    def contains(self, va: int, size: int = 1) -> bool:
        return self.layout.contains(va, size)

    def read(self, va: int, size: int) -> bytes | None:
        if not self.contains(va, size):
            return None
        start = self.layout.file_offset(va)
        payload = self.data[start:start + size]
        return payload if len(payload) == size else None

    def require(self, va: int, size: int) -> bytes:
        payload = self.read(va, size)
        if payload is None:
            raise ValueError(
                f"{self.image}: {va:#010x}+{size:#x} is outside the retail load image"
            )
        return payload

    def u32(self, va: int) -> int | None:
        payload = self.read(va, 4)
        return struct.unpack_from("<I", payload)[0] if payload is not None else None

    def region(self, va: int) -> str:
        return "load" if self.contains(va) else "outside-load"


@lru_cache(maxsize=3)
def retail(image: str) -> RetailImage:
    return RetailImage.load(image)
