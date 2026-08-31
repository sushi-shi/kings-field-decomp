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


ORDER_SOURCE = """\
static __inline__ int inline_candidate(int value) { return value - 7; }
int third(int value) { return inline_candidate(value * 3 + 8); }
int first(int value) { return third(value) + 5; }
int second(int value) { return first(value) - 2; }
"""


def function_symbols(path: Path) -> list[tuple[int, str]]:
    output = subprocess.run(
        ["mipsel-linux-gnu-nm", "-n", "--defined-only", str(path)],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    symbols = []
    for line in output.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in {"t", "T"}:
            symbols.append((int(parts[0], 16), parts[2]))
    return symbols


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
        order_source = root / "order.c"
        order_o0 = root / "order-o0.o"
        order_o2 = root / "order-o2.o"
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
            tuple({
                "image": "GAME.EXE",
                "va": f"{0x80010000 + index * 4:#x}",
                "size": "0x4",
                "body_size": "0x4",
                "name": Path(name).stem,
                "scope": "decomp",
                "provider": "",
                "library": "",
                "object": f"objects/{name}",
                "relocations": 0,
                "confidence": "test",
                "provenance": "test",
            } for index, name in enumerate(("simple.o", "order-o0.o", "order-o2.o"))),
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

        order_source.write_text(ORDER_SOURCE, encoding="utf-8")
        compile_source(
            order_source,
            "GAME.EXE",
            order_o0,
            root / "delink",
            optimization="O0",
        )
        compile_source(
            order_source,
            "GAME.EXE",
            order_o2,
            root / "delink",
            optimization="O2",
        )
        o0_symbols = function_symbols(order_o0)
        o2_symbols = function_symbols(order_o2)
        expected_order = ["third", "first", "second"]
        probe_names = {"inline_candidate", *expected_order}
        o0_names = [name for _address, name in o0_symbols if name in probe_names]
        o2_names = [name for _address, name in o2_symbols if name in probe_names]
        if "inline_candidate" not in o0_names:
            raise RuntimeError(f"GCC 2.6.0 -O0 omitted the inline body: {o0_symbols}")
        if [name for name in o0_names if name != "inline_candidate"] != expected_order:
            raise RuntimeError(f"GCC 2.6.0 -O0 changed source function order: {o0_symbols}")
        if o2_names != expected_order:
            raise RuntimeError(
                "GCC 2.6.0 -O2 did not preserve exported source order or omit the "
                f"inlined static body: {o2_symbols}"
            )
        o0_offsets = {name: address for address, name in o0_symbols}
        o2_offsets = {name: address for address, name in o2_symbols}
        if all(o0_offsets[name] == o2_offsets[name] for name in expected_order[1:]):
            raise RuntimeError(
                "optimization did not move either downstream function offset; "
                "the shift calibration is ineffective"
            )
    print(
        "GCC 2.6.0 PSX C calibration: MIPS ELF, source-order emission at O0/O2, "
        "O2 inlined-static omission, and downstream offset shifts"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
