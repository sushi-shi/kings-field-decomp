"""Annotated MIPS disassembly, basic blocks, CFG, and raw target dumps."""

from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

from scripts.kf.sema import Context, die, parse_number, resolve_binding
from scripts.kf.sema.cfg import Graph, build_graph
from scripts.kf.sema.index import Binding
from scripts.kf.sema.mips import decode_control, load_delay_register


ROW = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(.*?)\s*$", re.I)
REGISTERS = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)


@dataclass(frozen=True)
class DisplayInstruction:
    va: int
    raw: bytes
    word: int
    text: str

    def as_dict(self, notes: list[str], block: int | None) -> dict[str, object]:
        return {
            "va": self.va,
            "bytes": self.raw.hex(),
            "word": self.word,
            "text": self.text,
            "notes": notes,
            "block": block,
        }


def decode_display(payload: bytes, va: int) -> tuple[DisplayInstruction, ...]:
    objdump = shutil.which("mipsel-linux-gnu-objdump")
    if objdump is None:
        raise RuntimeError("mipsel-linux-gnu-objdump not found; enter nix develop")
    with tempfile.TemporaryDirectory(prefix="kf-sema-") as directory:
        path = Path(directory) / "payload.bin"
        path.write_bytes(payload)
        process = subprocess.run(
            [
                objdump,
                "-D",
                "-z",
                "-b",
                "binary",
                "-m",
                "mips:3000",
                "-EL",
                f"--adjust-vma={va:#x}",
                str(path),
            ],
            capture_output=True,
            text=True,
            check=False,
        )
    if process.returncode:
        raise RuntimeError(process.stderr.strip() or "MIPS objdump failed")
    rows: list[DisplayInstruction] = []
    for line in process.stdout.splitlines():
        match = ROW.match(line)
        if match is None:
            continue
        address = int(match.group(1), 16)
        offset = address - va
        raw = payload[offset:offset + 4]
        if len(raw) != 4:
            continue
        rows.append(DisplayInstruction(
            address,
            raw,
            int.from_bytes(raw, "little"),
            re.sub(r"\s+", " ", match.group(3)).strip(),
        ))
    if not rows and payload:
        raise RuntimeError("MIPS objdump decoded no instructions")
    return tuple(rows)


def function_extent(
    ctx: Context,
    target: str,
    size: str | None,
    *,
    full: bool = False,
) -> tuple[int, int, Binding | None]:
    va, binding = resolve_binding(ctx, target)
    function = ctx.idx.function(va) or ctx.idx.function_owner(va)
    if size is not None:
        return va, parse_number(size), function or binding
    if function is None or function.space != "text":
        die(f"{ctx.image}: 0x{va:08x} starts no admitted function; pass --size")
    return function.va, function.size if full else function.body_size, function


def _notes(
    ctx: Context,
    row: DisplayInstruction,
    function: Binding | None,
    graph: Graph | None,
    *,
    confirmed_only: bool = False,
) -> list[str]:
    notes: list[str] = []
    control = decode_control(row.va, row.word)
    if control is not None:
        if control.target is not None:
            if function is not None and function.contains_body(control.target):
                notes.append(f"-> +0x{control.target - function.va:x}")
            else:
                notes.append(f"-> {ctx.idx.label(control.target)}")
        if control.kind in {"indirect-call", "indirect-jump"}:
            notes.append("unresolved indirect target")
    for reference in ctx.refs.at_site(row.va, confirmed_only=confirmed_only):
        if reference.target is None:
            continue
        label = reference.target_label(ctx.idx)
        text = f"{reference.tier} {reference.kind}: {label}"
        if text not in notes:
            notes.append(text)
    if graph is not None and row.va in graph.delay_slots:
        notes.append(f"delay slot of 0x{graph.delay_slots[row.va]:08x}")
    register = load_delay_register(row.word)
    if register is not None:
        notes.append(f"load delay: ${REGISTERS[register]} unavailable to next instruction")
    return notes


def _block_start(graph: Graph | None, va: int) -> int | None:
    if graph is None:
        return None
    block = next((item for item in graph.blocks if item.contains(va)), None)
    return block.start if block else None


def _listing_document(
    ctx: Context,
    va: int,
    size: int,
    binding: Binding | None,
    graph: Graph | None,
    *,
    confirmed_only: bool = False,
) -> dict[str, object]:
    payload = ctx.img.require(va, size)
    rows = decode_display(payload, va)
    return {
        "target": binding.as_dict() if binding else {"image": ctx.image, "va": va},
        "va": va,
        "size": size,
        "instructions": [
            row.as_dict(
                _notes(
                    ctx,
                    row,
                    binding,
                    graph,
                    confirmed_only=confirmed_only,
                ),
                _block_start(graph, row.va),
            )
            for row in rows
        ],
        "cfg": graph.as_dict() if graph else None,
    }


def _render_listing(document: dict[str, object], *, lite: bool, blocks: bool) -> str:
    target = document["target"]
    assert isinstance(target, dict)
    name = target.get("name") or f"0x{int(document['va']):08x}"
    lines = [
        f"== {name}  0x{int(document['va']):08x}  0x{int(document['size']):x} B"
    ]
    previous_block = None
    cfg = document.get("cfg")
    block_details = {
        int(block["start"]): block
        for block in cfg.get("blocks", [])
    } if isinstance(cfg, dict) else {}
    instructions = document["instructions"]
    assert isinstance(instructions, list)
    for row in instructions:
        block = row["block"]
        if blocks and block != previous_block:
            detail = block_details.get(block, {})
            predecessors = ", ".join(
                f"0x{value:08x}" for value in detail.get("predecessors", [])
            ) or "entry"
            flags = ", ".join(detail.get("flags", []))
            lines.append("")
            lines.append(
                f"block 0x{block:08x}  in: {predecessors}"
                + (f"  [{flags}]" if flags else "")
            )
            previous_block = block
        notes = "  ; " + "; ".join(row["notes"]) if row["notes"] else ""
        if lite:
            lines.append(f"    {row['text']}{notes}")
        else:
            raw = bytes.fromhex(row["bytes"]).hex(" ")
            lines.append(f"  {row['va']:08x}: {raw:<11} {row['text']}{notes}")
    return "\n".join(lines)


def _graph_document(
    ctx: Context,
    target: str,
    *,
    confirmed_only: bool = False,
) -> tuple[Graph, dict[str, object]]:
    va, _size, binding = function_extent(ctx, target, None)
    assert binding is not None
    payload = ctx.img.require(va, binding.body_size)
    graph = build_graph(binding, payload, ctx.idx)
    display = {row.va: row for row in decode_display(payload, va)}
    document = graph.as_dict()
    for block in document["blocks"]:
        for instruction in block["instructions"]:
            shown = display.get(instruction["va"])
            instruction["text"] = shown.text if shown else f".word 0x{instruction['word']:08x}"
            instruction["bytes"] = shown.raw.hex() if shown else instruction["word"].to_bytes(4, "little").hex()
            instruction["notes"] = _notes(
                ctx,
                shown or DisplayInstruction(
                    instruction["va"],
                    instruction["word"].to_bytes(4, "little"),
                    instruction["word"],
                    instruction["text"],
                ),
                binding,
                graph,
                confirmed_only=confirmed_only,
            )
    return graph, document


def _render_cfg_text(document: dict[str, object]) -> str:
    function = document["function"]
    lines = [
        f"CFG {function['name']}  0x{function['va']:08x}  "
        f"{len(document['blocks'])} block(s), {len(document['edges'])} edge(s)"
    ]
    for block in document["blocks"]:
        preds = ", ".join(f"0x{value:08x}" for value in block["predecessors"]) or "entry"
        flags = ", ".join(block["flags"])
        lines.append("")
        lines.append(
            f"block 0x{block['start']:08x}..0x{block['end']:08x}  in: {preds}"
            + (f"  [{flags}]" if flags else "")
        )
        for row in block["instructions"]:
            notes = "  ; " + "; ".join(row["notes"]) if row["notes"] else ""
            lines.append(f"  {row['va']:08x}: {row['text']}{notes}")
        for edge in block["successors"]:
            target = (
                f"0x{edge['target']:08x}" if edge["target"] is not None else "?"
            )
            lines.append(f"    -> {target}  {edge['kind']}")
    for issue in document["issues"]:
        lines.append(f"WARNING: {issue}")
    return "\n".join(lines)


def _render_cfg_dot(document: dict[str, object]) -> str:
    function = document["function"]
    lines = [f'digraph "{function["name"]}" {{', '  node [shape=box,fontname="monospace"];']
    blocks = {block["start"] for block in document["blocks"]}
    external: set[int] = set()
    for block in document["blocks"]:
        body = "\\l".join(
            f"{row['va']:08x}: {row['text']}".replace('"', '\\"')
            for row in block["instructions"]
        ) + "\\l"
        flags = ",".join(block["flags"])
        label = f"{block['start']:08x}" + (f" [{flags}]" if flags else "") + "\\l" + body
        lines.append(f'  b{block["start"]:08x} [label="{label}"];')
    for edge in document["edges"]:
        if edge["target"] is None:
            continue
        target_block = edge["target_block"]
        target = target_block if target_block is not None else edge["target"]
        if target not in blocks and target not in external:
            external.add(target)
            lines.append(f'  x{target:08x} [shape=ellipse,label="0x{target:08x}"];')
        source_id = f"b{edge['source']:08x}"
        target_id = f"b{target:08x}" if target in blocks else f"x{target:08x}"
        style = "dotted" if edge["kind"] == "call" else "solid"
        lines.append(
            f'  {source_id} -> {target_id} [label="{edge["kind"]}",style={style}];'
        )
    lines.append("}")
    return "\n".join(lines)


def main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema disasm")
    parser.add_argument("target")
    parser.add_argument("--size")
    parser.add_argument("--lite", action="store_true")
    parser.add_argument("--blocks", action="store_true")
    parser.add_argument("--confirmed-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    va, size, binding = function_extent(ctx, args.target, args.size)
    graph = None
    if args.blocks:
        if binding is None:
            die("--blocks requires an admitted function")
        graph = build_graph(binding, ctx.img.require(binding.va, binding.body_size), ctx.idx)
    document = _listing_document(
        ctx,
        va,
        size,
        binding,
        graph,
        confirmed_only=args.confirmed_only,
    )
    if args.json:
        ctx.print_json("disasm", document)
    else:
        print(_render_listing(document, lite=args.lite, blocks=args.blocks))
    return 0


def cfg_main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema cfg")
    parser.add_argument("target")
    parser.add_argument("--format", choices=("text", "json", "dot"), default="text")
    parser.add_argument("--confirmed-only", action="store_true")
    args = parser.parse_args(argv)
    _graph, document = _graph_document(
        ctx,
        args.target,
        confirmed_only=args.confirmed_only,
    )
    if args.format == "json":
        ctx.print_json("cfg", document)
    elif args.format == "dot":
        print(_render_cfg_dot(document))
    else:
        print(_render_cfg_text(document))
    return 0


def _hexdump(va: int, payload: bytes, width: int = 16) -> list[str]:
    lines = []
    for offset in range(0, len(payload), width):
        chunk = payload[offset:offset + width]
        text = "".join(chr(value) if 0x20 <= value < 0x7F else "." for value in chunk)
        lines.append(f"  {va + offset:08x}  {chunk.hex(' '):<{width * 3}} {text}")
    return lines


def dump_main(ctx: Context, argv: list[str]) -> int:
    parser = argparse.ArgumentParser(prog="kf sema dump")
    parser.add_argument("target")
    parser.add_argument("--size")
    parser.add_argument("--hex", action="store_true", dest="want_hex")
    parser.add_argument("--no-disasm", action="store_true")
    parser.add_argument("--confirmed-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)
    va, binding = resolve_binding(ctx, args.target)
    if args.size:
        size = parse_number(args.size)
    elif binding is not None:
        va, size = binding.va, binding.size
    else:
        die(f"0x{va:08x} has no admitted extent; pass --size")
    payload = ctx.img.require(va, size)
    references = [
        row.as_dict() for row in ctx.refs.references
        if ctx.refs.visible(row, confirmed_only=args.confirmed_only)
        and va <= row.site < va + size
    ]
    document: dict[str, object] = {
        "target": binding.as_dict() if binding else None,
        "va": va,
        "size": size,
        "bytes": payload.hex(),
        "references": references,
    }
    if binding is not None and binding.space == "text" and not args.no_disasm:
        disassembly = _listing_document(
            ctx,
            va,
            min(size, binding.body_size),
            binding,
            None,
            confirmed_only=args.confirmed_only,
        )
        document["instructions"] = disassembly["instructions"]
    if args.json:
        ctx.print_json("dump", document)
        return 0
    name = binding.name if binding else f"0x{va:08x}"
    print(f"== {name}  0x{va:08x}  0x{size:x} B")
    print(f"References ({len(references)}):")
    for row in references:
        target = "?" if row["target"] is None else ctx.idx.label(row["target"])
        print(f"  +0x{row['site'] - va:04x} {row['tier']:<9} {row['kind']:<14} -> {target}")
    if not references:
        print("  (none)")
    if args.want_hex:
        print("\nBytes:")
        print("\n".join(_hexdump(va, payload)))
    if "instructions" in document:
        print("\n" + _render_listing({
            "target": document["target"],
            "va": va,
            "size": min(size, binding.body_size),
            "instructions": document["instructions"],
            "cfg": None,
        }, lite=False, blocks=False))
    return 0
