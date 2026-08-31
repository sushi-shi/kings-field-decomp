"""Selected-image memory map, coverage gaps, units, and symbol search."""

from __future__ import annotations

import argparse
from collections import Counter

from scripts.kf.sema import Context, die, parse_number, resolve_one
def _merge(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    merged: list[tuple[int, int]] = []
    for start, end in sorted(intervals):
        if not merged or start > merged[-1][1]:
            merged.append((start, end))
        else:
            merged[-1] = merged[-1][0], max(merged[-1][1], end)
    return merged


def _uncovered(ctx: Context) -> list[dict[str, int]]:
    intervals = [
        (max(row.va, ctx.img.load_start), min(row.end, ctx.img.load_end))
        for row in (*ctx.idx.functions, *ctx.idx.data)
        if row.end > ctx.img.load_start and row.va < ctx.img.load_end
    ]
    cursor = ctx.img.load_start
    gaps = []
    for start, end in _merge(intervals):
        if cursor < start:
            gaps.append({"va": cursor, "size": start - cursor})
        cursor = max(cursor, end)
    if cursor < ctx.img.load_end:
        gaps.append({"va": cursor, "size": ctx.img.load_end - cursor})
    return gaps


def overview(ctx: Context) -> dict[str, object]:
    gaps = _uncovered(ctx)
    functions = ctx.idx.functions
    data = ctx.idx.data
    return {
        "entry": ctx.img.layout.entry,
        "load_start": ctx.img.load_start,
        "load_end": ctx.img.load_end,
        "load_size": ctx.img.layout.load_size,
        "function_count": len(functions),
        "function_body_bytes": sum(row.body_size for row in functions),
        "function_extent_bytes": sum(row.size for row in functions),
        "data_count": len(data),
        "data_bytes": sum(row.size for row in data),
        "data_kinds": dict(sorted(Counter(row.kind for row in data).items())),
        "vendored_functions": sum(row.vendored for row in functions),
        "named_functions": sum(not row.generated for row in functions),
        "unit_functions": sum(bool(row.unit) for row in functions),
        "uncovered": gaps,
        "uncovered_bytes": sum(row["size"] for row in gaps),
        "unclassified": [
            row.as_dict() for row in data if row.kind == "unclassified"
        ],
    }


def _range(ctx: Context, start: int, end: int) -> list[dict[str, object]]:
    if end <= start:
        die("range end must be greater than start")
    return [
        row.as_dict()
        for row in sorted((*ctx.idx.functions, *ctx.idx.data), key=lambda item: item.va)
        if row.va < end and row.end > start
    ]


def _find(ctx: Context, query: str) -> list[dict[str, object]]:
    needle = query.casefold()
    rows = []
    for binding in (*ctx.idx.functions, *ctx.idx.data):
        values = (
            binding.name,
            binding.curated_name,
            binding.note,
            binding.provenance,
            binding.provider,
            binding.library,
            binding.unit,
            *(alias.name for alias in binding.aliases),
        )
        if any(needle in value.casefold() for value in values if value):
            rows.append(binding.as_dict())
    return rows


def _units(ctx: Context) -> list[dict[str, object]]:
    return [
        {
            "unit": name,
            "va": rows[0].va,
            "end": max(row.end for row in rows),
            "functions": [row.as_dict() for row in rows],
        }
        for name, rows in ctx.idx.units().items()
    ]


def _format_binding(row: dict[str, object]) -> str:
    return (
        f"0x{row['va']:08x}..0x{row['va'] + row['size']:08x} "
        f"{row['space']:<4} {row['kind']:<15} {row['name']}"
    )


def _render_overview(ctx: Context, row: dict[str, object]) -> str:
    kinds = ", ".join(f"{name}={count}" for name, count in row["data_kinds"].items())
    return "\n".join([
        f"Memory map {ctx.image}",
        f"  load: 0x{row['load_start']:08x}..0x{row['load_end']:08x} "
        f"(0x{row['load_size']:x} B); entry=0x{row['entry']:08x}",
        f"  functions: {row['function_count']} (named={row['named_functions']}, "
        f"vendored={row['vendored_functions']}, units={row['unit_functions']})",
        f"  function bytes: body=0x{row['function_body_bytes']:x}, "
        f"extent=0x{row['function_extent_bytes']:x}",
        f"  data: {row['data_count']} row(s), 0x{row['data_bytes']:x} B ({kinds})",
        f"  inventory holes: {len(row['uncovered'])} / 0x{row['uncovered_bytes']:x} B",
        f"  admitted unclassified regions: {len(row['unclassified'])}",
    ])


def main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema map")
    parser.add_argument(
        "mode",
        nargs="?",
        choices=("overview", "at", "range", "gaps", "units", "find"),
        default="overview",
    )
    parser.add_argument("query", nargs="*")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    view = f"map-{args.mode}"
    if args.mode == "overview":
        if args.query:
            die("map overview accepts no query")
        result: object = overview(ctx)
    elif args.mode == "at":
        if len(args.query) != 1:
            die("map at requires one address or symbol")
        va = resolve_one(ctx, args.query[0])
        binding = ctx.idx.covering(va)
        result = {
            "va": va,
            "label": ctx.idx.label(va),
            "binding": binding.as_dict() if binding else None,
        }
    elif args.mode == "range":
        if len(args.query) != 2:
            die("map range requires START END")
        result = _range(ctx, parse_number(args.query[0]), parse_number(args.query[1]))
    elif args.mode == "gaps":
        if args.query:
            die("map gaps accepts no query")
        result = {
            "inventory_holes": _uncovered(ctx),
            "unclassified": [
                row.as_dict() for row in ctx.idx.data if row.kind == "unclassified"
            ],
        }
    elif args.mode == "units":
        if args.query:
            die("map units accepts no query")
        result = _units(ctx)
    else:
        if len(args.query) != 1:
            die("map find requires one search string")
        result = _find(ctx, args.query[0])

    if args.json:
        ctx.print_json(view, result)
        return 0
    if args.mode == "overview":
        assert isinstance(result, dict)
        print(_render_overview(ctx, result))
    elif args.mode == "at":
        assert isinstance(result, dict)
        print(f"0x{result['va']:08x} {result['label']}")
        if result["binding"]:
            print("  " + _format_binding(result["binding"]))
        else:
            print("  (no admitted binding)")
    elif args.mode == "gaps":
        assert isinstance(result, dict)
        print(f"Inventory holes ({len(result['inventory_holes'])}):")
        for row in result["inventory_holes"]:
            print(f"  0x{row['va']:08x} +0x{row['size']:x}")
        print(f"Admitted unclassified regions ({len(result['unclassified'])}):")
        for row in result["unclassified"]:
            print("  " + _format_binding(row))
    elif args.mode == "units":
        assert isinstance(result, list)
        print(f"Reconstruction units ({len(result)}):")
        for row in result:
            print(
                f"  0x{row['va']:08x}..0x{row['end']:08x} "
                f"{row['unit']} ({len(row['functions'])} function(s))"
            )
    else:
        assert isinstance(result, list)
        print(f"Map rows ({len(result)}):")
        for row in result:
            print("  " + _format_binding(row))
    return 0
