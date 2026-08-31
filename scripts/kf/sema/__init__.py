"""Read-only semantic navigation over one King's Field retail executable.

Usage:
    kf sema --image game addr 0x80014fa8
    kf sema --image game disasm 0x80014fa8 --blocks
    kf sema --image game cfg 0x80014fa8 --format json
    kf sema --image game xref 0x80014268 --callees
    kf sema --image game strings --find "DEBUG STOP"
    kf sema --image game -
"""

from __future__ import annotations

import argparse
import importlib
import json
import shlex
import sys
from dataclasses import dataclass
from functools import cached_property

from scripts.kf.delink import image_key
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.sema.evidence import Evidence, evidence
from scripts.kf.sema.image import RetailImage, retail
from scripts.kf.sema.index import Binding, Index, index


IMAGE_ALIASES = {image_key(image): image for image in IMAGE_LAYOUTS}
SUBCOMMANDS = {
    "addr": ("scripts.kf.sema.addr", "main"),
    "rva": ("scripts.kf.sema.addr", "rva_main"),
    "disasm": ("scripts.kf.sema.disasm", "main"),
    "cfg": ("scripts.kf.sema.disasm", "cfg_main"),
    "dump": ("scripts.kf.sema.disasm", "dump_main"),
    "xref": ("scripts.kf.sema.xref", "main"),
    "strings": ("scripts.kf.sema.strings", "main"),
    "map": ("scripts.kf.sema.map", "main"),
    "match": ("scripts.kf.sema.match", "main"),
}


class SemaError(Exception):
    """Bad query or missing prerequisite; maps to return code 2."""


@dataclass
class Context:
    image: str

    @cached_property
    def idx(self) -> Index:
        return index(self.image)

    @cached_property
    def img(self) -> RetailImage:
        return retail(self.image)

    @cached_property
    def refs(self) -> Evidence:
        return evidence(self.image)

    @property
    def key(self) -> str:
        return image_key(self.image)

    def document(self, view: str, result: object) -> dict[str, object]:
        return {
            "schema": "kf-sema-v1",
            "image": self.image,
            "view": view,
            "result": result,
        }

    def print_json(self, view: str, result: object) -> None:
        print(json.dumps(self.document(view, result), indent=2, sort_keys=True))


def die(message: str) -> None:
    raise SemaError(message)


def resolve_one(ctx: Context, token: str) -> int:
    hits = ctx.idx.resolve(token)
    if not hits:
        die(f"{ctx.image}: {token!r} resolves to no address or binding")
    if len(hits) > 1:
        rows = "\n".join(f"  0x{va:08x} {ctx.idx.label(va)}" for va in hits[:12])
        more = f"\n  ... (+{len(hits) - 12} more)" if len(hits) > 12 else ""
        die(f"{ctx.image}: {token!r} is ambiguous:\n{rows}{more}")
    return hits[0]


def resolve_binding(ctx: Context, token: str) -> tuple[int, Binding | None]:
    va = resolve_one(ctx, token)
    return va, ctx.idx.function(va) or ctx.idx.datum(va) or ctx.idx.covering(va)


def parse_number(value: str) -> int:
    try:
        return int(value, 0)
    except ValueError:
        try:
            return int(value, 16)
        except ValueError:
            die(f"{value!r} is not an integer or hexadecimal address")


def dispatch(ctx: Context, argv: list[str]) -> int:
    if not argv or argv[0] in {"-h", "--help", "help"}:
        print(__doc__.strip())
        return 0 if argv else 2
    command, rest = argv[0], argv[1:]
    target = SUBCOMMANDS.get(command)
    if target is None:
        die(
            f"unknown semantic view {command!r}; have "
            + ", ".join(sorted(SUBCOMMANDS))
        )
    module_name, function_name = target
    function = getattr(importlib.import_module(module_name), function_name)
    return int(function(ctx, rest) or 0)


def batch(ctx: Context) -> int:
    result = 0
    for line in sys.stdin:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        print(f"== kf sema --image {ctx.key} {line}")
        result = dispatch(ctx, shlex.split(line)) or result
    return result


def main(argv: list[str] | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--image", choices=tuple(IMAGE_ALIASES), required=False)
    args, rest = parser.parse_known_args(argv)
    if not rest or rest[0] in {"-h", "--help", "help"}:
        print(__doc__.strip())
        return 0 if rest else 2
    if args.image is None:
        print("kf sema: --image psx|game|open is required", file=sys.stderr)
        return 2
    ctx = Context(IMAGE_ALIASES[args.image])
    try:
        return batch(ctx) if rest == ["-"] else dispatch(ctx, rest)
    except (OSError, RuntimeError, ValueError, SemaError) as error:
        print(f"kf sema: {error}", file=sys.stderr)
        return 2
