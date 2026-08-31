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
from scripts.kf.manifest import Manifest, load as load_manifest
from scripts.kf.retail import IMAGE_LAYOUTS, read_tsv, write_tsv


SCHEMA = "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json"
PAIRING_FIELDS = (
    "image",
    "va",
    "name",
    "body_size",
    "unit",
    "base",
    "target",
    "base_status",
)
EXCLUDED_FIELDS = (
    "image",
    "va",
    "name",
    "provider",
    "library",
    "target",
    "reason",
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
    manifest: Manifest | None = None,
) -> dict[str, tuple[Path, int, int, int]]:
    results: dict[str, tuple[Path, int, int, int]] = {}
    for image in images:
        key = image_key(image)
        target_dir = delink_dir / key
        object_manifest = target_dir / "objects.tsv"
        if not object_manifest.is_file():
            raise ValueError(f"{object_manifest}: run kf-delink first")
        _, all_objects = read_tsv(object_manifest)
        objects = [
            row for row in all_objects
            if row.get("scope", "decomp") == "decomp"
        ]
        excluded = [
            row for row in all_objects
            if row.get("scope", "decomp") == "vendored"
        ]

        project_dir = output_dir / key
        base_dir = project_dir / "base"
        base_dir.mkdir(parents=True, exist_ok=True)
        # Older projects paired missing reconstruction with this dummy. It made
        # zero-total units look matched, so it is no longer part of scoring.
        (project_dir / "missing-base.o").unlink(missing_ok=True)
        selected = {
            (unit.image, unit.va): unit
            for unit in manifest.units
        } if manifest is not None else {}

        units = []
        pairing_rows = []
        paired = 0
        for row in objects:
            target = target_dir / row["object"]
            base = base_dir / Path(row["object"]).name
            identity = image, int(row["va"], 0)
            selected_unit = selected.get(identity) if manifest is not None else None
            admitted = selected_unit is not None if manifest is not None else base.is_file()
            present = admitted and base.is_file()
            paired += int(present)
            unit_name = (
                selected_unit.unit if selected_unit is not None
                else (f"{key}:{row['va']}:{row['name']}" if admitted else "")
            )
            if present:
                units.append({
                    "name": unit_name,
                    "base_path": _relative(base, project_dir),
                    "target_path": _relative(target, project_dir),
                })
            pairing_rows.append({
                "image": image,
                "va": row["va"],
                "name": row["name"],
                "body_size": row["body_size"],
                "unit": unit_name,
                "base": _relative(base, project_dir),
                "target": _relative(target, project_dir),
                "base_status": (
                    "present" if present else
                    "manifest-missing-base" if admitted else
                    "unstarted"
                ),
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
        project_content = json.dumps(project, indent=2) + "\n"
        if (
            not project_path.is_file()
            or project_path.read_text(encoding="utf-8") != project_content
        ):
            project_path.write_text(project_content, encoding="utf-8")
        write_tsv(
            project_dir / "pairings.tsv",
            PAIRING_FIELDS,
            pairing_rows,
            (
                "GENERATED - one objdiff project per independently linked program.",
                "Place reconstructed objects under base/ using the target object filename.",
                "Only manifested units with real base objects enter objdiff scoring.",
            ),
        )
        write_tsv(
            project_dir / "vendored_excluded.tsv",
            EXCLUDED_FIELDS,
            ({
                "image": image,
                "va": row["va"],
                "name": row["name"],
                "provider": row.get("provider", ""),
                "library": row.get("library", ""),
                "target": _relative(target_dir / row["object"], project_dir),
                "reason": "vendored-library-code",
            } for row in excluded),
            (
                "GENERATED - provider-identified functions excluded from matching.",
                "They remain target/reference objects but never count as decomp units.",
            ),
        )
        results[image] = project_path, paired, len(objects), len(excluded)
    return results


def generate_report(project_dir: Path, output: Path | None = None) -> Path:
    project_dir = project_dir.resolve()
    project_path = project_dir / "objdiff.json"
    if not project_path.is_file():
        raise ValueError(f"{project_dir}: objdiff.json is missing")
    output = (output or project_dir / "report.json").resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    project = json.loads(project_path.read_text(encoding="utf-8"))
    if not project.get("units"):
        empty = {
            "units": [],
            "measures": {
                "total_units": 0,
                "total_functions": 0,
                "matched_functions": 0,
                "total_code": "0",
                "matched_code": "0",
                "fuzzy_match_percent": 0.0,
            },
        }
        content = json.dumps(empty, indent=2) + "\n"
        if not output.is_file() or output.read_text(encoding="utf-8") != content:
            output.write_text(content, encoding="utf-8")
        return output
    executable = shutil.which("objdiff-cli")
    if executable is None:
        raise RuntimeError("objdiff-cli not found; enter nix develop")
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
    project_parser.add_argument("--manifest", type=Path)

    report_parser = subparsers.add_parser("report")
    report_parser.add_argument("--project-dir", required=True, type=Path)
    report_parser.add_argument("--output", type=Path)

    args = parser.parse_args()
    if args.command == "project":
        results = generate_projects(
            args.delink_dir,
            args.output_dir,
            args.image or IMAGE_LAYOUTS,
            load_manifest(args.manifest) if args.manifest else None,
        )
        for image, (path, paired, total, excluded) in results.items():
            print(
                f"{image}: {path} ({paired}/{total} reconstructed bases present; "
                f"{excluded} vendored targets excluded)"
            )
    else:
        print(generate_report(args.project_dir, args.output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
