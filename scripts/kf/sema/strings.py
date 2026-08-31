"""Curated retail strings and the confidence-tagged references to them."""

from __future__ import annotations

import argparse
from dataclasses import dataclass

from scripts.kf.sema import Context, die, resolve_binding
from scripts.kf.sema.index import Binding


@dataclass(frozen=True)
class DecodedString:
    binding: Binding
    text: str
    encoding: str
    terminated: bool

    def as_dict(self) -> dict[str, object]:
        return {
            "binding": self.binding.as_dict(),
            "text": self.text,
            "encoding": self.encoding,
            "terminated": self.terminated,
        }


def decode_string(ctx: Context, binding: Binding) -> DecodedString:
    if binding.space != "data" or binding.kind != "string":
        raise ValueError(f"{binding.name} is not a curated string")
    payload = ctx.img.require(binding.va, binding.size)
    nul = payload.find(b"\0")
    terminated = nul >= 0
    content = payload[:nul] if terminated else payload
    encoding = "shift_jis" if binding.datatype == "shift_jis" else "ascii"
    text = content.decode(encoding, errors="replace")
    return DecodedString(binding, text, encoding, terminated)


def all_strings(ctx: Context) -> tuple[DecodedString, ...]:
    return tuple(
        decode_string(ctx, binding)
        for binding in ctx.idx.data
        if binding.kind == "string"
    )


def _reference_row(ctx: Context, reference) -> dict[str, object]:
    owner = ctx.idx.function_owner(reference.site) or ctx.idx.data_owner(reference.site)
    return {
        **reference.as_dict(),
        "site_label": ctx.idx.label(reference.site),
        "owner_name": owner.name if owner else None,
        "owner_va": owner.va if owner else None,
    }


def _entry(
    ctx: Context,
    decoded: DecodedString,
    *,
    confirmed_only: bool,
) -> dict[str, object]:
    references = ctx.refs.incoming(
        decoded.binding,
        confirmed_only=confirmed_only,
    )
    return {
        **decoded.as_dict(),
        "references": [_reference_row(ctx, row) for row in references],
        "reference_counts": {
            tier: sum(row.tier == tier for row in references)
            for tier in ("proven", "validated", "candidate")
        },
    }


def _ranked_functions(
    ctx: Context,
    strings: tuple[DecodedString, ...],
    *,
    confirmed_only: bool,
) -> list[dict[str, object]]:
    by_owner: dict[int, dict[str, object]] = {}
    for decoded in strings:
        for reference in ctx.refs.incoming(
            decoded.binding,
            confirmed_only=confirmed_only,
        ):
            owner = ctx.idx.function_owner(reference.site)
            if owner is None:
                continue
            row = by_owner.setdefault(owner.va, {
                "function": owner.as_dict(),
                "references": 0,
                "strings": set(),
                "proven": 0,
                "validated": 0,
                "candidate": 0,
            })
            row["references"] = int(row["references"]) + 1
            cast_strings = row["strings"]
            assert isinstance(cast_strings, set)
            cast_strings.add(decoded.binding.va)
            row[reference.tier] = int(row[reference.tier]) + 1
    result = []
    for row in by_owner.values():
        values = row.pop("strings")
        assert isinstance(values, set)
        row["string_count"] = len(values)
        result.append(row)
    return sorted(
        result,
        key=lambda row: (
            -int(row["proven"]),
            -int(row["validated"]),
            -int(row["references"]),
            int(row["function"]["va"]),
        ),
    )


def _render_entry(row: dict[str, object]) -> list[str]:
    binding = row["binding"]
    assert isinstance(binding, dict)
    text = str(row["text"]).replace("\n", "\\n").replace("\r", "\\r")
    lines = [
        f"0x{binding['va']:08x}  {row['encoding']:<9}  {text!r}",
    ]
    references = row["references"]
    assert isinstance(references, list)
    for reference in references:
        owner = reference["owner_name"] or "<unowned>"
        lines.append(
            f"  <- 0x{reference['site']:08x} {owner:<28} "
            f"[{reference['tier']}] {reference['kind']}"
        )
    if not references:
        lines.append("  <- (no visible references)")
    return lines


def main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema strings")
    parser.add_argument("target", nargs="?")
    parser.add_argument("--find")
    parser.add_argument("--ranked", action="store_true")
    parser.add_argument("--confirmed-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    rows = all_strings(ctx)
    if args.target:
        _va, binding = resolve_binding(ctx, args.target)
        if binding is None:
            die(f"{args.target!r} has no admitted binding")
        if binding.space == "text":
            referenced = {
                reference.target
                for reference in ctx.refs.outgoing(
                    binding,
                    confirmed_only=args.confirmed_only,
                )
                if reference.target is not None
            }
            rows = tuple(row for row in rows if row.binding.va in referenced)
        elif binding.kind == "string":
            rows = tuple(row for row in rows if row.binding.va == binding.va)
        else:
            die(f"{binding.name} is neither a function nor a curated string")
    if args.find:
        needle = args.find.casefold()
        rows = tuple(row for row in rows if needle in row.text.casefold())

    if args.ranked:
        result = _ranked_functions(
            ctx,
            rows,
            confirmed_only=args.confirmed_only,
        )
        if args.json:
            ctx.print_json("strings-ranked", result)
            return 0
        print(f"String-reference ranking ({ctx.image}; {len(result)} function(s))")
        for row in result:
            function = row["function"]
            print(
                f"  0x{function['va']:08x} {function['name']:<30} "
                f"strings={row['string_count']:<3} refs={row['references']:<3} "
                f"P/V/C={row['proven']}/{row['validated']}/{row['candidate']}"
            )
        return 0

    result = [
        _entry(ctx, row, confirmed_only=args.confirmed_only)
        for row in rows
    ]
    if args.json:
        ctx.print_json("strings", result)
    else:
        print(f"Strings ({ctx.image}; {len(result)} row(s))")
        for position, row in enumerate(result):
            if position:
                print()
            print("\n".join(_render_entry(row)))
    return 0
