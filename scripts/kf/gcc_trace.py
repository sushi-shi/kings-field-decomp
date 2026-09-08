"""Read observational GCC 2.5.7 traces and extract explicitly selected facts.

Unknown or ambiguous value chains stay unknown. Evidence sequence numbers and
RTL UIDs are retained for inspection but excluded from frontier equivalence.
"""

from __future__ import annotations

import hashlib
import json
import re
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator

from elftools.elf.elffile import ELFFile


STAGES = ("expand", "jump1", "cse1", "loop", "cse2", "flow", "combine", "sched1",
          "local-alloc", "global-alloc", "reload", "sched2", "jump2", "delay", "final")
KINDS = {"frame", "constant_registers", "address_lifetime", "source_lifetime", "crossjump",
         "spill_registers"}
HASH = re.compile(r"[0-9a-f]{64}\Z")
FIRST_PSEUDO_REGISTER = 67


def nodes(value: object) -> Iterator[dict]:
    if isinstance(value, dict):
        yield value
        for key in ("ops", "pattern"):
            yield from nodes(value.get(key))
    elif isinstance(value, list):
        for child in value:
            yield from nodes(child)


def reg(value: object) -> int | None:
    if isinstance(value, dict) and value.get("code") == "reg":
        return value["ops"][0]
    return None


def constant(value: object) -> int | None:
    if isinstance(value, dict) and value.get("code") == "const_int":
        return value["ops"][0]
    return None


def symbolic(value: object) -> tuple[str, int] | None:
    if not isinstance(value, dict):
        return None
    code, ops = value.get("code"), value.get("ops", [])
    if code == "symbol_ref":
        return ops[0], 0
    if code == "const":
        return symbolic(ops[0])
    if code == "plus" and (base := symbolic(ops[0])) is not None:
        if (offset := constant(ops[1])) is not None:
            return base[0], base[1] + offset
    return None


def register_offset(value: object) -> tuple[int, int] | None:
    if (number := reg(value)) is not None:
        return number, 0
    if isinstance(value, dict) and value.get("code") == "plus":
        left, right = value["ops"]
        if (number := reg(left)) is not None and (offset := constant(right)) is not None:
            return number, offset
    return None


def assignment(insn: dict | None) -> tuple[dict, dict] | None:
    pattern = (insn or {}).get("pattern") or {}
    if pattern.get("code") == "set":
        return tuple(pattern["ops"])
    return None


def _validate_rtx(value: object) -> None:
    if value is None:
        return
    if not isinstance(value, dict) or not isinstance(value.get("code"), str):
        raise ValueError("invalid serialized RTL")
    if not isinstance(value.get("mode"), str) or value.get("flags") is None:
        raise ValueError("missing RTL mode/flags")
    if not isinstance(value["flags"], list) or len(value["flags"]) != 4:
        raise ValueError("invalid RTL flags")
    if "uid" in value:
        if type(value["uid"]) is not int or value["uid"] < 0:
            raise ValueError("invalid RTL UID")
        for key in ("pattern", "notes"):
            _validate_rtx(value.get(key))
        return
    ops = value.get("ops")
    if not isinstance(ops, list):
        raise ValueError("missing RTL operands")
    lengths = {"reg": 1, "const_int": 1, "plus": 2, "set": 2,
               "mem": 1, "const": 1, "symbol_ref": 1, "subreg": 2}
    if value["code"] in lengths and len(ops) != lengths[value["code"]]:
        raise ValueError("invalid RTL operand count")
    if value["code"] in {"reg", "const_int"} and type(ops[0]) is not int:
        raise ValueError("invalid RTL integer")
    for op in ops:
        if isinstance(op, dict):
            _validate_rtx(op)
        elif isinstance(op, list):
            for child in op:
                _validate_rtx(child)


@dataclass
class Trace:
    function: str
    source_sha256: str
    compiler_source_sha256: str
    events: list[dict]

    def select(self, event: str, *, stage: str | None = None) -> list[dict]:
        return [row for row in self.events if row["event"] == event
                and (stage is None or row["reason"] == stage)]

    def snapshot(self, stage: str) -> list[dict]:
        return self.select("rtl.snapshot", stage=stage)

    def source_pseudos(self, name: str) -> set[int]:
        return {number for row in self.select("source.value")
                if row["reason"] == name and (number := reg(row["x"])) is not None
                and number >= FIRST_PSEUDO_REGISTER}


def read_trace(path: Path, function: str, *, source_sha256: str | None = None) -> Trace:
    selected = []
    previous = 0
    identity = None
    with path.open(encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            try:
                row = json.loads(line)
                if not isinstance(row, dict) or row.get("schema") != 1:
                    raise ValueError("unsupported schema")
                for key in ("function", "pass", "event", "source_sha256", "compiler_source_sha256"):
                    if not isinstance(row.get(key), str) or not row[key]:
                        raise ValueError(f"missing {key}")
                for key in ("seq", "uid", "pseudo", "hard_reg", "expression", "parser_line"):
                    if type(row.get(key)) is not int:
                        raise ValueError(f"invalid {key}")
                if row["seq"] <= previous:
                    raise ValueError("non-increasing event sequence")
                previous = row["seq"]
                pair = row["source_sha256"], row["compiler_source_sha256"]
                if not all(HASH.fullmatch(value) for value in pair):
                    raise ValueError("invalid provenance hash")
                if identity is not None and pair != identity:
                    raise ValueError("mixed source/compiler provenance")
                identity = pair
                if source_sha256 is not None and pair[0] != source_sha256:
                    raise ValueError("source hash does not match candidate")
                if row["pass"] not in STAGES:
                    raise ValueError("unknown compiler pass")
                if (not isinstance(row.get("values"), list) or len(row["values"]) != 3
                        or any(type(value) is not int for value in row["values"])):
                    raise ValueError("invalid event values")
                for key in ("x", "y", "reason", "context"):
                    if key not in row:
                        raise ValueError(f"missing {key}")
                _validate_rtx(row["x"])
                _validate_rtx(row["y"])
                if row["function"] == function:
                    selected.append(row)
            except (ValueError, TypeError, KeyError) as error:
                raise ValueError(f"{path}:{line_number}: {error}") from error
    if not selected or identity is None:
        raise ValueError(f"{path}: no events for {function}")
    ended = {row["reason"] for row in selected if row["event"] == "stage.end"}
    if ended != set(STAGES):
        raise ValueError(f"{path}: incomplete trace for {function}; stages {sorted(ended)}")
    return Trace(function, *identity, selected)


def validate_features(raw: object) -> list[dict]:
    if not isinstance(raw, list) or not raw:
        raise ValueError("tracing requires a non-empty trace_features list")
    names = set()
    for spec in raw:
        if not isinstance(spec, dict) or spec.get("kind") not in KINDS:
            raise ValueError(f"trace feature kind must be one of {sorted(KINDS)}")
        name = spec.get("name")
        if not isinstance(name, str) or not name or name in names:
            raise ValueError("trace feature names must be unique non-empty strings")
        names.add(name)
        kind = spec["kind"]
        if kind in {"source_lifetime", "constant_registers"}:
            if not isinstance(spec.get("source_value"), str) or not spec["source_value"]:
                raise ValueError(f"{name}: source_value is required")
        if kind == "constant_registers":
            values = spec.get("constants")
            if not isinstance(values, list) or len(values) < 2 or any(type(n) is not int for n in values):
                raise ValueError(f"{name}: constants must contain at least two integers")
            if len(set(values)) != len(values):
                raise ValueError(f"{name}: constants must be distinct")
        if kind == "address_lifetime":
            if (not isinstance(spec.get("symbol"), str) or not spec["symbol"]
                    or type(spec.get("offset")) is not int):
                raise ValueError(f"{name}: symbol and integer offset are required")
            members = spec.get("members")
            if not isinstance(members, list) or not members or any(type(n) is not int for n in members):
                raise ValueError(f"{name}: members must contain integer offsets from symbol")
    return raw


def _lifetime(trace: Trace, pseudos: set[int]) -> dict:
    result = {}
    for stage in ("local-alloc", "global-alloc"):
        rows = [row for row in trace.select("allocation.pseudo", stage=stage)
                if row["pseudo"] in pseudos]
        result[stage] = sorted(
            [{"hard_reg": row["hard_reg"], "references": row["values"][0],
              "calls_crossed": row["values"][1], "live_length": row["values"][2]}
             for row in rows], key=lambda row: tuple(row.values()))
    return result


def _constants(trace: Trace, spec: dict) -> tuple[dict, list[int]]:
    pseudos = trace.source_pseudos(spec["source_value"])
    if len(pseudos) != 1:
        return {"status": "unavailable", "reason": "source value has no unique pseudo"}, []
    pseudo = next(iter(pseudos))
    origins: dict[int, list[int]] = {value: [] for value in spec["constants"]}
    for row in trace.snapshot("global-alloc"):
        if (parts := assignment(row["x"])) is not None:
            dest, value = parts
            if reg(dest) == pseudo and constant(value) in origins:
                origins[constant(value)].append(row["uid"])
    after = {row["uid"]: row for row in trace.snapshot("reload")}
    registers = {}
    evidence = []
    store_uids = set()
    for value, uids in origins.items():
        hard = []
        for uid in uids:
            row = after.get(uid)
            parts = assignment(row["x"]) if row else None
            if parts and constant(parts[1]) == value and (number := reg(parts[0])) is not None:
                hard.append(number)
                evidence.append(row["seq"])
                # Reload emits the destination spill immediately after the
                # defining insn. Require both its direction and register.
                ordered = list(after)
                index = ordered.index(uid)
                if index + 1 < len(ordered):
                    following = after[ordered[index + 1]]
                    store = assignment(following["x"])
                    if store and store[0].get("code") == "mem" and reg(store[1]) == number:
                        store_uids.add(following["uid"])
        registers[str(value)] = sorted(set(hard))
    comparisons = []
    # Only report comparisons of the selected value's actual spill stores.
    for row in trace.events:
        if row["event"] in {"crossjump.equal", "crossjump.reject"}:
            pair = {(row[key] or {}).get("uid") for key in ("x", "y")}
            if len(pair) == 2 and pair <= store_uids:
                comparisons.append(row["event"] == "crossjump.equal")
                evidence.append(row["seq"])
    known = all(len(origins[n]) == 1 and len(registers[str(n)]) == 1 for n in origins)
    same = len({numbers[0] for numbers in registers.values()}) == 1 if known else None
    return {"status": "observed" if known else "unavailable", "registers": registers,
            "same_hard_register": same,
            "tails_equal": comparisons[-1] if comparisons else None}, evidence


def _address(trace: Trace, spec: dict) -> tuple[dict, list[int]]:
    root = spec["symbol"], spec["offset"]
    pseudos = set()
    evidence = []
    for row in trace.snapshot("cse1"):
        if (parts := assignment(row["x"])) is not None:
            if symbolic(parts[1]) == root and (number := reg(parts[0])) is not None:
                pseudos.add(number)
                evidence.append(row["seq"])
    if len(pseudos) != 1:
        return {"status": "unavailable", "reason": "address root has no unique CSE pseudo"}, evidence
    pseudo = next(iter(pseudos))
    stores = {}
    for stage in ("cse1", "cse2", "global-alloc"):
        snapshot = trace.snapshot(stage)
        # Only propagate single-definition pseudo addresses. Hard registers
        # and multiply assigned pseudos require dataflow that this reader
        # deliberately does not guess.
        definitions: dict[int, list[dict]] = {}
        for row in snapshot:
            parts = assignment(row["x"])
            if parts and (number := reg(parts[0])) is not None and number >= FIRST_PSEUDO_REGISTER:
                definitions.setdefault(number, []).append(parts[1])
        roots = {}
        for _ in range(len(definitions)):
            added = False
            for number, values in definitions.items():
                if number in roots or len(values) != 1:
                    continue
                resolved = symbolic(values[0])
                if resolved is None and (alias := register_offset(values[0])) is not None:
                    if alias[0] in roots:
                        name, offset = roots[alias[0]]
                        resolved = name, offset + alias[1]
                if resolved is not None:
                    roots[number] = resolved
                    added = True
            if not added:
                break
        counts = {"relative": 0, "other_base_relative": 0, "absolute": 0}
        for row in snapshot:
            parts = assignment(row["x"])
            if not parts or parts[0].get("code") != "mem":
                continue
            addr = parts[0]["ops"][0]
            relative = register_offset(addr)
            absolute = symbolic(addr)
            if relative and relative[0] == pseudo and root[1] + relative[1] in spec["members"]:
                counts["relative"] += 1
                evidence.append(row["seq"])
            elif absolute and absolute[0] == root[0] and absolute[1] in spec["members"]:
                counts["absolute"] += 1
                evidence.append(row["seq"])
            elif relative and relative[0] in roots:
                name, offset = roots[relative[0]]
                if name == root[0] and offset + relative[1] in spec["members"]:
                    counts["other_base_relative"] += 1
                    evidence.append(row["seq"])
        stores[stage] = counts
    rewrites = []
    for row in trace.select("change.commit"):
        before, after = register_offset(row["x"]), symbolic(row["y"])
        if (before and before[0] == pseudo and after
                and after[0] == root[0] and after[1] in spec["members"]):
            rewrites.append({"pass": row["pass"], "context": row["context"]})
            evidence.append(row["seq"])
    return {"status": "observed", "stores": stores, "allocation": _lifetime(trace, pseudos),
            "absolute_rewrites": rewrites}, evidence


def object_frame(path: Path, symbol: str) -> dict:
    """Decode the O32 prologue, stopping at the first control transfer."""
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        rows = elf.get_section_by_name(".symtab").get_symbol_by_name(symbol) or []
        if len(rows) != 1 or not isinstance(rows[0]["st_shndx"], int):
            raise ValueError(f"{path}: no unique defined symbol {symbol}")
        row = rows[0]
        section = elf.get_section(row["st_shndx"])
        start = row["st_value"] - section["sh_addr"]
        data = section.data()[start:start + row["st_size"]]
    words = struct.unpack(f"<{len(data) // 4}I", data)
    size = 0
    saved = []
    for word in words:
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        immediate = (word & 0x7fff) - (word & 0x8000)
        if op in {1, 2, 3, 4, 5, 6, 7} or (op == 0 and word & 63 in {8, 9}):
            break
        if op == 9 and rs == rt == 29 and immediate < 0:
            size = -immediate
        if op == 43 and rs == 29 and rt in {*range(16, 24), 30, 31}:
            saved.append([rt, immediate])
    return {"size": size, "saved": sorted(saved)}


def summarize(trace: Trace, specs: list[dict], candidate: Path, target: Path) -> dict:
    result = {}
    for spec in validate_features(specs):
        kind = spec["kind"]
        evidence = []
        if kind == "frame":
            actual, expected = object_frame(candidate, trace.function), object_frame(target, trace.function)
            value = {"candidate": actual, "target": expected,
                     "frame_agrees": actual["size"] == expected["size"],
                     "saved_agree": actual["saved"] == expected["saved"]}
        elif kind == "constant_registers":
            value, evidence = _constants(trace, spec)
        elif kind == "address_lifetime":
            value, evidence = _address(trace, spec)
        elif kind == "source_lifetime":
            pseudos = trace.source_pseudos(spec["source_value"])
            value = {"status": "observed" if len(pseudos) == 1 else "unavailable",
                     "allocation": _lifetime(trace, pseudos)}
        elif kind == "spill_registers":
            searches = trace.select("reload.search")
            reserved = trace.select("reload.reserve")
            requirements = trace.select("reload.requirements")
            classes = {row["values"][0] for row in requirements}
            value = {
                "status": "observed" if reserved or searches else "unavailable",
                "registers": sorted({row["hard_reg"] for row in reserved}),
                "maximum_search_count": max((row["values"][2] for row in searches), default=0),
                "class_maxima": {str(number): max(row["values"][1] for row in requirements
                                                  if row["values"][0] == number)
                                 for number in sorted(classes)},
            }
            evidence = [row["seq"] for row in reserved + requirements]
        else:
            value = {event: len(trace.select(f"crossjump.{event}"))
                     for event in ("equal", "reject", "apply")}
            evidence = [row["seq"] for row in trace.events if row["event"].startswith("crossjump.")]
        result[spec["name"]] = {"value": value, "evidence": sorted(set(evidence))}
    return result


def frontier_key(features: dict) -> str:
    state = {name: feature["value"] for name, feature in features.items()}
    return hashlib.sha256(json.dumps(state, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
