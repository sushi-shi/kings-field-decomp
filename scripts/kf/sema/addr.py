"""Address and RVA dossiers for the selected retail executable."""

from __future__ import annotations

import argparse

from scripts.kf.sema import Context, die, parse_number, resolve_one
from scripts.kf.sema.match import score_for
from scripts.kf.sema.strings import decode_string


def _counts(rows) -> dict[str, int]:
    return {
        tier: sum(row.tier == tier for row in rows)
        for tier in ("proven", "validated", "candidate")
    }


def dossier(
    ctx: Context,
    va: int,
    *,
    confirmed_only: bool = False,
) -> dict[str, object]:
    binding = ctx.idx.covering(va)
    function = ctx.idx.function_owner(va)
    incoming = ctx.refs.incoming(
        binding or va,
        confirmed_only=confirmed_only,
    )
    outgoing = (
        ctx.refs.outgoing(function, confirmed_only=confirmed_only)
        if function else ()
    )
    result: dict[str, object] = {
        "va": va,
        "rva": va - ctx.img.load_start,
        "region": ctx.img.region(va),
        "file_offset": (
            ctx.img.layout.file_offset(va) if ctx.img.contains(va) else None
        ),
        "label": ctx.idx.label(va),
        "binding": binding.as_dict() if binding else None,
        "offset": va - binding.va if binding else None,
        "in_body": function.contains_body(va) if function else None,
        "incoming_counts": _counts(incoming),
        "outgoing_counts": _counts(outgoing),
        "incoming": [row.as_dict() for row in incoming],
        "outgoing": [row.as_dict() for row in outgoing],
    }
    if binding is not None and binding.kind == "string":
        result["string"] = decode_string(ctx, binding).as_dict()
    if function is not None and function.unit:
        current = score_for(function)
        result["match"] = None if current is None else {
            "unit": current.unit.unit,
            "source": str(current.unit.source),
            "profile": current.unit.profile,
            "compiled": current.compiled,
            "scored": current.scored,
            "percent": current.pct,
            "exact": current.pct == 100.0,
        }
    return result


def _render(ctx: Context, row: dict[str, object]) -> str:
    lines = [
        f"{ctx.image}  VA 0x{row['va']:08x}  RVA 0x{row['rva']:x}",
        f"label: {row['label']}",
        f"region: {row['region']}"
        + (
            f"  file+0x{row['file_offset']:x}"
            if row["file_offset"] is not None else ""
        ),
    ]
    binding = row["binding"]
    if binding is None:
        lines.append("binding: (none)")
    else:
        lines.extend([
            f"binding: {binding['space']} {binding['kind']} {binding['name']}",
            f"extent: 0x{binding['va']:08x}..0x{binding['va'] + binding['size']:08x} "
            f"(+0x{row['offset']:x})",
            f"evidence: {binding['confidence']} — {binding['provenance']}",
        ])
        if binding["space"] == "text":
            lines.append(
                f"body: 0x{binding['body_size']:x} B; fragments={binding['fragments']}; "
                f"in_body={row['in_body']}"
            )
            if binding["return_type"] or binding["parameters"]:
                parameters = (binding["parameters"] or "").replace(";", ", ")
                signature = (
                    f"{binding['return_type'] or 'unknown'} {binding['name']}"
                    f"({parameters or 'void'})"
                )
                lines.append(
                    f"identity: {signature} "
                    f"[{binding['signature_confidence'] or 'unreviewed'}]"
                )
            if binding["owner_type"]:
                lines.append(
                    f"method: owner={binding['owner_type']} action={binding['action']}"
                )
        elif binding["datatype"] or binding["owner_type"]:
            details = []
            if binding["datatype"]:
                details.append(f"type={binding['datatype']}")
            if binding["owner_type"]:
                details.append(f"owner={binding['owner_type']}")
            lines.append("identity: " + " ".join(details))
        if binding["link_name"] != binding["name"]:
            lines.append(f"link name: {binding['link_name']}")
        if binding["provider"]:
            lines.append(
                f"vendored: {binding['provider']} / {binding['library'] or '?'}"
            )
        if binding["unit"]:
            lines.append(f"unit: {binding['unit']}")
    if row.get("string"):
        string = row["string"]
        lines.append(f"string ({string['encoding']}): {string['text']!r}")
    if row.get("match"):
        match = row["match"]
        pct = "not scored" if match["percent"] is None else f"{match['percent']:.9f}%"
        lines.append(f"match: {pct} ({match['unit']}; {match['source']})")
    inc = row["incoming_counts"]
    out = row["outgoing_counts"]
    lines.append(
        f"references: incoming P/V/C={inc['proven']}/{inc['validated']}/{inc['candidate']}; "
        f"outgoing P/V/C={out['proven']}/{out['validated']}/{out['candidate']}"
    )
    return "\n".join(lines)


def _main(ctx: Context, argv: list[str], *, rva: bool) -> int:
    parser = argparse.ArgumentParser(prog=f"kf sema {'rva' if rva else 'addr'}")
    parser.add_argument("target")
    parser.add_argument("--confirmed-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    if rva:
        offset = parse_number(args.target)
        if offset < 0:
            die("RVA must be non-negative")
        va = ctx.img.load_start + offset
    else:
        va = resolve_one(ctx, args.target)
    result = dossier(ctx, va, confirmed_only=args.confirmed_only)
    if args.json:
        ctx.print_json("rva" if rva else "addr", result)
    else:
        print(_render(ctx, result))
    return 0


def main(ctx: Context, argv: list[str]) -> int:
    return _main(ctx, argv, rva=False)


def rva_main(ctx: Context, argv: list[str]) -> int:
    return _main(ctx, argv, rva=True)
