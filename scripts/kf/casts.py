"""Count written C-style casts with the target-C Clang AST.

The same source can appear in several image/define contexts, and casts written
in shared headers or macros can expand many times.  This audit parses every
selected manifest variant, attributes each cast to its spelling location, and
deduplicates that location while retaining all contexts and observed types.
"""

from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ProcessPoolExecutor
from ctypes import byref, c_uint
from dataclasses import asdict, dataclass
import hashlib
import json
import multiprocessing
import os
from pathlib import Path
import sys
from typing import Any, Sequence

from scripts.kf.check_types import select_units
from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.manifest import Manifest, Unit, load as load_manifest
from scripts.kf.paths import REPO


IMAGE_NAMES = {"psx": "PSX.EXE", "game": "GAME.EXE", "open": "OPEN.EXE"}
SOURCE_SUFFIXES = {".c", ".h", ".inc"}


@dataclass(frozen=True, order=True)
class Location:
    file: str
    line: int
    column: int
    offset: int


@dataclass(frozen=True, order=True)
class TypeDescription:
    spelling: str
    canonical: str
    kind: str
    size: int


@dataclass(frozen=True, order=True)
class Context:
    image: str
    unit: str
    function: str


@dataclass(frozen=True)
class CastSite:
    location: Location
    category: str
    source_types: tuple[TypeDescription, ...]
    target_types: tuple[TypeDescription, ...]
    contexts: tuple[Context, ...]
    line_text: str


@dataclass(frozen=True)
class Audit:
    variants: int
    ast_casts: int
    external_macro_casts: int
    sites: tuple[CastSite, ...]
    headers_seen: tuple[str, ...]


@dataclass(frozen=True)
class _RawCast:
    origin: Location
    category: str
    source_type: TypeDescription
    target_type: TypeDescription
    context: Context


@dataclass(frozen=True)
class _UnitResult:
    casts: tuple[_RawCast, ...]
    ast_casts: int
    external_macro_casts: int
    headers: tuple[str, ...]
    errors: tuple[str, ...]


def _cindex():
    try:
        from clang import cindex  # type: ignore[import-not-found]
    except ImportError as error:
        raise RuntimeError(
            "the cast audit requires the pinned libclang binding; enter nix develop"
        ) from error
    # Load libclang before worker threads create independent indexes.
    cindex.conf.get_cindex_library()
    return cindex


def _source_files(repo: Path) -> tuple[Path, ...]:
    return tuple(sorted(
        path
        for root in (repo / "src", repo / "include", repo / "vendor")
        if root.is_dir()
        for path in root.rglob("*")
        if path.is_file() and path.suffix in SOURCE_SUFFIXES
    ))


def _hashes(repo: Path) -> dict[str, str]:
    return {
        str(path.relative_to(repo)): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in _source_files(repo)
    }


def _relative_location(
    filename: str, line: int, column: int, offset: int, repo: Path,
) -> Location | None:
    if not filename:
        return None
    try:
        relative = Path(filename).resolve().relative_to(repo)
    except ValueError:
        return None
    return Location(str(relative), line, column, offset)


def _location(location: Any, repo: Path) -> Location | None:
    filename = str(location.file) if location.file is not None else ""
    return _relative_location(
        filename, location.line, location.column, location.offset, repo,
    )


def _spelling_location(location: Any, repo: Path, cindex: Any) -> Location | None:
    file_pointer = cindex.c_object_p()
    line = c_uint()
    column = c_uint()
    offset = c_uint()
    cindex.conf.lib.clang_getSpellingLocation(
        location, byref(file_pointer), byref(line), byref(column), byref(offset),
    )
    filename = str(cindex.File(file_pointer)) if file_pointer else ""
    return _relative_location(
        filename, line.value, column.value, offset.value, repo,
    )


def _describe(typ: Any) -> TypeDescription:
    canonical = typ.get_canonical()
    kind = str(canonical.kind).rsplit(".", 1)[-1].removeprefix("CXType_").lower()
    try:
        size = canonical.get_size()
    except Exception:  # libclang reports invalid/incomplete types through the binding.
        size = -1
    return TypeDescription(typ.spelling, canonical.spelling, kind, size)


def _cast_category(typ: Any, cindex: Any) -> str:
    return "pointer" if typ.get_canonical().kind == cindex.TypeKind.POINTER else "scalar"


def _parse_unit(unit: Unit, repo: Path, sdk: Path, cindex: Any) -> _UnitResult:
    path = repo / unit.source
    arguments = [
        *MODES["retail"], *FLAGS,
        "-I", str(repo / "include"), "-I", str(repo / "vendor/include"), "-isystem", str(sdk),
        *(f"-D{define}" for define in unit.defines),
    ]
    translation_unit = cindex.Index.create().parse(
        str(path),
        args=arguments,
        options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
    )
    errors = tuple(
        str(diagnostic)
        for diagnostic in translation_unit.diagnostics
        if diagnostic.severity >= cindex.Diagnostic.Error
    )
    headers: set[str] = set()
    for inclusion in translation_unit.get_includes():
        if inclusion.include is None:
            continue
        try:
            relative = Path(str(inclusion.include)).resolve().relative_to(repo)
        except ValueError:
            continue
        if relative.parts and (relative.parts[0] == "include" or relative.suffix == ".inc"):
            headers.add(str(relative))

    casts: list[_RawCast] = []
    ast_casts = 0
    external_macro_casts = 0
    expression_kinds = {
        cindex.CursorKind.UNEXPOSED_EXPR,
        cindex.CursorKind.PAREN_EXPR,
    }

    def walk(cursor: Any, function: str = "") -> None:
        nonlocal ast_casts, external_macro_casts
        here = _location(cursor.location, repo)
        if cursor.kind != cindex.CursorKind.TRANSLATION_UNIT and here is None:
            return
        if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
            function = cursor.spelling
        children = tuple(cursor.get_children())
        if cursor.kind == cindex.CursorKind.CSTYLE_CAST_EXPR:
            ast_casts += 1
            origin = _spelling_location(cursor.location, repo, cindex)
            if origin is None:
                external_macro_casts += 1
            else:
                operands = [child for child in children if child.kind.is_expression()]
                operand = operands[-1] if operands else None
                while operand is not None and operand.kind in expression_kinds:
                    nested = [child for child in operand.get_children()
                              if child.kind.is_expression()]
                    if len(nested) != 1:
                        break
                    operand = nested[0]
                source_type = (
                    _describe(operand.type) if operand is not None
                    else TypeDescription("", "", "invalid", -1)
                )
                casts.append(_RawCast(
                    origin,
                    _cast_category(cursor.type, cindex),
                    source_type,
                    _describe(cursor.type),
                    Context(unit.image, unit.unit, function),
                ))
        for child in children:
            walk(child, function)

    walk(translation_unit.cursor)
    return _UnitResult(
        tuple(casts), ast_casts, external_macro_casts,
        tuple(sorted(headers)), errors,
    )


def _parse_unit_worker(arguments: tuple[Unit, Path, Path]) -> _UnitResult:
    unit, repo, sdk = arguments
    return _parse_unit(unit, repo, sdk, _cindex())


def _sdk_path(sdk: Path | None) -> Path:
    if sdk is None:
        value = os.environ.get("PSYQ_INCLUDE")
        if not value:
            raise ValueError("cast auditing requires PSYQ_INCLUDE; enter nix develop")
        sdk = Path(value)
    resolved = sdk.resolve()
    if not resolved.is_dir():
        raise ValueError(f"PSYQ_INCLUDE is not a directory: {resolved}")
    return resolved


def collect(
    *,
    images: tuple[str, ...] = (),
    names: tuple[str, ...] = (),
    jobs: int = 4,
    repo: Path = REPO,
    manifest: Manifest | None = None,
    sdk: Path | None = None,
) -> Audit:
    """Parse selected target-C variants and return deduplicated written casts."""
    if jobs < 1:
        raise ValueError("cast-audit jobs must be positive")
    repo = repo.resolve()
    selected = select_units(manifest or load_manifest(), images=images, names=names)
    before = _hashes(repo)
    texts = {
        path: (repo / path).read_text(encoding="utf-8", errors="replace").splitlines()
        for path in before
    }
    sdk = _sdk_path(sdk)
    work = tuple((unit, repo, sdk) for unit in selected)
    if jobs == 1:
        results = tuple(_parse_unit_worker(arguments) for arguments in work)
    else:
        # Loading libclang independently avoids the binding's shared-state
        # deadlocks while walking several large ASTs at once.
        with ProcessPoolExecutor(
            max_workers=min(jobs, len(selected)),
            mp_context=multiprocessing.get_context("fork"),
        ) as pool:
            results = tuple(pool.map(_parse_unit_worker, work))

    if _hashes(repo) != before:
        raise RuntimeError("source changed during cast audit; rerun on a stable tree")
    errors = [error for result in results for error in result.errors]
    if errors:
        detail = "\n".join(errors[:20])
        suffix = f"\n... {len(errors) - 20} more" if len(errors) > 20 else ""
        raise RuntimeError(
            f"target-C parsing produced {len(errors)} error(s):\n{detail}{suffix}"
        )

    merged: dict[Location, dict[str, set[Any]]] = {}
    for result in results:
        for cast in result.casts:
            row = merged.setdefault(cast.origin, {
                "categories": set(), "sources": set(), "targets": set(), "contexts": set(),
            })
            row["categories"].add(cast.category)
            row["sources"].add(cast.source_type)
            row["targets"].add(cast.target_type)
            row["contexts"].add(cast.context)

    sites: list[CastSite] = []
    for location, row in sorted(merged.items()):
        categories = row["categories"]
        category = next(iter(categories)) if len(categories) == 1 else "mixed"
        lines = texts[location.file]
        line_text = lines[location.line - 1].strip() if 0 < location.line <= len(lines) else ""
        sites.append(CastSite(
            location,
            category,
            tuple(sorted(row["sources"])),
            tuple(sorted(row["targets"])),
            tuple(sorted(row["contexts"])),
            line_text,
        ))

    headers = tuple(sorted({header for result in results for header in result.headers}))
    if not images and not names:
        expected_headers = {
            path for path in before if (path.startswith("include/") and path.endswith(".h"))
            or path.endswith(".inc")
        }
        missing = sorted(expected_headers - set(headers))
        if missing:
            raise RuntimeError(
                f"cast audit did not parse {len(missing)} project header(s): "
                + ", ".join(missing[:10])
            )
    return Audit(
        len(selected),
        sum(result.ast_casts for result in results),
        sum(result.external_macro_casts for result in results),
        tuple(sites),
        headers,
    )


def filter_sites(
    sites: Sequence[CastSite], *, kind: str = "all", scope: str = "all",
    paths: tuple[str, ...] = (),
) -> tuple[CastSite, ...]:
    def keep(site: CastSite) -> bool:
        if kind != "all" and site.category != kind:
            return False
        is_header = site.location.file.startswith("include/")
        if scope == "source" and is_header:
            return False
        if scope == "header" and not is_header:
            return False
        return not paths or any(site.location.file.startswith(path) for path in paths)

    return tuple(site for site in sites if keep(site))


def summary(sites: Sequence[CastSite]) -> dict[str, Any]:
    by_kind = Counter(site.category for site in sites)
    by_scope = Counter(
        "header" if site.location.file.startswith("include/") else "source"
        for site in sites
    )
    by_scope_kind = Counter(
        (
            "header" if site.location.file.startswith("include/") else "source",
            site.category,
        )
        for site in sites
    )
    return {
        "total": len(sites),
        "by_kind": dict(sorted(by_kind.items())),
        "by_scope": dict(sorted(by_scope.items())),
        "by_scope_and_kind": {
            scope: {
                kind: by_scope_kind[(scope, kind)]
                for kind in ("pointer", "scalar", "mixed")
                if by_scope_kind[(scope, kind)]
            }
            for scope in ("source", "header")
            if by_scope[scope]
        },
    }


def document(audit: Audit, sites: Sequence[CastSite]) -> dict[str, Any]:
    return {
        "schema": 1,
        "mode": "target-c-gnu89",
        "variants": audit.variants,
        "ast_cast_expansions": audit.ast_casts,
        "external_macro_cast_expansions": audit.external_macro_casts,
        "headers_seen": list(audit.headers_seen),
        "summary": summary(sites),
        "sites": [asdict(site) for site in sites],
    }


def _print_summary(audit: Audit, sites: Sequence[CastSite]) -> None:
    counts = summary(sites)
    print(
        f"[casts] {counts['total']} written C-style cast(s); "
        f"{audit.variants} target-C variant(s); {audit.ast_casts} AST expansion(s)"
    )
    for scope in ("source", "header"):
        kinds = counts["by_scope_and_kind"].get(scope)
        if not kinds:
            continue
        parts = ", ".join(f"{kind} {number}" for kind, number in kinds.items())
        print(f"[casts] {scope}: {sum(kinds.values())} ({parts})")
    top = Counter(site.location.file for site in sites).most_common(15)
    if top:
        print("[casts] top files:")
        for path, number in top:
            print(f"    {number:4}  {path}")


def _print_sites(sites: Sequence[CastSite]) -> None:
    for site in sites:
        source = " | ".join(value.spelling for value in site.source_types)
        target = " | ".join(value.spelling for value in site.target_types)
        contexts = ", ".join(
            f"{context.unit}:{context.function or '<file>'}" for context in site.contexts
        )
        print(
            f"{site.location.file}:{site.location.line}:{site.location.column}\t"
            f"{site.category}\t{source} -> {target}\t{contexts}\t{site.line_text}"
        )


def add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--image", action="append", choices=tuple(IMAGE_NAMES))
    parser.add_argument("--unit", action="append")
    parser.add_argument("-j", "--jobs", type=int, default=4)
    parser.add_argument("--kind", choices=("all", "pointer", "scalar", "mixed"),
                        default="all")
    parser.add_argument("--scope", choices=("all", "source", "header"), default="all")
    parser.add_argument("--path", action="append", default=[], metavar="PREFIX",
                        help="keep spelling locations under this repository path prefix")
    output = parser.add_mutually_exclusive_group()
    output.add_argument("--list", action="store_true", help="list every selected cast site")
    output.add_argument("--json", action="store_true", help="emit the selected sites as JSON")
    parser.add_argument("--max", type=int, dest="maximum", metavar="N",
                        help="fail when the selected written-site count exceeds N")


def run(args: argparse.Namespace) -> int:
    if args.maximum is not None and args.maximum < 0:
        raise ValueError("--max must be non-negative")
    images = tuple(IMAGE_NAMES[value] for value in args.image or ())
    audit = collect(
        images=images,
        names=tuple(args.unit or ()),
        jobs=args.jobs,
    )
    sites = filter_sites(
        audit.sites,
        kind=args.kind,
        scope=args.scope,
        paths=tuple(args.path),
    )
    if args.json:
        print(json.dumps(document(audit, sites), indent=2))
    elif args.list:
        _print_sites(sites)
        print(f"# {len(sites)} written C-style cast(s)")
    else:
        _print_summary(audit, sites)
    if args.maximum is not None and len(sites) > args.maximum:
        print(
            f"[casts] limit exceeded: {len(sites)} > {args.maximum}",
            file=sys.stderr,
        )
        return 1
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="kf casts", description=__doc__)
    add_arguments(parser)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
