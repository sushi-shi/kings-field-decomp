"""Count assignments whose value is used by another expression, using libclang.

Scan every manifest C variant in retail C and modern C++ modes, including
project headers and macro expansions. Standalone assignments and assignments
in for initializers/increments are statements; conditions, arguments, returns,
initializers, comma operands and chained assignments are counted.
"""
from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import sys
from typing import Any

from clang import cindex

from scripts.kf.casts import _spelling_location
from scripts.kf.check_types import select_units
from scripts.kf.clangd import environment, unit_arguments
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import REPO

CK = cindex.CursorKind
ASSIGNMENTS = {'Assign', 'MulAssign', 'DivAssign', 'RemAssign', 'AddAssign',
               'SubAssign', 'ShlAssign', 'ShrAssign', 'AndAssign', 'XorAssign', 'OrAssign'}
WRAPPERS = {CK.UNEXPOSED_EXPR, CK.PAREN_EXPR}
OVERLOADED = {'operator' + spelling: name for spelling, name in (
    ('=', 'Assign'), ('*=', 'MulAssign'), ('/=', 'DivAssign'), ('%=', 'RemAssign'),
    ('+=', 'AddAssign'), ('-=', 'SubAssign'), ('<<=', 'ShlAssign'),
    ('>>=', 'ShrAssign'), ('&=', 'AndAssign'), ('^=', 'XorAssign'), ('|=', 'OrAssign'),
)}


@dataclass(frozen=True, order=True)
class Site:
    file: str
    line: int
    column: int
    offset: int
    operator: str
    origin_file: str
    origin_offset: int
    function: str
    line_text: str


def _used(ancestors: tuple[Any, ...], cursor: Any) -> bool:
    """Is the assignment an operand or a statement condition?"""
    child = cursor
    for parent in reversed(ancestors):
        if parent.kind in WRAPPERS:
            child = parent
            continue
        if parent.kind.is_expression() or parent.kind in {
                CK.RETURN_STMT, CK.VAR_DECL}:
            return True
        children = tuple(parent.get_children())
        if parent.kind in {CK.IF_STMT, CK.WHILE_STMT,
                           CK.SWITCH_STMT}:
            return bool(children and child == children[0])
        if parent.kind == CK.DO_STMT:
            return bool(children and child == children[-1])
        if parent.kind == CK.FOR_STMT:
            # Missing clauses have no cursor, so use the actual semicolon
            # boundaries instead of assuming a fixed AST child index.
            tokens = list(parent.get_tokens())
            depth = 0
            boundaries = []
            for token in tokens:
                if token.spelling == '(':
                    depth += 1
                elif token.spelling == ')':
                    depth -= 1
                    if depth == 0:
                        break
                elif token.spelling == ';' and depth == 1:
                    boundaries.append(token.location.offset)
            if len(boundaries) != 2:
                raise ValueError(f'cannot locate for condition at {parent.location}')
            return boundaries[0] < child.location.offset < boundaries[1]
        return False
    return False


def scan_file(path: Path, arguments: list[str], *, root: Path) -> tuple[Site, ...]:
    root = root.resolve()
    tu = cindex.Index.create().parse(str(path.resolve()), args=arguments,
                                    options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError(f'{path}: parsing failed:\n' + '\n'.join(errors))
    sites = set()
    texts = {}

    def walk(cursor: Any, ancestors: tuple[Any, ...] = (), function: str = '') -> None:
        if cursor.kind in {CK.FUNCTION_DECL, CK.CXX_METHOD, CK.FUNCTION_TEMPLATE}:
            function = cursor.spelling
        operator = ''
        if cursor.kind in {CK.BINARY_OPERATOR, CK.COMPOUND_ASSIGNMENT_OPERATOR}:
            operator = cursor.binary_operator.name
        elif cursor.kind == CK.CALL_EXPR and cursor.referenced is not None:
            operator = OVERLOADED.get(cursor.referenced.spelling, '')
        if operator in ASSIGNMENTS and _used(ancestors, cursor):
            origin = _spelling_location(cursor.extent.start, root, cindex)
            if origin is None or Path(origin.file).parts[0] not in {'src', 'include'}:
                for child in cursor.get_children():
                    walk(child, (*ancestors, cursor), function)
                return
            location = cursor.location
            if location.file:
                filename = Path(location.file.name).resolve()
                if filename.is_relative_to(root) and filename.relative_to(root).parts[0] in {'src', 'include'}:
                    lines = texts[filename] if filename in texts else filename.read_text().splitlines()
                    texts[filename] = lines
                    sites.add(Site(str(filename.relative_to(root)), location.line,
                                   location.column, location.offset, operator,
                                   origin.file, origin.offset, function,
                                   lines[location.line - 1].strip()))
        for child in cursor.get_children():
            if not child.location.is_in_system_header:
                walk(child, (*ancestors, cursor), function)
    walk(tu.cursor)
    return tuple(sorted(sites))


def census(*, repo: Path = REPO, manifest=None, sdk: Path | None = None) -> list[dict]:
    manifest = manifest or load_manifest()
    units = select_units(manifest)
    compiler, environment_sdk = environment()
    sdk = sdk or environment_sdk
    rows = {}
    for unit in units:
        for mode in ('retail', 'modern'):
            arguments = unit_arguments(unit, repo, compiler, sdk, mode=mode)[1:-2]
            for site in scan_file(repo / unit.source, arguments, root=repo):
                key = (site.file, site.offset, site.origin_file, site.origin_offset, site.operator)
                row = rows.setdefault(key, {**asdict(site), 'contexts': []})
                context = {'image': unit.image, 'unit': unit.unit, 'mode': mode,
                           'function': site.function}
                if context not in row['contexts']:
                    row['contexts'].append(context)
    return [rows[key] for key in sorted(rows)]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--json', action='store_true')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args(argv)
    try:
        sites = census()
    except (ValueError, RuntimeError, OSError, cindex.TranslationUnitLoadError) as error:
        print(f'expression-assignments: {error}', file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps({'variants': len(select_units(load_manifest())), 'modes': ['retail', 'modern'],
                          'count': len(sites), 'sites': sites}, indent=2))
    else:
        for site in sites:
            print(f"{site['file']}:{site['line']}:{site['column']}: {site['operator']}: {site['line_text']}")
        print(f'{len(sites)} assignments inside expressions')
    return int(args.check and bool(sites))


if __name__ == '__main__':
    raise SystemExit(main())
