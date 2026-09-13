"""Require explicit object-pointer erasure at void-pointer boundaries.

Clang's -Wimplicit-void-ptr-cast covers the reverse conversion in C. This
read-only AST check covers typed pointers converted to void pointers, which
are valid (and silent) in both C and C++. It never rewrites checker input.
"""

from dataclasses import dataclass
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


def implicit_void_erasures(unit: Unit, repo: Path, sdk: Path) -> tuple[PointerErasure, ...]:
    from clang import cindex

    repo = repo.resolve()
    args = unit_arguments(unit, repo, "clang", sdk, mode="retail")
    args = args[1:args.index("-c")]
    tu = cindex.Index.create().parse(str(repo / unit.source), args=args)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError("target-C pointer-policy parse failed:\n" + "\n".join(errors))
    found: set[PointerErasure] = set()

    def visit(node):
        if node.location.file is not None:
            path = Path(str(node.location.file)).resolve()
            if not path.is_relative_to(repo):
                return
        children = tuple(node.get_children())
        if node.kind == cindex.CursorKind.UNEXPOSED_EXPR and len(children) == 1:
            target = node.type.get_canonical()
            source = children[0].type.get_canonical()
            pointer = cindex.TypeKind.POINTER
            if (target.kind == pointer and source.kind == pointer
                    and target.get_pointee().kind == cindex.TypeKind.VOID
                    and source.get_pointee().kind != cindex.TypeKind.VOID):
                start, end = node.extent.start, node.extent.end
                if start.file is not None:
                    path = Path(str(start.file)).resolve()
                    if path.is_relative_to(repo):
                        found.add(PointerErasure(
                            path, start.offset, end.offset, start.line, start.column,
                            children[0].type.spelling, node.type.spelling,
                        ))
        for child in children:
            visit(child)

    visit(tu.cursor)
    return tuple(sorted(found))
