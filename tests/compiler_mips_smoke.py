"""End-to-end native GCC 2.6.0/2.5.7 PSX -> maspsx -> GNU-as smoke test."""

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


PROBE_257_SOURCE = """\
extern void callee(int value);
int ratio(int value, int span) { return (value << 6) / (span + 1) + 1; }
void framed(int value) { callee(value); callee(value + 1); }
"""

DATA_CLAIM_SOURCE = """\
#define DATA(va, size)
typedef unsigned long u32;
DATA(0x80057b0c, 0x4)
static u32 counter = 0;
DATA(0x80057b10, 0x10)
u32 table[4] = {1, 2, 3, 4};
void tick(void) { counter += table[1]; }
"""


def text_words(path: Path) -> list[int]:
    data = subprocess.run(
        ["mipsel-linux-gnu-objcopy", "-O", "binary", "--only-section=.text",
         str(path), "/dev/stdout"],
        capture_output=True,
        check=True,
    ).stdout
    return list(struct.unpack(f"<{len(data) // 4}I", data[: len(data) // 4 * 4]))


def object_symbols(path: Path) -> dict[str, tuple[str, int, str]]:
    """name -> (binding, size, section) for OBJECT-typed symbols."""
    output = subprocess.run(
        ["mipsel-linux-gnu-readelf", "-s", "-W", str(path)],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    symbols = {}
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[3] == "OBJECT":
            symbols[parts[7]] = (parts[4], int(parts[2]), parts[6])
    return symbols


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
        "cpppsx-257",
        "cc1psx-257",
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
        probe_source = root / "probe257.c"
        probe_257 = root / "probe257.o"
        claim_source = root / "claims.c"
        claim_object = root / "claims.o"
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
            } for index, name in enumerate((
                "simple.o", "order-o0.o", "order-o2.o", "probe257.o", "claims.o",
            ))),
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

        # GCC 2.5.7 probe calibration against two retail-observed forms: every
        # framed GAME.EXE epilogue restores $sp in the `jr $ra` delay slot, and
        # signed division carries the checked ASPSX expansion (`break 7`).
        probe_source.write_text(PROBE_257_SOURCE, encoding="utf-8")
        compile_source(
            probe_source,
            "GAME.EXE",
            probe_257,
            root / "delink",
            optimization="O2",
            compiler="gcc257-native",
            maspsx_flags=("--expand-div",),
        )
        words = text_words(probe_257)
        jr_ra = 0x03E00008
        returns = [index for index, word in enumerate(words) if word == jr_ra]
        if not returns:
            raise RuntimeError("GCC 2.5.7 probe object has no jr $ra")
        framed = [
            index for index in returns
            if index + 1 < len(words) and words[index + 1] >> 16 == 0x27BD
        ]
        if not framed:
            raise RuntimeError(
                "GCC 2.5.7 probe did not restore $sp in the jr $ra delay slot"
            )
        if 0x0007000D not in words:
            raise RuntimeError("--expand-div did not emit the break 7 divide check")

        # DATA() claims: the compiler prints no data sizes, so kf-compile
        # annotates claimed symbols with the claimed size and object type.
        claim_source.write_text(DATA_CLAIM_SOURCE, encoding="utf-8")
        compile_source(
            claim_source,
            "GAME.EXE",
            claim_object,
            root / "delink",
            optimization="O2",
            compiler="gcc257-native",
            maspsx_flags=("--expand-div",),
        )
        objects = object_symbols(claim_object)
        if objects.get("counter") != ("LOCAL", 4, "3"):
            raise RuntimeError(f"claimed static datum is not a sized local object: {objects}")
        if objects.get("table", ("", 0, ""))[:2] != ("GLOBAL", 16):
            raise RuntimeError(f"claimed initialized table is not a sized global: {objects}")
        if objects["table"][2] != objects["counter"][2]:
            raise RuntimeError("claimed data did not land in one .data section")
    print(
        "GCC 2.6.0 PSX C calibration: MIPS ELF, source-order emission at O0/O2, "
        "O2 inlined-static omission, and downstream offset shifts; "
        "GCC 2.5.7 probe: delay-slot $sp restore, checked div expansion, "
        "sized DATA() claim symbols"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
