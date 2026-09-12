"""Check every selected source/image variant with modern enum domains."""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import subprocess

from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import REPO


def select_units(
    manifest: Manifest, *, images: tuple[str, ...] = (), names: tuple[str, ...] = (),
) -> tuple[Unit, ...]:
    known = manifest.by_name()
    for name in names:
        if name not in known:
            raise ValueError(f"unknown unit {name!r}")
        if images and known[name].image not in images:
            raise ValueError(f"unit {name!r} is outside the selected images")
        if manifest.profiles[known[name].profile].language != "c":
            raise ValueError(f"unit {name!r} is not a C source")
    units = tuple(
        unit for unit in manifest.units
        if manifest.profiles[unit.profile].language == "c"
        and (not images or unit.image in images)
        and (not names or unit.unit in names)
    )
    if not units:
        raise ValueError("no C sources selected for type checking")
    return units


def check(
    *, images: tuple[str, ...] = (), names: tuple[str, ...] = (), jobs: int = 4,
    repo: Path = REPO, manifest: Manifest | None = None,
) -> int:
    if jobs < 1:
        raise ValueError("type-check jobs must be positive")
    units = select_units(manifest or load_manifest(), images=images, names=names)
    compiler, sdk = environment()
    logs = repo / "build/clangd/checks"
    logs.mkdir(parents=True, exist_ok=True)

    def run(unit: Unit) -> tuple[Unit, subprocess.CompletedProcess]:
        args = unit_arguments(unit, repo, compiler, sdk)
        args[args.index("-c")] = "-fsyntax-only"
        args.insert(1, "-ferror-limit=0")
        result = subprocess.run(
            args, cwd=repo, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        )
        if result.returncode:
            from scripts.kf.c_compat import void_conversions, write_overlay

            try:
                conversions = void_conversions(unit, repo.resolve(), sdk)
                if conversions:
                    overlay = write_overlay(conversions, repo.resolve(), logs / unit.unit)
                    checked = subprocess.run(
                        [*args, "-ivfsoverlay", str(overlay)], cwd=repo, text=True,
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                    )
                    checked.stdout = (
                        result.stdout
                        + f"\n[types] target-C view: {len(conversions)} implicit void-pointer "
                        f"conversion(s); checking generated view {overlay}\n"
                        + checked.stdout
                    )
                    result = checked
            except ValueError as error:
                result.stdout += f"\n[types] {error}\n"
        return unit, result

    failures = 0
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for unit, result in pool.map(run, units):
            (logs / f"{unit.unit}.log").write_text(result.stdout, encoding="utf-8")
            if result.returncode:
                failures += 1
                print(f"[types] FAIL {unit.unit}")
                print(result.stdout, end="" if result.stdout.endswith("\n") else "\n")
            else:
                print(f"[types] PASS {unit.unit}")
    print(f"[types] {len(units) - failures}/{len(units)} source/image variants pass; "
          f"{failures} failed; diagnostics: {logs}")
    return int(failures != 0)
