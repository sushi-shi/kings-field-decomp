"""Require explicit object-pointer erasure at void-pointer boundaries.

Clang's -Wimplicit-void-ptr-cast covers the reverse conversion in C. This
read-only AST check covers typed pointers converted to void pointers, which
are valid (and silent) in both C and C++. It never rewrites checker input.
"""

from dataclasses import dataclass
from functools import cache
from pathlib import Path

from scripts.kf.clangd import unit_arguments
from scripts.kf.manifest import Unit


@dataclass(frozen=True, order=True)
class PointerErasure:
    file: Path
    start: int
    end: int
    line: int
    column: int
    source: str
    target: str


def _mode_erasures(
    unit: Unit, repo: Path, sdk: Path, mode: str,
) -> tuple[PointerErasure, ...]:
    from clang import cindex

    repo = repo.resolve()
    args = unit_arguments(unit, repo, "clang", sdk, mode=mode)
    args = args[1:args.index("-c")]
    tu = cindex.Index.create().parse(str(repo / unit.source), args=args)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError(f"{mode} pointer-policy parse failed:\n" + "\n".join(errors))
    found: set[PointerErasure] = set()

    @cache
    def source_path(filename):
        path = Path(filename).resolve()
        return path if path.is_relative_to(repo) else None

    explicit_casts = {
        cindex.CursorKind.CSTYLE_CAST_EXPR,
        cindex.CursorKind.CXX_STATIC_CAST_EXPR,
        cindex.CursorKind.CXX_REINTERPRET_CAST_EXPR,
        cindex.CursorKind.CXX_FUNCTIONAL_CAST_EXPR,
    }

    def visit(node, explicit=False):
        if node.location.file is not None:
            if source_path(str(node.location.file)) is None:
                return
        children = tuple(node.get_children())
        if not explicit and node.kind == cindex.CursorKind.UNEXPOSED_EXPR and len(children) == 1:
            target = node.type.get_canonical()
            source = children[0].type.get_canonical()
            pointer = cindex.TypeKind.POINTER
            if (target.kind == pointer and source.kind == pointer
                    and target.get_pointee().kind == cindex.TypeKind.VOID
                    and source.get_pointee().kind != cindex.TypeKind.VOID):
                start, end = node.extent.start, node.extent.end
                if start.file is not None:
                    path = source_path(str(start.file))
                    if path is not None:
                        found.add(PointerErasure(
                            path, start.offset, end.offset, start.line, start.column,
                            children[0].type.spelling, node.type.spelling,
                        ))
        # C++ represents an explicit cast's conversion with an implicit child.
        # Suppress only that conversion chain, never a nested call's arguments.
        child_explicit = explicit and node.kind in (
            cindex.CursorKind.UNEXPOSED_EXPR, cindex.CursorKind.PAREN_EXPR,
        )
        if node.kind in explicit_casts:
            target = node.type.get_canonical()
            child_explicit = (
                target.kind == cindex.TypeKind.POINTER
                and target.get_pointee().kind == cindex.TypeKind.VOID
            )
        for child in children:
            visit(child, child_explicit)

    visit(tu.cursor)
    return tuple(sorted(found))


def implicit_void_erasures(unit: Unit, repo: Path, sdk: Path) -> tuple[PointerErasure, ...]:
    # The C SDK has old-style declarations where the modern view supplies
    # prototypes. Neither language's conversions are a superset of the other.
    found = {}
    for mode in ("retail", "modern"):
        for site in _mode_erasures(unit, repo, sdk, mode):
            found[site.file, site.start, site.end, site.target] = site
    return tuple(sorted(found.values()))
