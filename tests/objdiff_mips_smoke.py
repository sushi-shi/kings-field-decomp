"""End-to-end GNU-as -> synthetic target -> objdiff MIPS calibration."""

from __future__ import annotations

import json
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.mips_elf import MipsRelocation, write_mips_elf


LEAF_ASSEMBLY = """\
.set noreorder
.set noat
.text
.globl calibration_leaf
.type calibration_leaf,@function
calibration_leaf:
  addiu $v0,$a0,1
  jr $ra
   addu $v0,$v0,$a1
  nop
.size calibration_leaf,.-calibration_leaf
"""

LEAF_TEXT = bytes.fromhex(
    "01 00 82 24 08 00 e0 03 21 10 45 00 00 00 00 00"
)

RELOCATION_ASSEMBLY = """\
.set noreorder
.set noat
.text
.globl relocation_test
.type relocation_test,@function
relocation_test:
  lui $a0,%hi(external_data+0x1234)
  addiu $a0,$a0,%lo(external_data+0x1234)
  jal external_function
   nop
  j local
   nop
local:
  jr $ra
   nop
.size relocation_test,.-relocation_test
"""

RELOCATION_TEXT = struct.pack(
    "<8I",
    0x3C040000,
    0x24841234,
    0x0C000000,
    0,
    0x08000006,
    0,
    0x03E00008,
    0,
)


def _match_percentages(value: object) -> list[float]:
    found: list[float] = []
    if isinstance(value, dict):
        for key, item in value.items():
            if key == "match_percent" and isinstance(item, (int, float)):
                found.append(float(item))
            found.extend(_match_percentages(item))
    elif isinstance(value, list):
        for item in value:
            found.extend(_match_percentages(item))
    return found


def _run_diff(objdiff: str, target: Path, base: Path, symbol: str, output: Path) -> None:
    output.unlink(missing_ok=True)
    process = subprocess.run(
        [
            objdiff,
            "diff",
            "-1",
            str(target),
            "-2",
            str(base),
            symbol,
            "-o",
            str(output),
            "--format",
            "json-pretty",
            "-c",
            "functionRelocDiffs=all",
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    if not output.is_file():
        raise RuntimeError(
            f"objdiff produced no output (exit {process.returncode}): "
            f"{process.stderr or process.stdout}"
        )
    percentages = _match_percentages(json.loads(output.read_text(encoding="utf-8")))
    if 100.0 not in percentages:
        raise RuntimeError(f"{symbol} calibration did not byte-match: {percentages!r}")


def main() -> int:
    assembler = shutil.which("mipsel-linux-gnu-as")
    objdiff = shutil.which("objdiff-cli")
    if assembler is None or objdiff is None:
        raise RuntimeError("run this smoke test inside nix develop")

    with tempfile.TemporaryDirectory(prefix="kf-objdiff-mips-") as directory:
        root = Path(directory)
        source = root / "base.s"
        base = root / "base.o"
        target = root / "target.o"
        output = root / "diff.json"
        source.write_text(LEAF_ASSEMBLY, encoding="utf-8")
        target.write_bytes(write_mips_elf(
            LEAF_TEXT, "calibration_leaf", len(LEAF_TEXT)
        ))
        subprocess.run(
            [assembler, "-march=r3000", "-mabi=32", "-o", str(base), str(source)],
            check=True,
        )
        _run_diff(objdiff, target, base, "calibration_leaf", output)

        source.write_text(RELOCATION_ASSEMBLY, encoding="utf-8")
        target.write_bytes(write_mips_elf(
            RELOCATION_TEXT,
            "relocation_test",
            len(RELOCATION_TEXT),
            (
                MipsRelocation(0, "R_MIPS_HI16", "external_data"),
                MipsRelocation(4, "R_MIPS_LO16", "external_data"),
                MipsRelocation(8, "R_MIPS_26", "external_function"),
                MipsRelocation(16, "R_MIPS_26", ".text"),
            ),
        ))
        subprocess.run(
            [assembler, "-march=r3000", "-mabi=32", "-o", str(base), str(source)],
            check=True,
        )
        _run_diff(objdiff, target, base, "relocation_test", output)
    print(
        "objdiff MIPS calibration: calibration_leaf and relocation_test=100.0%"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
