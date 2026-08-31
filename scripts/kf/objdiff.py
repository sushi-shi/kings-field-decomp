"""Generate and score one objdiff project for each King's Field program."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Iterable

from scripts.kf.delink import image_key
from scripts.kf.mips_elf import write_mips_elf
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv, write_tsv


SCHEMA = "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json"
PAIRING_FIELDS = (
    "image",
    "va",
    "name",
    "unit",
    "base",
    "target",
    "base_status",
)


def _relative(path: Path, directory: Path) -> str:
    value = os.path.relpath(path.resolve(), directory.resolve())
    if not value.startswith("."):
        value = "./" + value
    return value


def generate_projects(
    delink_dir: Path,
    output_dir: Path,
    images: Iterable[str] = IMAGE_LAYOUTS,
) -> dict[str, tuple[Path, int, int]]:
    results: dict[str, tuple[Path, int, int]] = {}
    for image in images:
        key = image_key(image)
        target_dir = delink_dir / key
        manifest = target_dir / "objects.tsv"
        if not manifest.is_file():
            raise ValueError(f"{manifest}: run kf-delink first")
        _, objects = read_tsv(manifest)

        project_dir = output_dir / key
        base_dir = project_dir / "base"
        base_dir.mkdir(parents=True, exist_ok=True)
        dummy = project_dir / "missing-base.o"
        dummy.write_bytes(write_mips_elf(
            b"\0\0\0\0", "__kf_missing_reconstruction", 4
        ))

        units = []
        pairing_rows = []
        paired = 0
        for row in objects:
            target = target_dir / row["object"]
            base = base_dir / Path(row["object"]).name
            present = base.is_file()
            paired += int(present)
            unit_name = f"{key}:{row['va']}:{row['name']}"
            units.append({
                "name": unit_name,
                "base_path": _relative(base if present else dummy, project_dir),
                "target_path": _relative(target, project_dir),
            })
            pairing_rows.append({
                "image": image,
                "va": row["va"],
                "name": row["name"],
                "unit": unit_name,
                "base": _relative(base, project_dir),
                "target": _relative(target, project_dir),
                "base_status": "present" if present else "missing-placeholder",
            })

        project = {
            "$schema": SCHEMA,
            "build_base": False,
            "build_target": False,
            "options": {"functionRelocDiffs": "all"},
            "watch_patterns": ["**/*.o"],
            "units": units,
        }
        project_path = project_dir / "objdiff.json"
        project_path.write_text(
            json.dumps(project, indent=2) + "\n", encoding="utf-8"
        )
        write_tsv(
            project_dir / "pairings.tsv",
            PAIRING_FIELDS,
            pairing_rows,
            (
                "GENERATED - one objdiff project per independently linked program.",
                "Place reconstructed objects under base/ using the target object filename.",
                "Missing reconstructions pair with a synthetic MIPS placeholder.",
            ),
        )
        results[image] = project_path, paired, len(units)
    return results


def generate_report(project_dir: Path, output: Path | None = None) -> Path:
    executable = shutil.which("objdiff-cli")
    if executable is None:
        raise RuntimeError("objdiff-cli not found; enter nix develop")
    project_dir = project_dir.resolve()
    if not (project_dir / "objdiff.json").is_file():
        raise ValueError(f"{project_dir}: objdiff.json is missing")
    output = (output or project_dir / "report.json").resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    process = subprocess.run(
        [
            executable,
            "report",
            "generate",
            "-p",
            str(project_dir),
            "-o",
            str(output),
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    if not output.is_file():
        details = (process.stderr or process.stdout).strip()
        raise RuntimeError(
            f"objdiff-cli produced no report (exit {process.returncode}): {details}"
        )
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    project_parser = subparsers.add_parser("project")
    project_parser.add_argument("--delink-dir", type=Path, default=Path("build/delink"))
    project_parser.add_argument("--output-dir", type=Path, default=Path("build/objdiff"))
    project_parser.add_argument(
        "--image", action="append", choices=tuple(IMAGE_LAYOUTS)
    )

    report_parser = subparsers.add_parser("report")
    report_parser.add_argument("--project-dir", required=True, type=Path)
    report_parser.add_argument("--output", type=Path)

    args = parser.parse_args()
    if args.command == "project":
        results = generate_projects(
            args.delink_dir,
            args.output_dir,
            args.image or IMAGE_LAYOUTS,
        )
        for image, (path, paired, total) in results.items():
            print(f"{image}: {path} ({paired}/{total} reconstructed bases present)")
    else:
        print(generate_report(args.project_dir, args.output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
