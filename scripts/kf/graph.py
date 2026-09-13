"""Generate and drive the incremental three-image King's Field build graph."""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

from scripts.kf import compile as compiler
from scripts.kf.clangd import generate as generate_clangd
from scripts.kf.delink import delink, image_key
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.native_referents import MANIFEST as NATIVE_REFERENTS
from scripts.kf.objdiff import generate_projects, generate_report
from scripts.kf.paths import (
    BASELINE,
    BUILD,
    LOCAL_CONFIG,
    NINJA,
    REPO,
    RETAIL_CONFIG,
    TOOLCHAIN_ID,
    UNITS_MANIFEST,
)
from scripts.kf.retail import IMAGE_LAYOUTS


INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*[<"]([^>"]+)[>"]', re.M)
PHASES = ("all", "base", "target", "compare", "verify")


def _write_if_changed(path: Path, content: str | bytes) -> bool:
    payload = content.encode("utf-8") if isinstance(content, str) else content
    if path.is_file() and path.read_bytes() == payload:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    try:
        temporary.write_bytes(payload)
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)
    return True


def _write_generator(path: Path, content: str) -> None:
    """Atomically rewrite a Ninja generator output, including identical text.

    A generator output must become newer than the input that invoked it. A
    write-if-changed build.ninja stays older and makes Ninja regenerate forever.
    """
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    try:
        temporary.write_text(content, encoding="utf-8")
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)


class IncludeScanner:
    """Resolve transitive repo-local quoted or angle includes."""

    def __init__(self) -> None:
        self._direct: dict[str, list[str]] = {}
        self._read: set[str] = set()

    def _includes(self, relative: str) -> list[str]:
        if relative in self._direct:
            return self._direct[relative]
        self._direct[relative] = []
        self._read.add(relative)
        path = REPO / relative
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            return []
        result: list[str] = []
        parent = os.path.dirname(relative)
        for include in INCLUDE_RE.findall(text):
            for base in (parent, "include", "vendor/include"):
                candidate = os.path.normpath(os.path.join(base, include))
                if not candidate.startswith("..") and (REPO / candidate).is_file():
                    result.append(candidate)
                    break
        self._direct[relative] = result
        return result

    def headers(self, source: str) -> list[str]:
        found: set[str] = set()
        stack = [source]
        while stack:
            for header in self._includes(stack.pop()):
                if header not in found:
                    found.add(header)
                    stack.append(header)
        return sorted(found)

    def scanned(self) -> list[str]:
        return sorted(self._read)


def toolchain_identity() -> str:
    tools = (
        "cc1psx-260", "cpppsx-260", "cc1psx-257", "cpppsx-257", "dosbox-x",
        "mipsel-linux-gnu-as", "mipsel-linux-gnu-ld", "mipsel-linux-gnu-objcopy", "psyk", "objdiff-cli",
    )
    rows = []
    for tool in tools:
        resolved = shutil.which(tool)
        rows.append(f"{tool}={os.path.realpath(resolved) if resolved else '-'}")
    for variable in ("PSYQ_SDK", "PSYQ_INCLUDE", "PSYQ_LIB", "PSYQ_ASPSX", "PSYQ_BIN"):
        rows.append(f"{variable}={os.environ.get(variable) or '-'}")
    return "\n".join(rows) + "\n"


def _ninja_path(path: str | Path) -> str:
    return str(path).replace("$", "$$").replace(" ", "$ ").replace(":", "$:")


def _build_line(
    outputs: str | list[str],
    rule: str,
    *,
    inputs: list[str] | None = None,
    implicit: list[str] | None = None,
    variables: dict[str, str] | None = None,
) -> list[str]:
    output_list = [outputs] if isinstance(outputs, str) else outputs
    line = "build " + " ".join(_ninja_path(item) for item in output_list) + f": {rule}"
    if inputs:
        line += " " + " ".join(_ninja_path(item) for item in inputs)
    if implicit:
        line += " | " + " ".join(_ninja_path(item) for item in implicit)
    lines = [line]
    for key, value in (variables or {}).items():
        lines.append(f"  {key} = {value}")
    return lines


def _script_inputs() -> list[str]:
    return sorted(str(path.relative_to(REPO))
                  for directory in ("scripts/kf", "scripts/psxbuild")
                  for path in (REPO / directory).rglob("*.py"))


def _base_path(unit: Unit) -> str:
    return f"build/objdiff/{unit.image_key}/base/{unit.object_name}"


def _prune_orphans(manifest: Manifest) -> int:
    live = {_base_path(unit) for unit in manifest.units}
    removed = 0
    for key in ("psx", "game", "open"):
        base = BUILD / "objdiff" / key / "base"
        if not base.is_dir():
            continue
        for path in base.glob("*.o"):
            relative = str(path.relative_to(REPO))
            if relative not in live:
                path.unlink()
                path.with_suffix(path.suffix + ".json").unlink(missing_ok=True)
                removed += 1
    return removed


def emit(out: Path = NINJA, retail_dir: Path | None = None) -> tuple[int, int]:
    from scripts.kf.config_data import load as load_contributions

    manifest = load_manifest()
    generate_clangd(manifest)
    contributions = load_contributions(modules=manifest.modules())
    retail = configured_retail_dir(retail_dir)
    pruned = _prune_orphans(manifest)
    _write_if_changed(TOOLCHAIN_ID, toolchain_identity())

    scanner = IncludeScanner()
    headers = {unit.unit: scanner.headers(unit.source) for unit in manifest.units}
    py = f"PYTHONPATH={shlex.quote(str(REPO))} python3"
    lines = [
        "# GENERATED by scripts.kf.graph from config/units.toml; do not edit.",
        "ninja_required_version = 1.11",
        "builddir = build",
        f"py = {py}",
        f"retail = {shlex.quote(str(retail))}",
        "",
        "rule configure",
        "  command = $py -m scripts.kf.cli configure",
        "  description = configure build/build.ninja",
        "  generator = 1",
        "",
        "rule delink",
        "  command = $py -m scripts.kf.graph edge-delink --image $image --retail-dir $retail --stamp $out",
        "  description = delink $image",
        "  restat = 1",
        "",
        "rule compile",
        "  command = $py -m scripts.kf.graph edge-compile --unit $unit --out $out",
        "  description = compile $unit",
        "  restat = 1",
        "",
        "rule sdkdata",
        "  command = $py -m scripts.kf.graph edge-sdkdata --unit $unit --out $out",
        "  description = import SDK data $unit",
        "  restat = 1",
        "",
        "rule project",
        "  command = $py -m scripts.kf.graph edge-project",
        "  description = objdiff project",
        "  restat = 1",
        "",
        "rule report",
        "  command = $py -m scripts.kf.graph edge-report --out $out",
        "  description = objdiff report",
        "  restat = 1",
        "",
        "rule check",
        "  command = $py -m scripts.kf.graph edge-check --image $image --stamp $out",
        "  description = check $image",
        "  restat = 1",
        "",
    ]

    generator_inputs = [
        str(UNITS_MANIFEST.relative_to(REPO)),
        str((RETAIL_CONFIG / "data_contributions.tsv").relative_to(REPO)),
        str((RETAIL_CONFIG / "data_identities.tsv").relative_to(REPO)),
        str((RETAIL_CONFIG / "data_sections.tsv").relative_to(REPO)),
        str((RETAIL_CONFIG / "data_reservations.tsv").relative_to(REPO)),
        *_script_inputs(),
        *scanner.scanned(),
    ]
    if LOCAL_CONFIG.is_file():
        generator_inputs.append(str(LOCAL_CONFIG.relative_to(REPO)))
    lines += _build_line(
        str(out.relative_to(REPO)), "configure", implicit=sorted(set(generator_inputs))
    )
    lines.append("")

    config_inputs = [
        str(path.relative_to(REPO))
        for path in sorted(RETAIL_CONFIG.glob("*.tsv"))
    ]
    scripts = _script_inputs()
    project = "build/objdiff/objdiff.json"
    report = "build/objdiff/report.json"
    project_inputs: list[str] = []
    for image in IMAGE_LAYOUTS:
        key = image_key(image)
        units = [unit for unit in manifest.units if unit.image == image]
        bases = [_base_path(unit) for unit in units]
        data_contributions = [c for c in contributions if c.image == image]
        bases += [str(c.base_path().relative_to(REPO)) for c in data_contributions]
        delink_stamp = f"build/delink/{key}/.delink.stamp"
        verify_stamp = f"build/objdiff/{key}/.verify.stamp"
        executable = str((retail / image))
        lines += _build_line(
            delink_stamp,
            "delink",
            inputs=[executable],
            implicit=[
                *config_inputs,
                *scripts,
                str(UNITS_MANIFEST.relative_to(REPO)),
                *[unit.source for unit in units],
            ],
            variables={"image": image},
        )
        for contribution in data_contributions:
            archive = Path(os.environ.get("PSYQ_LIB", "")) / contribution.library
            if not archive.is_file():
                raise ValueError(f"missing SDK archive {archive}; use nix develop")
            lines += _build_line(
                str(contribution.base_path().relative_to(REPO)), "sdkdata", inputs=[str(archive)],
                implicit=[*config_inputs, *scripts, str(TOOLCHAIN_ID.relative_to(REPO))],
                variables={"unit": contribution.unit},
            )
        for unit in units:
            profile = manifest.profiles[unit.profile]
            lines += _build_line(
                _base_path(unit),
                "compile",
                inputs=[unit.source],
                implicit=[
                    *headers[unit.unit],
                    str(NATIVE_REFERENTS.relative_to(REPO)),
                    delink_stamp,
                    str(UNITS_MANIFEST.relative_to(REPO)),
                    str(TOOLCHAIN_ID.relative_to(REPO)),
                    *scripts,
                ],
                variables={
                    "unit": unit.unit,
                    "profile": profile.name,
                },
            )
        project_inputs.extend([delink_stamp, *bases])
        lines += _build_line(
            verify_stamp,
            "check",
            inputs=[report],
            implicit=[
                str(BASELINE.relative_to(REPO)),
                str(TOOLCHAIN_ID.relative_to(REPO)),
                *scripts,
            ],
            variables={"image": image},
        )
        lines += _build_line(f"base-{key}", "phony", inputs=bases)
        lines += _build_line(f"target-{key}", "phony", inputs=[delink_stamp])
        lines += _build_line(f"compare-{key}", "phony", inputs=[report])
        lines += _build_line(f"verify-{key}", "phony", inputs=[verify_stamp])
        lines += _build_line(f"all-{key}", "phony", inputs=[report, verify_stamp])
        lines.append("")

    lines += _build_line(
        project,
        "project",
        inputs=project_inputs,
        implicit=[str(UNITS_MANIFEST.relative_to(REPO)), *scripts],
    )
    lines += _build_line(
        report,
        "report",
        inputs=[project, *project_inputs],
        # Recompiled objects may restat unchanged when only objdiff changes.
        # Its identity must invalidate the report directly, not via objects.
        implicit=[str(TOOLCHAIN_ID.relative_to(REPO)), *scripts],
    )

    for phase in PHASES:
        inputs = [f"{phase}-{image_key(image)}" for image in IMAGE_LAYOUTS]
        lines += _build_line(phase, "phony", inputs=inputs)
    lines += ["default all", ""]
    _write_generator(out, "\n".join(lines))
    return len(manifest.units), pruned


def configure_if_needed(force: bool = False, retail_dir: Path | None = None) -> None:
    repinned = NINJA.is_file() and (
        not TOOLCHAIN_ID.is_file() or TOOLCHAIN_ID.read_text() != toolchain_identity()
    )
    if force or repinned or not NINJA.is_file():
        units, pruned = emit(NINJA, retail_dir)
        suffix = f", pruned {pruned} orphan(s)" if pruned else ""
        print(f"[configure] wrote {NINJA.relative_to(REPO)} ({units} units{suffix})")
    else:
        # Refresh environment paths and recreate a removed editor database even
        # when the Ninja graph itself is current.
        generate_clangd()


def run_ninja(
    targets: list[str], *, jobs: int | None = None, verbose: bool = False
) -> int:
    arguments = ["ninja", "-f", str(NINJA.relative_to(REPO))]
    if jobs is not None:
        arguments += ["-j", str(jobs)]
    if verbose:
        arguments.append("-v")
    arguments.extend(targets)
    return subprocess.run(arguments, cwd=REPO).returncode


def _directory_digest(path: Path) -> str:
    digest = hashlib.sha256()
    for item in sorted(file for file in path.rglob("*") if file.is_file()):
        if item.name.startswith("."):
            continue
        digest.update(str(item.relative_to(path)).encode())
        digest.update(b"\0")
        digest.update(item.read_bytes())
    return digest.hexdigest() + "\n"


def edge_delink(image: str, retail_dir: Path, stamp: Path) -> int:
    manifest = load_manifest(write_bindings_file=True)
    modules = tuple(module for module in manifest.modules() if module.image == image)
    delink(retail_dir, RETAIL_CONFIG, BUILD / "delink", (image,), (), "safe", modules)
    _write_if_changed(stamp, _directory_digest(stamp.parent))
    return 0


def edge_compile(unit_name: str, output: Path) -> int:
    manifest = load_manifest()
    unit = manifest.by_name().get(unit_name)
    if unit is None:
        raise ValueError(f"unknown unit {unit_name!r}")
    profile = manifest.profiles[unit.profile]
    includes = [REPO / "include", REPO / "vendor/include"]
    if os.environ.get("PSYQ_INCLUDE"):
        includes.append(Path(os.environ["PSYQ_INCLUDE"]))
    compiler.compile_source(
        unit.source_path,
        unit.image,
        output,
        BUILD / "delink",
        profile.optimization,
        profile.small_data,
        profile.aspsx_version,
        tuple(includes),
        profile.cc1_flags,
        profile.compiler,
        defines=unit.defines,
    )
    return 0


def edge_sdkdata(unit_name: str, output: Path) -> int:
    from scripts.kf.config_data import build_base, load as load_contributions

    units = {c.unit: c for c in load_contributions(modules=load_manifest().modules())}
    if unit_name not in units:
        raise ValueError(f"unknown config-data contribution {unit_name!r}")
    build_base(units[unit_name], output)
    return 0


def edge_project() -> int:
    generate_projects(
        BUILD / "delink",
        BUILD / "objdiff",
        manifest=load_manifest(),
    )
    return 0


def edge_report(output: Path) -> int:
    return_code = generate_report(BUILD / "objdiff", output)
    print(return_code)
    return 0


def edge_check(image: str, stamp: Path) -> int:
    from scripts.kf.progress import check

    result = check((image,))
    if result == 0:
        report = BUILD / "objdiff" / "report.json"
        payload = hashlib.sha256(report.read_bytes() + BASELINE.read_bytes()).hexdigest()
        _write_if_changed(stamp, payload + "\n")
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subs = parser.add_subparsers(dest="command", required=True)
    subs.add_parser("edge-project")
    report_parser = subs.add_parser("edge-report")
    report_parser.add_argument("--out", required=True, type=Path)
    for command in ("edge-delink", "edge-check"):
        sub = subs.add_parser(command)
        sub.add_argument("--image", required=True, choices=tuple(IMAGE_LAYOUTS))
        if command == "edge-delink":
            sub.add_argument("--retail-dir", required=True, type=Path)
            sub.add_argument("--stamp", required=True, type=Path)
        if command == "edge-check":
            sub.add_argument("--stamp", required=True, type=Path)
    compile_parser = subs.add_parser("edge-compile")
    compile_parser.add_argument("--unit", required=True)
    compile_parser.add_argument("--out", required=True, type=Path)
    sdk_parser = subs.add_parser("edge-sdkdata")
    sdk_parser.add_argument("--unit", required=True)
    sdk_parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        if args.command == "edge-delink":
            return edge_delink(args.image, args.retail_dir, args.stamp)
        if args.command == "edge-compile":
            return edge_compile(args.unit, args.out)
        if args.command == "edge-sdkdata":
            return edge_sdkdata(args.unit, args.out)
        if args.command == "edge-project":
            return edge_project()
        if args.command == "edge-report":
            return edge_report(args.out)
        return edge_check(args.image, args.stamp)
    except (OSError, RuntimeError, ValueError) as error:
        print(f"[{args.command}] {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
