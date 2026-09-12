"""Expose C's implicit void-pointer conversions to the auxiliary C++ checker.

Only Clang's target-C AST determines conversion sites. The generated VFS view
adds explicit conversions at those sites, then the ordinary C++ compiler must
accept the entire translation unit. No diagnostic is suppressed.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
import json
from pathlib import Path

from scripts.kf.clangd import unit_arguments
from scripts.kf.manifest import Unit


@dataclass(frozen=True, order=True)
class VoidConversion:
    file: Path
    start: int
    end: int
    target: str


def void_conversions(unit: Unit, repo: Path, sdk: Path) -> tuple[VoidConversion, ...]:
    """Find valid implicit void* -> object-pointer conversions in project C."""
    from clang import cindex

    args = unit_arguments(unit, repo, "clang", sdk, mode="retail")
    args = args[1:args.index("-c")]
    tu = cindex.Index.create().parse(str(repo / unit.source), args=args)
    errors = [str(d) for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        raise ValueError("target-C compatibility parse failed:\n" + "\n".join(errors))
    conversions: set[VoidConversion] = set()
    pointer = cindex.TypeKind.POINTER
    function_types = (cindex.TypeKind.FUNCTIONPROTO, cindex.TypeKind.FUNCTIONNOPROTO)

    def visit(node) -> None:
        if node.location.file is not None:
            path = Path(str(node.location.file)).resolve()
            if not path.is_relative_to(repo):
                return
        children = tuple(node.get_children())
        if node.kind == cindex.CursorKind.UNEXPOSED_EXPR and len(children) == 1:
            target = node.type.get_canonical()
            source = children[0].type.get_canonical()
            if target.kind == pointer and source.kind == pointer:
                to = target.get_pointee()
                origin = source.get_pointee()
                if (origin.kind == cindex.TypeKind.VOID
                        and to.kind not in (*function_types, cindex.TypeKind.VOID)
                        and (not origin.is_const_qualified() or to.is_const_qualified())
                        and (not origin.is_volatile_qualified() or to.is_volatile_qualified())):
                    start, end = node.extent.start, node.extent.end
                    if (start.file is not None and end.file == start.file
                            and start.offset < end.offset):
                        path = Path(str(start.file)).resolve()
                        if path.is_relative_to(repo):
                            conversions.add(VoidConversion(
                                path, start.offset, end.offset, node.type.spelling,
                            ))
        for child in children:
            visit(child)

    visit(tu.cursor)
    return tuple(sorted(conversions))


def converted_source(source: bytes, conversions: tuple[VoidConversion, ...]) -> bytes:
    """Wrap expression spans, rejecting ambiguous macro expansions or overlaps."""
    ordered = sorted(set(conversions), key=lambda c: (c.start, -c.end, c.target))
    active: list[VoidConversion] = []
    inserts: dict[int, list[bytes]] = defaultdict(list)
    for conversion in ordered:
        if not 0 <= conversion.start < conversion.end <= len(source):
            raise ValueError("void conversion is outside its source file")
        while active and conversion.start >= active[-1].end:
            active.pop()
        if active:
            previous = active[-1]
            if conversion.end > previous.end:
                raise ValueError("overlapping void conversion expressions")
            if (conversion.start, conversion.end) == (previous.start, previous.end):
                raise ValueError("ambiguous void conversion target in macro expansion")
        active.append(conversion)
        inserts[conversion.start].append(f"({conversion.target})(".encode())
        inserts[conversion.end].insert(0, b")")
    for offset in sorted(inserts, reverse=True):
        source = source[:offset] + b"".join(inserts[offset]) + source[offset:]
    return source


def write_overlay(
    conversions: tuple[VoidConversion, ...], repo: Path, output: Path,
) -> Path:
    """Write only generated checker inputs; the game source remains target C."""
    grouped: dict[Path, list[VoidConversion]] = defaultdict(list)
    for conversion in conversions:
        grouped[conversion.file].append(conversion)
    roots = []
    output.mkdir(parents=True, exist_ok=True)
    for path, sites in sorted(grouped.items()):
        target = output / "source" / path.relative_to(repo)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(converted_source(path.read_bytes(), tuple(sites)))
        roots.append({"type": "file", "name": str(path), "external-contents": str(target)})
    overlay = output / "overlay.json"
    overlay.write_text(json.dumps({"version": 0, "roots": roots}), encoding="utf-8")
    return overlay
