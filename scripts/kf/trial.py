"""Compile one unit source and compare it per function against its module target.

This is the fast inner loop of a matching campaign: it compiles the unit's
source (or an alternative file with the same claims) with the unit's profile
into a scratch object, then prints, for every claimed function, whether the
instruction stream and ordered relocations are identical to the carved retail
run. Nothing under build/objdiff changes; `kf match` remains the recorded
comparison.
"""

from __future__ import annotations

import argparse
import difflib
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from scripts.kf import compile as compiler
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import BUILD, REPO

INSTRUCTION_RE = re.compile(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(.*)$")
RELOCATION_RE = re.compile(r"^\s*([0-9a-f]+):\s+(R_MIPS\S+)\s+(.*)$")
SYMBOL_RE = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")
ABSOLUTE_TARGET_RE = re.compile(r"\b[0-9a-f]+ <([^>]+)>")


def listing(path: Path) -> dict[str, list[str]]:
    """Normalized instruction/relocation lines per function symbol."""
    objdump = shutil.which("mipsel-linux-gnu-objdump")
    if objdump is None:
        raise RuntimeError("mipsel-linux-gnu-objdump not found; enter nix develop")
    output = subprocess.run(
        [objdump, "-dr", str(path)], capture_output=True, text=True, check=True
    ).stdout
    functions: dict[str, list[str]] = {}
    current: list[str] | None = None
    for line in output.splitlines():
        symbol = SYMBOL_RE.match(line)
        if symbol:
            current = functions.setdefault(symbol.group(1), [])
            continue
        if current is None:
            continue
        instruction = INSTRUCTION_RE.match(line)
        if instruction:
            # Branch targets print as section offsets; keep only the
            # symbol-relative form so a size change in an earlier function of
            # the module does not shift every later target.
            text = ABSOLUTE_TARGET_RE.sub(r"<\1>", instruction.group(2).strip())
            current.append(text)
            continue
        relocation = RELOCATION_RE.match(line)
        if relocation:
            current.append(f"  {relocation.group(2)} {relocation.group(3)}")
    for lines in functions.values():
        while lines and lines[-1] == "nop":
            lines.pop()
    return functions


def compare(unit_name: str, source: Path | None, context: int) -> int:
    manifest = load_manifest()
    unit = manifest.by_name().get(unit_name)
    if unit is None:
        raise ValueError(f"unknown unit {unit_name!r}")
    profile = manifest.profiles[unit.profile]
    target = BUILD / "delink" / unit.image_key / "modules" / unit.object_name
    if not target.is_file():
        raise ValueError(f"{target}: module target is missing; run `kf build` first")
    includes = [REPO / "include"]
    if os.environ.get("PSYQ_INCLUDE"):
        includes.append(Path(os.environ["PSYQ_INCLUDE"]))
    with tempfile.TemporaryDirectory(prefix="kf-try-") as directory:
        output = Path(directory) / unit.object_name
        compiler.compile_source(
            source or unit.source_path,
            unit.image,
            output,
            BUILD / "delink",
            profile.optimization,
            profile.small_data,
            profile.aspsx_version,
            tuple(includes),
            profile.cc1_flags,
            profile.compiler,
            profile.maspsx_flags,
        )
        base = listing(output)
    expected = listing(target)
    exact = 0
    for function in unit.functions:
        want = expected.get(function.symbol, [])
        have = base.get(function.symbol)
        if have is None:
            print(f"MISSING {function.symbol}: not defined by the compiled object")
            continue
        if want == have:
            exact += 1
            print(f"EXACT   {function.symbol}")
            continue
        ratio = difflib.SequenceMatcher(None, want, have).ratio() * 100
        print(f"DIFF    {function.symbol} ({ratio:.1f}% similar; < target, > compiled)")
        for line in difflib.unified_diff(want, have, "target", "compiled", n=context, lineterm=""):
            if line.startswith(("---", "+++")):
                continue
            print("    " + line.replace("\t", " "))
    print(f"{exact}/{len(unit.functions)} exact in {unit.unit}")
    return 0 if exact == len(unit.functions) else 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--unit", required=True)
    parser.add_argument("--source", type=Path, help="alternative source with the same claims")
    parser.add_argument("--context", type=int, default=2)
    args = parser.parse_args()
    return compare(args.unit, args.source, args.context)


if __name__ == "__main__":
    raise SystemExit(main())
