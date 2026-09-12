"""Compare evaluated enum constants across manifest C variants.

Equal values are search leads, not semantic equivalence. Reuses the target-C
pylibclang parsing approach from pointer_zeros and the earlier cast census.
Only active declarations in project sources and included headers are visible.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import sys

from pylibclang import cindex

from scripts.kf.check_types import select_units
from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import REPO


@dataclass(frozen=True, order=True)
class Constant:
    value: int
    name: str
    file: str
    line: int
    column: int
    offset: int
    enum: str


def scan_file(path: Path, arguments: list[str], *, root: Path) -> tuple[Constant, ...]:
    """Resolve expressions, aliases and implicit values with the target ABI."""
    root = root.resolve()
    tu = cindex.Index.create().parse(str(path.resolve()), args=arguments)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError(f"{path}: parsing failed:\n" + "\n".join(errors))
    constants = set()

    def walk(cursor):
        if cursor.kind == cindex.CursorKind.CXCursor_EnumConstantDecl:
            location = cursor.location
            if location.file is not None:
                filename = Path(location.file.name).resolve()
                if filename.is_relative_to(root):
                    constants.add(Constant(
                        cursor.enum_value, cursor.spelling, str(filename.relative_to(root)),
                        location.line, location.column, location.offset,
                        "" if cursor.semantic_parent.is_anonymous()
                        else cursor.semantic_parent.spelling,
                    ))
        for child in cursor.get_children():
            if not child.location.is_in_system_header:
                walk(child)

    walk(tu.cursor)
    return tuple(sorted(constants))


def collect(units, compiler: str, sdk: Path, *, root: Path = REPO) -> list[dict]:
    # A header declaration appears once, retaining every image/define context.
    # Include value in the key: conditional macros can change the same site.
    rows = {}
    for unit in units:
        arguments = unit_arguments(unit, root, compiler, sdk, mode="retail")[1:-2]
        for constant in scan_file(root / unit.source, arguments, root=root):
            key = (constant.value, constant.name, constant.file, constant.offset)
            row = rows.setdefault(key, {**asdict(constant), "contexts": []})
            context = {"image": unit.image, "unit": unit.unit, "defines": list(unit.defines)}
            if context not in row["contexts"]:
                row["contexts"].append(context)
    return [rows[key] for key in sorted(rows)]


def add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--image", action="append", choices=("psx", "game", "open"), default=[])
    parser.add_argument("--unit", action="append", default=[])
    parser.add_argument("--value", action="append", type=lambda text: int(text, 0), default=[],
                        help="evaluated integer, decimal or hex (repeatable)")
    parser.add_argument("--duplicates", action="store_true",
                        help="only values declared at two or more sites (including repeated names)")
    parser.add_argument("--json", action="store_true", help="JSON instead of a TSV comparison table")


def run(args: argparse.Namespace) -> int:
    try:
        units = select_units(load_manifest(),
                             images=tuple(f"{image.upper()}.EXE" for image in args.image),
                             names=tuple(args.unit))
        compiler, sdk = environment()
        rows = collect(units, compiler, sdk)
    except (ValueError, RuntimeError, OSError, cindex.TranslationUnitLoadError) as error:
        print(f"enums: {error}", file=sys.stderr)
        return 2
    counts = {}
    for row in rows:
        counts[row["value"]] = counts.get(row["value"], 0) + 1
    rows = [row for row in rows
            if (not args.value or row["value"] in args.value)
            and (not args.duplicates or counts[row["value"]] > 1)]
    if args.json:
        print(json.dumps({"variants": len(units), "constants": rows}, indent=2))
    else:
        writer = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
        writer.writerow(("value", "hex", "name", "file", "line", "enum", "images", "units"))
        for row in rows:
            writer.writerow((row["value"], hex(row["value"]), row["name"], row["file"],
                             row["line"], row["enum"],
                             ";".join(sorted({c["image"] for c in row["contexts"]})),
                             ";".join(sorted({c["unit"] for c in row["contexts"]}))))
    print(f"{len(rows)} declarations in {len(units)} C variants", file=sys.stderr)
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    add_arguments(parser)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
