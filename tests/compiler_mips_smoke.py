"""End-to-end native GCC 2.6.0 PSX -> maspsx -> GNU-as smoke test."""

from __future__ import annotations

import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.compile import compile_source
from scripts.kf.retail import write_tsv


def main() -> int:
    for tool in (
        "cpppsx-260",
        "cc1psx-260",
        "maspsx",
        "mipsel-linux-gnu-as",
        "mipsel-linux-gnu-nm",
    ):
        if shutil.which(tool) is None:
            raise RuntimeError(f"{tool} is missing; run inside nix develop")

    with tempfile.TemporaryDirectory(prefix="kf-compiler-mips-") as directory:
        root = Path(directory)
        source = root / "simple.c"
        output = root / "simple.o"
        manifest = root / "delink/game/objects.tsv"
        source.write_text(
            "int add(int left, int right) { return left + right; }\n",
            encoding="utf-8",
        )
        write_tsv(
            manifest,
            (
                "image", "va", "size", "body_size", "name", "scope",
                "provider", "library", "object", "relocations", "confidence",
                "provenance",
            ),
            ({
                "image": "GAME.EXE",
                "va": "0x80010000",
                "size": "0x4",
                "body_size": "0x4",
                "name": "add",
                "scope": "decomp",
                "provider": "",
                "library": "",
                "object": "objects/simple.o",
                "relocations": 0,
                "confidence": "test",
                "provenance": "test",
            },),
            (),
        )
        compile_source(
            source,
            "GAME.EXE",
            output,
            root / "delink",
            optimization="O2",
        )
        data = output.read_bytes()
        if data[:7] != b"\x7fELF\x01\x01\x01":
            raise RuntimeError("compiler pipeline did not emit ELF32 little-endian")
        if struct.unpack_from("<H", data, 18)[0] != 8:
            raise RuntimeError("compiler pipeline did not emit EM_MIPS")
        symbols = subprocess.run(
            ["mipsel-linux-gnu-nm", str(output)],
            capture_output=True,
            text=True,
            check=True,
        ).stdout
        if " add" not in symbols:
            raise RuntimeError(f"compiled add symbol is missing: {symbols}")
    print("GCC 2.6.0 PSX C calibration: simple.c -> MIPS ELF object")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
