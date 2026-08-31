"""Delay-slot-aware basic-block and control-flow graph construction."""

from __future__ import annotations

from dataclasses import dataclass, field

from scripts.kf.sema.index import Binding, Index
from scripts.kf.sema.mips import Instruction, with_controls, decode_words


@dataclass
class Edge:
    source: int
    site: int
    kind: str
    target: int | None = None
    target_block: int | None = None
    confidence: str = "proven"

    def as_dict(self) -> dict[str, object]:
        return {
            "source": self.source,
            "site": self.site,
            "kind": self.kind,
            "target": self.target,
            "target_block": self.target_block,
            "confidence": self.confidence,
        }


@dataclass
class Block:
    start: int
    end: int
    instructions: tuple[Instruction, ...]
    predecessors: list[int] = field(default_factory=list)
    successors: list[Edge] = field(default_factory=list)
    flags: set[str] = field(default_factory=set)

    def contains(self, va: int) -> bool:
        return self.start <= va < self.end

    def as_dict(self) -> dict[str, object]:
        return {
            "start": self.start,
            "end": self.end,
            "instructions": [
                {"va": row.va, "word": row.word} for row in self.instructions
            ],
            "predecessors": sorted(self.predecessors),
            "successors": [edge.as_dict() for edge in self.successors],
            "flags": sorted(self.flags),
        }


@dataclass
class Graph:
    function: Binding
    blocks: tuple[Block, ...]
    edges: tuple[Edge, ...]
    delay_slots: dict[int, int]
    issues: tuple[str, ...]

    def as_dict(self) -> dict[str, object]:
        return {
            "function": self.function.as_dict(),
            "blocks": [block.as_dict() for block in self.blocks],
            "edges": [edge.as_dict() for edge in self.edges],
            "delay_slots": [
                {"slot": slot, "owner": owner}
                for slot, owner in sorted(self.delay_slots.items())
            ],
            "issues": list(self.issues),
        }


def _block_for(blocks: list[Block], va: int | None) -> Block | None:
    if va is None:
        return None
    return next((block for block in blocks if block.contains(va)), None)


def build_graph(function: Binding, payload: bytes, idx: Index) -> Graph:
    if function.space != "text":
        raise ValueError(f"{function.name}: CFG requires a function")
    if function.fragments != 1:
        raise ValueError(
            f"{function.name}: CFG unavailable for fragmented function; "
            "curate fragment ranges first"
        )
    instructions = with_controls(decode_words(function.va, payload[:function.body_size]))
    if not instructions:
        return Graph(function, (), (), {}, ("no complete instruction decoded",))

    addresses = {row.va for row in instructions}
    instruction_by_va = {row.va: row for row in instructions}
    leaders = {instructions[0].va}
    delay_slots: dict[int, int] = {}
    issues: list[str] = []

    for row in instructions:
        control = row.control
        if control is None:
            continue
        after = row.va + (8 if control.delay_slot else 4)
        if control.delay_slot and row.va + 4 in addresses:
            previous = delay_slots.setdefault(row.va + 4, row.va)
            if previous != row.va:
                issues.append(
                    f"0x{row.va + 4:08x} is a delay slot for multiple transfers"
                )
        if control.target in addresses:
            leaders.add(control.target)
        if control.terminates_block and after in addresses:
            leaders.add(after)

    targeted_delay_slots = sorted(leaders & set(delay_slots))
    for va in targeted_delay_slots:
        leaders.remove(va)
        issues.append(
            f"branch target 0x{va:08x} enters the delay slot owned by "
            f"0x{delay_slots[va]:08x}; represented as an interior target"
        )

    ordered_leaders = sorted(leaders)
    blocks: list[Block] = []
    for position, start in enumerate(ordered_leaders):
        next_leader = (
            ordered_leaders[position + 1]
            if position + 1 < len(ordered_leaders)
            else function.body_end
        )
        rows: list[Instruction] = []
        va = start
        while va < next_leader and va in instruction_by_va:
            row = instruction_by_va[va]
            rows.append(row)
            control = row.control
            if control is not None and control.terminates_block:
                if control.delay_slot and va + 4 in instruction_by_va:
                    rows.append(instruction_by_va[va + 4])
                break
            va += 4
        if rows:
            blocks.append(Block(start, rows[-1].va + 4, tuple(rows)))

    edges: list[Edge] = []
    for number, block in enumerate(blocks):
        terminal = next(
            (
                row for row in reversed(block.instructions)
                if row.control is not None and row.control.terminates_block
            ),
            None,
        )
        for row in block.instructions:
            control = row.control
            if control is None or not control.call:
                continue
            if control.kind == "indirect-call":
                edges.append(Edge(block.start, row.va, "indirect-call"))
            else:
                target_block = _block_for(blocks, control.target)
                edges.append(Edge(
                    block.start,
                    row.va,
                    "call",
                    control.target,
                    target_block.start if target_block else None,
                ))

        if terminal is None:
            if number + 1 < len(blocks) and block.end == blocks[number + 1].start:
                edges.append(Edge(
                    block.start,
                    block.instructions[-1].va,
                    "fallthrough",
                    blocks[number + 1].start,
                    blocks[number + 1].start,
                ))
            continue

        control = terminal.control
        assert control is not None
        after = terminal.va + (8 if control.delay_slot else 4)
        target_block = _block_for(blocks, control.target)
        if control.kind == "branch":
            edges.append(Edge(
                block.start,
                terminal.va,
                "taken" if target_block else "taken-external",
                control.target,
                target_block.start if target_block else None,
            ))
            fallthrough = _block_for(blocks, after)
            if fallthrough is not None:
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "fallthrough",
                    after,
                    fallthrough.start,
                ))
        elif control.kind == "call" and control.conditional:
            fallthrough = _block_for(blocks, after)
            if fallthrough is not None:
                edges.append(Edge(
                    block.start,
                    terminal.va,
                    "fallthrough",
                    after,
                    fallthrough.start,
                ))
        elif control.kind == "jump":
            external = idx.function(control.target or 0)
            kind = "jump" if target_block else "tail" if external else "jump-external"
            edges.append(Edge(
                block.start,
                terminal.va,
                kind,
                control.target,
                target_block.start if target_block else None,
            ))
        elif control.kind == "return":
            edges.append(Edge(block.start, terminal.va, "return"))
        elif control.kind == "indirect-jump":
            edges.append(Edge(block.start, terminal.va, "indirect-jump"))
        elif control.kind == "trap":
            edges.append(Edge(block.start, terminal.va, "trap"))

    for edge in edges:
        source = _block_for(blocks, edge.source)
        if source is not None:
            source.successors.append(edge)
        target = _block_for(blocks, edge.target_block)
        if target is not None and edge.kind not in {"call"}:
            target.predecessors.append(edge.source)

    reachable: set[int] = set()
    pending = [blocks[0].start] if blocks else []
    while pending:
        start = pending.pop()
        if start in reachable:
            continue
        reachable.add(start)
        block = _block_for(blocks, start)
        if block is None:
            continue
        for edge in block.successors:
            if edge.target_block is None:
                continue
            if edge.kind == "call" and not function.contains_body(edge.target_block):
                continue
            pending.append(edge.target_block)

    for block in blocks:
        if block.start not in reachable:
            block.flags.add("unreachable")
        if any(source > block.start for source in block.predecessors):
            block.flags.add("loop-head")
        if len(set(block.predecessors)) > 2 and any(
            row.control is not None and row.control.kind == "return"
            for row in block.instructions[:8]
        ):
            block.flags.add("common-tail")
        if any(row.va in delay_slots for row in block.instructions):
            block.flags.add("has-delay-slot")

    return Graph(
        function,
        tuple(blocks),
        tuple(edges),
        delay_slots,
        tuple(dict.fromkeys(issues)),
    )
