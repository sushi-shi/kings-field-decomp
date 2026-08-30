"""Import a synthetic PS-X EXE to test the loader and compiled PSX language."""

from __future__ import annotations

import struct
import tempfile
from pathlib import Path

import jpype
import pyghidra


with tempfile.TemporaryDirectory(prefix="kf-ghidra-plugin-check-") as directory:
    root = Path(directory)
    binary = root / "SMOKE.EXE"
    image = bytearray(0x1000)
    image[:8] = b"PS-X EXE"
    struct.pack_into(
        "<10I",
        image,
        0x10,
        0x80010000,
        0,
        0x80010000,
        0x800,
        0,
        0,
        0,
        0,
        0,
        0,
    )
    struct.pack_into("<2I", image, 0x800, 0x03E00008, 0)
    binary.write_bytes(image)
    with pyghidra.open_program(
        binary,
        project_location=root / "project",
        project_name="smoke",
        analyze=False,
    ) as api:
        program = api.getCurrentProgram()
        assert str(program.getLanguageID()) == "PSX:LE:32:default"
        assert str(jpype.JClass("psx.PsxLoader")) == "<java class 'psx.PsxLoader'>"
