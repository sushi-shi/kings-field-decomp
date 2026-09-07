"""Paired object CFG clues; never an equivalence or exactness gate.

Function extents come from each ELF independently. Only direct control targets
are relocated here; data references remain in the ordinary instruction diff.
Block ordinals tolerate instruction movement, but are not graph isomorphism.
"""

from __future__ import annotations

from dataclasses import dataclass, replace

from scripts.kf.parser_machine import _ObjectFile
from scripts.kf.sema.cfg import Graph, build_graph
from scripts.kf.sema.index import Binding, Index
from scripts.kf.sema.mips import Instruction, decode_control


@dataclass(frozen=True)
class Flow:
    graph: Graph | None
    issues: tuple[str, ...]


def object_flow(obj: _ObjectFile, function: Binding, idx: Index) -> Flow:
    """Place one ELF body at its retail entry, resolving direct jump targets.

    Do not substitute the retail body size for a missing candidate extent, or
    guess a destination for an unresolved relocation. Local section addends use
    the candidate's own symbol offset, even when an earlier function changed size.
    """
    fn = obj.named_symbol(function.name)
    section = obj.sections.get(fn.section, b"")
    if (fn.kind != "STT_FUNC" or fn.size <= 0 or fn.size % 4 or fn.value % 4
            or fn.value + fn.size > len(section)):
        return Flow(None, ("missing, unaligned, or truncated ELF function extent",))
    if function.fragments != 1:
        return Flow(None, ("fragmented function has no curated fragment ranges",))
    binding = replace(function, size=fn.size, body_size=fn.size)
    payload = bytearray(section[fn.value:fn.value + fn.size])
    issues: list[str] = []
    relocated: set[int] = set()
    for reloc in obj.relocations:
        if reloc.section != fn.section or not fn.value <= reloc.offset < fn.value + fn.size:
            continue
        offset = reloc.offset - fn.value
        word = int.from_bytes(payload[offset:offset + 4], "little")
        control = decode_control(binding.va + offset, word)
        if reloc.kind != 4:
            if control is not None:
                issues.append(f"+0x{offset:x}: unsupported control relocation {reloc.kind}")
            continue
        if offset % 4 or offset in relocated or word >> 26 not in {2, 3}:
            issues.append(f"+0x{offset:x}: invalid/duplicate R_MIPS_26")
            continue
        relocated.add(offset)
        symbol = obj.symbol(reloc.symbol_index)
        addend = (word & 0x03FFFFFF) << 2
        local = symbol.value + addend
        if symbol.section == fn.section and fn.value <= local < fn.value + fn.size:
            target = binding.va + local - fn.value
        else:
            name = symbol.name
            if symbol.kind == "STT_SECTION":
                owners = [s for s in obj.symbols if s.section == symbol.section
                          and s.kind == "STT_FUNC" and s.value <= local < s.value + s.size]
                if len(owners) != 1:
                    issues.append(f"+0x{offset:x}: unresolved section jump {name}+0x{addend:x}")
                    continue
                name, addend = owners[0].name, local - owners[0].value
            addresses = idx.resolve(name)
            if len(addresses) != 1:
                issues.append(f"+0x{offset:x}: unresolved jump symbol {name!r}")
                continue
            target = addresses[0] + addend
            if binding.contains_body(target):
                issues.append(f"+0x{offset:x}: external target overlaps trial function placement")
                continue
        if target & 3 or target >> 28 != (binding.va + offset + 4) >> 28:
            issues.append(f"+0x{offset:x}: R_MIPS_26 target is unaligned/out of region")
            continue
        word = (word & 0xFC000000) | ((target >> 2) & 0x03FFFFFF)
        payload[offset:offset + 4] = word.to_bytes(4, "little")
    # Unrelocated J/JAL fields in an ET_REL object may be section offsets or
    # retained linked addresses. Neither interpretation is silently assumed.
    for offset in range(0, fn.size, 4):
        word = int.from_bytes(payload[offset:offset + 4], "little")
        if word >> 26 in {2, 3} and offset not in relocated:
            issues.append(f"+0x{offset:x}: J/JAL has no R_MIPS_26 target evidence")
    if issues:
        return Flow(None, tuple(issues))
    graph = build_graph(binding, bytes(payload), idx)
    return Flow(graph, graph.issues)


def _destination(graph: Graph, target: int | None) -> str:
    if target is None:
        return "none"
    for number, block in enumerate(graph.blocks):
        if block.contains(target):
            interior = target - block.start
            return f"B{number}" + (f"+0x{interior:x}" if interior else "")
    return f"0x{target:08x}"


def _edges(graph: Graph) -> tuple[tuple[tuple[str, str], ...], ...]:
    return tuple(tuple((e.kind, _destination(graph, e.target)) for e in block.successors)
                 for block in graph.blocks)


def _controls(graph: Graph) -> list[tuple[int, Instruction]]:
    # Include controls after an unresolved dispatcher and in cold blocks.
    return [(number, row) for number, block in enumerate(graph.blocks)
            for row in block.instructions if row.control is not None]


def _return_frontier(graph: Graph) -> tuple[tuple[int, int, str], ...]:
    """Identify every known edge entering a block that contains a return.

    A single shared epilogue can represent many source-level exits, so counting
    ``jr ra`` instructions alone is insufficient.  Block ordinals retain the
    same diagnostic (rather than equivalence) contract as ``_edges``.
    """
    numbers = {block.start: number for number, block in enumerate(graph.blocks)}
    returns = {
        block.start for block in graph.blocks
        if any(row.control is not None and row.control.kind == "return"
               for row in block.instructions)
    }
    return tuple(sorted(
        (numbers[edge.target_block], numbers[edge.source], edge.kind)
        for edge in graph.edges
        if edge.target_block in returns and edge.source != edge.target_block
    ))


def _operand_word(row: Instruction) -> int:
    assert row.control is not None
    if row.control.conditional:
        return row.word & 0xFFFF0000
    if row.word >> 26 in {2, 3}:
        return row.word & 0xFC000000
    return row.word


def _window(graph: Graph, number: int, row: Instruction) -> str:
    rows = graph.blocks[number].instructions
    position = rows.index(row)
    before = ", ".join(f"+0x{r.va - graph.function.va:x}:{r.word:08x}"
                       for r in rows[max(0, position - 3):position]) or "none"
    slot = next((r for r in rows if r.va == row.va + 4), None)
    delay = (f"{slot.word:08x}" if slot else "MISSING") if row.control.delay_slot else "none"
    return (f"B{number} +0x{row.va - graph.function.va:x} {row.control.mnemonic} "
            f"{row.word:08x} -> {_destination(graph, row.control.target)}; "
            f"preceding words [{before}]; delay {delay}")


def compare_flow(want: Flow, have: Flow) -> list[str]:
    """Explain the first structural/control clue, without claiming equivalence."""
    lines: list[str] = []
    for side, flow in (("target", want), ("compiled", have)):
        for issue in flow.issues:
            lines.append(f"CFG WARNING {side}: {issue}")
    if want.graph is None or have.graph is None:
        return [*lines, "CFG comparison unavailable; unresolved object control/extent"]
    left, right = want.graph, have.graph
    a, b = _controls(left), _controls(right)

    def counts(rows, kind):
        return sum(r.control.kind == kind for _, r in rows)

    lines.append(
        f"CFG target/compiled: {len(left.blocks)}/{len(right.blocks)} blocks, "
        f"{counts(a, 'branch')}/{counts(b, 'branch')} branches, "
        f"{counts(a, 'return')}/{counts(b, 'return')} returns (not exit-path counts)"
    )
    frontier_a, frontier_b = _return_frontier(left), _return_frontier(right)
    lines.append(
        f"CFG return frontiers target/compiled: {len(frontier_a)}/{len(frontier_b)} "
        "known incoming edges to return blocks"
    )
    if frontier_a != frontier_b:
        lines.append(
            f"CFG differing return frontiers by block order: "
            f"target {frontier_a}; compiled {frontier_b}"
        )
    edges_a, edges_b = _edges(left), _edges(right)
    edge_difference = edges_a != edges_b
    if edge_difference:
        for number in range(max(len(edges_a), len(edges_b))):
            ea = edges_a[number] if number < len(edges_a) else None
            eb = edges_b[number] if number < len(edges_b) else None
            if ea != eb:
                lines.append(f"CFG first differing successors at B{number}: target {ea}; compiled {eb}")
                break
    else:
        qualifier = "INCOMPLETE: " if want.issues or have.issues else ""
        lines.append(f"CFG {qualifier}known successor lists agree by block order; "
                     "not semantic equivalence")
    for pair in range(max(len(a), len(b))):
        if pair >= len(a) or pair >= len(b):
            lines.append(f"CFG control #{pair}: missing on {'target' if pair >= len(a) else 'compiled'}")
            break
        na, ra = a[pair]
        nb, rb = b[pair]
        signature_a = (na, ra.control.kind, _operand_word(ra), _destination(left, ra.control.target))
        signature_b = (nb, rb.control.kind, _operand_word(rb), _destination(right, rb.control.target))
        if signature_a != signature_b:
            lines.append(f"CFG first differing control #{pair} (ordered correspondence):")
            lines.append("  target   " + _window(left, na, ra))
            lines.append("  compiled " + _window(right, nb, rb))
            break
    else:
        # Even equal edges and branch operands can test different values or
        # execute different side effects. This window is a clue, not dataflow.
        for (na, ra), (nb, rb) in zip(a, b):
            rows_a, rows_b = left.blocks[na].instructions, right.blocks[nb].instructions
            pa, pb = rows_a.index(ra), rows_b.index(rb)
            wa = tuple(r.word for r in rows_a[max(0, pa - 3):pa])
            wb = tuple(r.word for r in rows_b[max(0, pb - 3):pb])
            sa = next((r.word for r in rows_a if r.va == ra.va + 4), None)
            sb = next((r.word for r in rows_b if r.va == rb.va + 4), None)
            if wa != wb or (ra.control.delay_slot and sa != sb):
                lines.append("CFG first differing preceding-word/delay-slot window (not dataflow):")
                lines.append("  target   " + _window(left, na, ra))
                lines.append("  compiled " + _window(right, nb, rb))
                break
    return lines
