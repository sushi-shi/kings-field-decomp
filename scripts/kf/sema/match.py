"""Read-only per-function reconstruction and objdiff score lookup."""

from __future__ import annotations

import argparse
from functools import lru_cache

from scripts.kf.progress import LOOSE_EXACT, Current, current_state
from scripts.kf.sema import Context, die, resolve_binding
from scripts.kf.sema.index import Binding


@lru_cache(maxsize=3)
def score_rows(image: str) -> tuple[tuple[Current, ...], tuple[str, ...]]:
    _manifest, _universe, rows, failures = current_state((image,))
    return tuple(rows), tuple(failures)


def score_for(binding: Binding) -> Current | None:
    rows, _failures = score_rows(binding.image)
    return next((row for row in rows if row.target.va == binding.va), None)


def _row(current: Current, *, loose: bool) -> dict[str, object]:
    threshold = LOOSE_EXACT if loose else 100.0
    return {
        "image": current.target.image,
        "va": current.target.va,
        "name": current.target.name,
        "code_size": current.target.code_size,
        "unit": current.unit.unit,
        "source": str(current.unit.source),
        "profile": current.unit.profile,
        "compiled": current.compiled,
        "scored": current.scored,
        "percent": current.pct,
        "exact": current.pct is not None and current.pct >= threshold,
        "exact_threshold": threshold,
    }


def _render(row: dict[str, object]) -> str:
    percent = "not scored" if row["percent"] is None else f"{row['percent']:.9f}%"
    exact = " exact" if row["exact"] else ""
    return (
        f"0x{row['va']:08x} {row['name']:<30} {percent:<16}{exact:<6} "
        f"{row['unit']} ({row['source']})"
    )


def main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema match")
    parser.add_argument("target", nargs="?")
    parser.add_argument("--worst", type=int)
    parser.add_argument("--all", action="store_true")
    parser.add_argument("--loose", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    if args.worst is not None and args.worst < 1:
        die("--worst must be positive")

    current, failures = score_rows(ctx.image)
    selected = list(current)
    if args.target:
        hits = [row for row in current if row.unit.unit == args.target]
        if not hits:
            _va, binding = resolve_binding(ctx, args.target)
            if binding is None or binding.space != "text":
                die(f"{args.target!r} is not a reconstruction function or unit")
            hits = [row for row in current if row.target.va == binding.va]
        if not hits:
            die(f"{args.target!r} has no reconstruction unit")
        selected = hits
    elif not args.all:
        selected = [row for row in selected if row.scored]

    selected.sort(key=lambda row: (
        101.0 if row.pct is None else row.pct,
        row.target.va,
    ))
    if args.worst is not None:
        selected = selected[:args.worst]
    result = {
        "rows": [_row(row, loose=args.loose) for row in selected],
        "failures": list(failures),
    }
    if args.json:
        ctx.print_json("match", result)
    else:
        print(f"Match state ({ctx.image}; {len(selected)} unit(s))")
        for row in result["rows"]:
            print("  " + _render(row))
        for failure in failures:
            print(f"  WARNING: {failure}")
    return 0
