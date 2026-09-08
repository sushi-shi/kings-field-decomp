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
from scripts.kf.parser_machine import _ObjectFile, _load_object
from scripts.kf.paths import BUILD, REPO
from scripts.kf.sema.index import index
from scripts.kf.trial_flow import compare_flow, object_flow

INSTRUCTION_RE = re.compile(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(.*)$")
RELOCATION_RE = re.compile(r"^\s*([0-9a-f]+):\s+(R_MIPS\S+)\s+(.*)$")
SECTION_RE = re.compile(r"^Disassembly of section (.*):$")
ABSOLUTE_TARGET_RE = re.compile(r"\b[0-9a-f]+ <([^>]+)>")


def listing(path: Path, obj: _ObjectFile | None = None) -> dict[str, list[str]]:
    """Normalized lines inside ELF function extents, including terminal nops."""
    obj = obj or _load_object(path)
    objdump = shutil.which("mipsel-linux-gnu-objdump")
    if objdump is None:
        raise RuntimeError("mipsel-linux-gnu-objdump not found; enter nix develop")
    output = subprocess.run(
        [objdump, "-drz", str(path)], capture_output=True, text=True, check=True
    ).stdout
    rows: dict[tuple[str, int], list[str]] = {}
    section = ""
    for line in output.splitlines():
        heading = SECTION_RE.match(line)
        if heading:
            section = heading.group(1)
            continue
        instruction = INSTRUCTION_RE.match(line)
        if instruction:
            # Branch targets print as section offsets; keep only the
            # symbol-relative form so a size change in an earlier function of
            # the module does not shift every later target.
            text = ABSOLUTE_TARGET_RE.sub(r"<\1>", instruction.group(2).strip())
            rows.setdefault((section, int(instruction.group(1), 16)), []).append(text)
            continue
        relocation = RELOCATION_RE.match(line)
        if relocation:
            rows.setdefault((section, int(relocation.group(1), 16)), []).append(
                f"  {relocation.group(2)} {relocation.group(3)}")
    functions: dict[str, list[str]] = {}
    for fn in obj.symbols:
        if fn.kind != "STT_FUNC" or fn.section is None:
            continue
        if fn.size <= 0 or fn.size % 4 or fn.value % 4:
            raise ValueError(f"{path}: {fn.name}: missing/unaligned ELF function extent")
        lines: list[str] = []
        for offset in range(fn.value, fn.value + fn.size, 4):
            decoded = rows.get((fn.section, offset))
            if decoded is None:
                raise ValueError(f"{path}: {fn.name}: no disassembly at +0x{offset - fn.value:x}")
            lines.extend(decoded)
        functions[fn.name] = lines
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
    idx = index(unit.image)
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
            defines=unit.defines,
            section_alignment=profile.section_alignment,
        )
        base_object = _load_object(output)
        base = listing(output, base_object)
    target_object = _load_object(target)
    expected = listing(target, target_object)
    exact = 0
    for function in unit.functions:
        want = expected.get(function.symbol, [])
        have = base.get(function.symbol)
        if have is None:
            print(f"MISSING {function.symbol}: not defined by the compiled object")
            continue
        if want == have:
            exact += 1
            print(f"SAME    {function.symbol} (listing only)")
            continue
        ratio = difflib.SequenceMatcher(None, want, have).ratio() * 100
        print(f"DIFF    {function.symbol} ({ratio:.1f}% similar; < target, > compiled)")
        binding = idx.function(function.va)
        if binding is not None:
            for clue in compare_flow(object_flow(target_object, binding, idx),
                                     object_flow(base_object, binding, idx)):
                print("    " + clue)
        for line in difflib.unified_diff(want, have, "target", "compiled", n=context, lineterm=""):
            if line.startswith(("---", "+++")):
                continue
            print("    " + line.replace("\t", " "))
    print(f"{exact}/{len(unit.functions)} identical listings in {unit.unit}; "
          "strict exactness requires kf match")
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
