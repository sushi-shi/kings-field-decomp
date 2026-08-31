"""Incoming/outgoing reference lists and confidence-safe call trees."""

from __future__ import annotations

import argparse

from scripts.kf.sema import Context, die, resolve_binding
from scripts.kf.sema.evidence import Reference
from scripts.kf.sema.index import Binding


TRAVERSABLE_KINDS = {"call", "tail", "jump", "branch"}
TRAVERSABLE_TIERS = {"proven", "validated"}


def _endpoint(ctx: Context, reference: Reference, *, outgoing: bool) -> Binding | None:
    va = reference.target if outgoing else reference.owner
    if va is None:
        return None
    return ctx.idx.function(va) or ctx.idx.function_owner(va) or ctx.idx.data_owner(va)


def _row(ctx: Context, reference: Reference, *, outgoing: bool, raw: bool) -> dict[str, object]:
    endpoint = _endpoint(ctx, reference, outgoing=outgoing)
    result = {
        **reference.as_dict(),
        "site_label": ctx.idx.label(reference.site),
        "target_label": (
            ctx.idx.label(reference.target)
            if reference.target is not None else None
        ),
        "endpoint": endpoint.as_dict() if endpoint else None,
    }
    if not raw:
        result.pop("origins")
    return result


def _references(
    ctx: Context,
    binding: Binding,
    *,
    outgoing: bool,
    confirmed_only: bool,
) -> tuple[Reference, ...]:
    return (
        ctx.refs.outgoing(binding, confirmed_only=confirmed_only)
        if outgoing
        else ctx.refs.incoming(binding, confirmed_only=confirmed_only)
    )


def _tree(
    ctx: Context,
    binding: Binding,
    *,
    outgoing: bool,
    confirmed_only: bool,
    raw: bool,
    depth: int,
    seen: frozenset[int] = frozenset(),
) -> dict[str, object]:
    cycle = binding.va in seen
    node: dict[str, object] = {
        "binding": binding.as_dict(),
        "cycle": cycle,
        "children": [],
    }
    if cycle or depth == 0:
        return node
    seen = seen | {binding.va}
    children = []
    for reference in _references(
        ctx,
        binding,
        outgoing=outgoing,
        confirmed_only=confirmed_only,
    ):
        endpoint = _endpoint(ctx, reference, outgoing=outgoing)
        traversable = (
            endpoint is not None
            and endpoint.space == "text"
            and reference.kind in TRAVERSABLE_KINDS
            and reference.tier in TRAVERSABLE_TIERS
        )
        child: dict[str, object] = {
            "reference": _row(ctx, reference, outgoing=outgoing, raw=raw),
            "traversed": traversable,
            "node": None,
        }
        if traversable and endpoint is not None:
            child["node"] = _tree(
                ctx,
                endpoint,
                outgoing=outgoing,
                confirmed_only=confirmed_only,
                raw=raw,
                depth=depth - 1,
                seen=seen,
            )
        children.append(child)
    node["children"] = children
    return node


def _render_reference(ctx: Context, row: dict[str, object], *, outgoing: bool) -> str:
    if outgoing:
        endpoint = row["target_label"] or "?"
        arrow = "->"
    else:
        endpoint_row = row["endpoint"]
        endpoint = endpoint_row["name"] if endpoint_row else row["site_label"]
        arrow = "<-"
    paired = (
        f" / 0x{row['paired_site']:08x}" if row["paired_site"] is not None else ""
    )
    return (
        f"{arrow} {endpoint:<34} @ 0x{row['site']:08x}{paired} "
        f"[{row['tier']}] {row['kind']}"
    )


def _render_tree(
    ctx: Context,
    node: dict[str, object],
    *,
    outgoing: bool,
    prefix: str = "",
) -> list[str]:
    binding = node["binding"]
    marker = " (cycle)" if node["cycle"] else ""
    lines = [f"{prefix}0x{binding['va']:08x} {binding['name']}{marker}"]
    children = node["children"]
    for position, child in enumerate(children):
        last = position == len(children) - 1
        joint = "└─ " if last else "├─ "
        continuation = "   " if last else "│  "
        reference = child["reference"]
        lines.append(
            prefix + joint
            + _render_reference(ctx, reference, outgoing=outgoing)
            + (" [not traversed]" if not child["traversed"] else "")
        )
        nested = child["node"]
        if nested is not None:
            nested_lines = _render_tree(
                ctx,
                nested,
                outgoing=outgoing,
                prefix=prefix + continuation,
            )
            lines.extend(nested_lines[1:])
    return lines


def main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema xref")
    parser.add_argument("target")
    parser.add_argument("--callees", action="store_true")
    parser.add_argument("--tree", action="store_true")
    parser.add_argument("--depth", type=int, default=3)
    parser.add_argument("--raw", action="store_true")
    parser.add_argument("--confirmed-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    if args.depth < 0:
        die("--depth must be non-negative")
    _va, binding = resolve_binding(ctx, args.target)
    if binding is None:
        die(f"{args.target!r} has no admitted binding")
    if args.callees and binding.space != "text":
        die("--callees requires a function")

    direction = "outgoing" if args.callees else "incoming"
    if args.tree:
        result = _tree(
            ctx,
            binding,
            outgoing=args.callees,
            confirmed_only=args.confirmed_only,
            raw=args.raw,
            depth=args.depth,
        )
        if args.json:
            ctx.print_json(f"xref-{direction}-tree", result)
        else:
            print(
                f"{direction.title()} reference tree ({ctx.image}; "
                f"depth={args.depth})"
            )
            print("\n".join(_render_tree(ctx, result, outgoing=args.callees)))
        return 0

    references = _references(
        ctx,
        binding,
        outgoing=args.callees,
        confirmed_only=args.confirmed_only,
    )
    result = {
        "binding": binding.as_dict(),
        "direction": direction,
        "references": [
            _row(ctx, row, outgoing=args.callees, raw=args.raw)
            for row in references
        ],
    }
    if args.json:
        ctx.print_json(f"xref-{direction}", result)
    else:
        print(
            f"{direction.title()} references to {binding.name} "
            f"({ctx.image}; {len(references)} row(s))"
        )
        for row in result["references"]:
            print("  " + _render_reference(ctx, row, outgoing=args.callees))
            if args.raw:
                for origin in row["origins"]:
                    print(
                        f"      origin {origin['channel']} {origin['confidence']} "
                        f"({origin['tier']}) {origin['reason'] or ''}"
                    )
    return 0
