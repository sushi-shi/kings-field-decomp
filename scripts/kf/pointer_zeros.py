"""Find written integer-zero literals converted to pointers; never edit sources.

Run in nix develop: python -m scripts.kf.pointer_zeros [--image game] [--json]
Every manifest C variant is parsed with the retail MIPS flags. This includes
assignments, scalar/aggregate initializers, casts, arguments, returns and
comparisons. Only active preprocessor branches and included headers are visible.
Implicit aggregate zero-fill and arithmetic constant expressions are not written
zero literals. Existing NULL expansions are excluded.
"""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import re
import sys
from typing import Any

from pylibclang import cindex, _C

from scripts.kf.check_types import select_units
from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import REPO


CK = cindex.CursorKind
TK = cindex.TypeKind
WRAPPERS = {CK.CXCursor_UnexposedExpr, CK.CXCursor_ParenExpr, CK.CXCursor_CStyleCastExpr}


@dataclass(frozen=True, order=True)
class Site:
    file: str
    line: int
    column: int
    offset: int
    spelling: str
    pointer_type: str
    function: str
    line_text: str


def _zero_literal(cursor: Any) -> Any | None:
    """Peel parentheses/integer casts without treating p + 0 as a null value."""
    while cursor.kind in WRAPPERS:
        children = [child for child in cursor.get_children() if child.kind.is_expression()]
        if len(children) != 1 or cursor.type.get_canonical().kind == TK.CXType_Pointer:
            return None
        cursor = children[0]
    if cursor.kind != CK.CXCursor_IntegerLiteral:
        return None
    value = _C.clang_Cursor_Evaluate(cursor)
    if value is None:
        return None
    try:
        if (_C.clang_EvalResult_getKind(value) == _C.CXEvalResultKind.CXEval_Int
                and _C.clang_EvalResult_getAsUnsigned(value) == 0):
            return cursor
    finally:
        _C.clang_EvalResult_dispose(value)
    return None


def _null_only_macro(tu: Any, location: Any) -> bool:
    """Exclude nested NULL expansions only when no written zero can explain them."""
    definitions: dict[str, list[str]] = {}
    invocation: list[str] = []
    for cursor in tu.cursor.get_children():
        if cursor.kind == CK.CXCursor_MacroDefinition:
            definitions.setdefault(cursor.spelling, []).extend(
                token.spelling for token in cursor.get_tokens())
        elif (cursor.kind == CK.CXCursor_MacroExpansion
              and cursor.location.file is not None and location.file is not None
              and cursor.location.file.name == location.file.name
              and cursor.location.offset == location.offset):
            invocation.extend(token.spelling for token in cursor.get_tokens())
    seen: set[str] = set()
    has_null = False
    while invocation:
        spelling = invocation.pop()
        if spelling == "NULL":
            has_null = True
        elif re.fullmatch(r"(?:0+|0[xX]0+)[uUlL]*", spelling):
            return False
        elif spelling not in seen:
            seen.add(spelling)
            invocation.extend(definitions.get(spelling, ()))
    return has_null


def _literal_location(tu: Any, literal: Any) -> tuple[Any, str] | None:
    token = next(literal.get_tokens(), None)
    if token is not None:
        return token.location, token.spelling
    # Cross-file macro extents cannot be tokenized as a range. Ask for the
    # spelling token directly; pylibclang owns this returned pointer.
    token = _C.clang_getToken(tu, literal.extent.start)
    if token is not None:
        return _C.clang_getTokenLocation(tu, token), _C.clang_getTokenSpelling(tu, token)
    # Some nested/pasted expansions have no standalone spelling token. Report
    # the actual invocation text at its expansion location for manual review.
    location = literal.location
    if _null_only_macro(tu, location):
        return None
    start = cindex.SourceLocation.from_offset(tu, location.file, location.offset)
    end = cindex.SourceLocation.from_offset(tu, location.file, location.offset + 1)
    token = next(tu.get_tokens(extent=cindex.SourceRange.from_locations(start, end)), None)
    if token is None:
        raise ValueError(f"cannot locate zero literal at {location}")
    return token.location, token.spelling


def scan_file(path: Path, arguments: list[str], *, root: Path) -> tuple[Site, ...]:
    """Parse one C translation unit, reporting only locations under root.

    arguments are Clang frontend flags (no compiler, source or output operands).
    Parse errors abort the scan rather than presenting an incomplete clean result.
    """
    root = root.resolve()
    tu = cindex.Index.create().parse(
        str(path.resolve()), args=arguments,
        options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
    )
    errors = [str(diagnostic) for diagnostic in tu.diagnostics
              if diagnostic.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError(f"{path}: parsing failed:\n" + "\n".join(errors))

    # Token locations recover the written literal in a macro definition or
    # argument. Cursor locations instead point to the macro invocation.
    null_ranges = [cursor.extent for cursor in tu.cursor.get_children()
                   if cursor.kind == CK.CXCursor_MacroDefinition and cursor.spelling == "NULL"]
    texts: dict[Path, list[str]] = {}
    sites: set[Site] = set()

    def walk(cursor: Any, function: str = "") -> None:
        if cursor.kind == CK.CXCursor_FunctionDecl:
            function = cursor.spelling
        children = tuple(cursor.get_children())
        if (cursor.kind in {CK.CXCursor_UnexposedExpr, CK.CXCursor_CStyleCastExpr}
                and cursor.type.get_canonical().kind == TK.CXType_Pointer):
            operands = [child for child in children if child.kind.is_expression()]
            literal = _zero_literal(operands[0]) if len(operands) == 1 else None
            if literal is not None:
                origin = _literal_location(tu, literal)
                if origin is None:
                    return
                location, spelling = origin
                in_null = any(
                    span.start.file is not None and location.file is not None
                    and span.start.file.name == location.file.name
                    and span.start.offset <= location.offset < span.end.offset
                    for span in null_ranges
                )
                if (location.file is not None and not in_null
                        and spelling != "NULL"):
                    filename = Path(location.file.name).resolve()
                    if filename.is_relative_to(root):
                        if filename not in texts:
                            texts[filename] = filename.read_text(encoding="utf-8").splitlines()
                        lines = texts[filename]
                        sites.add(Site(
                            str(filename.relative_to(root)), location.line, location.column,
                            location.offset, spelling,
                            cursor.type.spelling, function,
                            lines[location.line - 1].strip() if location.line else "",
                        ))
        for child in children:
            # SDK declarations cannot contain project-written literal sites.
            if child.location.is_in_system_header:
                continue
            walk(child, function)

    walk(tu.cursor)
    return tuple(sorted(sites))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", choices=("psx", "game", "open"), action="append", default=[])
    parser.add_argument("--unit", action="append", default=[], help="manifest unit (repeatable)")
    parser.add_argument("--path", action="append", default=[], help="output path prefix (repeatable)")
    parser.add_argument("--json", action="store_true", help="emit a JSON report to stdout")
    parser.add_argument("--check", action="store_true", help="exit 1 if any sites remain")
    args = parser.parse_args(argv)
    try:
        manifest = load_manifest()
        units = select_units(
            manifest, images=tuple(f"{image.upper()}.EXE" for image in args.image),
            names=tuple(args.unit),
        )
        compiler, sdk = environment()
        rows: dict[tuple[str, int], dict] = {}
        for unit in units:
            arguments = unit_arguments(unit, REPO, compiler, sdk, mode="retail")[1:-2]
            for site in scan_file(REPO / unit.source, arguments, root=REPO):
                if args.path and not any(site.file.startswith(prefix) for prefix in args.path):
                    continue
                key = (site.file, site.offset)
                row = rows.setdefault(key, {
                    **{key: value for key, value in asdict(site).items()
                       if key not in {"pointer_type", "function"}},
                    "contexts": [],
                })
                context = {"image": unit.image, "unit": unit.unit,
                           "function": site.function, "pointer_type": site.pointer_type}
                if context not in row["contexts"]:
                    row["contexts"].append(context)
        sites = [rows[key] for key in sorted(rows)]
    except (ValueError, RuntimeError, OSError, cindex.TranslationUnitLoadError) as error:
        print(f"pointer-zeros: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps({"variants": len(units), "count": len(sites), "sites": sites}, indent=2))
    else:
        for site in sites:
            print(f"{site['file']}:{site['line']}:{site['column']}: "
                  f"zero converted to pointer: {site['line_text']}")
        print(f"{len(sites)} written sites in {len(units)} C variants", file=sys.stderr)
    return 1 if args.check and sites else 0


if __name__ == "__main__":
    raise SystemExit(main())
