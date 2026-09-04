"""Fresh pinned-toolchain candidate objects for execution-based codec tests."""

from __future__ import annotations

import os
from pathlib import Path
from typing import Iterable

from scripts.kf.compile import compile_source
from scripts.kf.manifest import load
from scripts.kf.paths import BUILD, REPO


def rebuild_units(names: Iterable[str]) -> None:
    manifest = load()
    includes = [REPO / "include"]
    if os.environ.get("PSYQ_INCLUDE"):
        includes.append(Path(os.environ["PSYQ_INCLUDE"]))
    for name in dict.fromkeys(names):
        unit = manifest.by_name()[name]
        profile = manifest.profiles[unit.profile]
        output = BUILD / "objdiff" / unit.image_key / "base" / unit.object_name
        compile_source(
            unit.source_path, unit.image, output, BUILD / "delink",
            profile.optimization, profile.small_data, profile.aspsx_version,
            tuple(includes), profile.cc1_flags, profile.compiler,
            profile.maspsx_flags, defines=unit.defines,
        )
        print(f"[codec-candidate] rebuilt {name} ({profile.compiler})", flush=True)
