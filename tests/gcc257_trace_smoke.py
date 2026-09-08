"""Compile real controls with native/debug/traced GCC; compare whole ELF bytes."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import struct
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.compile import compile_source
from scripts.kf.gcc_trace import (
    STAGES, Trace, assignment, constant, nodes, object_frame, read_trace, summarize,
)
from scripts.kf.hypotheses import strict_score
from scripts.kf.manifest import load
from elftools.elf.elffile import ELFFile


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def words(path: Path, name: str) -> tuple[int, ...]:
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        symbol, = elf.get_section_by_name(".symtab").get_symbol_by_name(name)
        section = elf.get_section(symbol["st_shndx"])
        start = symbol["st_value"] - section["sh_addr"]
        data = section.data()[start:start + symbol["st_size"]]
    return struct.unpack(f"<{len(data) // 4}I", data)


def validate_reload(trace: Trace) -> None:
    for index, row in enumerate(trace.events):
        if row["event"] != "reload.select":
            continue
        previous = trace.events[:index]
        searched = [event for event in previous if event["event"] == "reload.search"
                    and event["uid"] == row["uid"] and event["values"][0] == row["values"][0]]
        require(bool(searched), "reload choice has no search")
        start = searched[-1]["seq"]
        candidates = [event for event in previous if event["seq"] > start
                      and event["event"] == "reload.candidate" and event["uid"] == row["uid"]]
        require(bool(candidates) and candidates[-1]["hard_reg"] == row["hard_reg"],
                "reload selected a register outside the ordered candidate search")
        verdicts = [event for event in previous if event["seq"] > start
                    and event["event"] == "reload.free" and event["uid"] == row["uid"]
                    and event["hard_reg"] == row["hard_reg"]]
        require(bool(verdicts) and verdicts[-1]["values"][2] == 1,
                "reload selected a conflicting register")


def validate_controls(path: Path, obj: Path) -> None:
    # One reader pass validates every serialized row. Then group the controlled
    # functions without reparsing the complete TU once per assertion.
    checked = read_trace(path, "phase_one_spill")
    grouped = defaultdict(list)
    for line in path.read_text().splitlines():
        row = json.loads(line)
        grouped[row["function"]].append(row)
    traces = {name: Trace(name, checked.source_sha256, checked.compiler_source_sha256, events)
              for name, events in grouped.items() if name != "<translation-unit>"}
    for name, trace in traces.items():
        require({row["reason"] for row in trace.select("stage.end")} == set(STAGES),
                f"{name}: incomplete controlled trace")

    def crossing(name):
        return [row for row in traces[name].select("allocation.pseudo", stage="global-alloc")
                if row["values"][1] > 0]

    require(not crossing("crossing_absent") and len(crossing("crossing_present")) == 1,
            "call-crossing pseudo control failed")
    require(crossing("crossing_present")[0]["hard_reg"] in range(16, 24),
            "call-crossing value not assigned a saved register")
    require(len(object_frame(obj, "crossing_present")["saved"])
            == len(object_frame(obj, "crossing_absent")["saved"]) + 1,
            "allocation claim disagrees with emitted register saves")

    address = [{"name": "member", "kind": "address_lifetime", "symbol": "objects",
                "offset": 16, "members": [17, 37]}]
    folded = summarize(traces["member_folded"], address, obj, obj)["member"]["value"]
    require(folded["stores"]["cse1"]["relative"] == 4
            and folded["stores"]["cse2"]["absolute"] == 4,
            "CSE member-address preservation/destruction control failed")
    require(folded["absolute_rewrites"] == [{"pass": "cse2", "context": "cse.fold-address"}] * 4,
            "member rewrites were attributed to the wrong CSE decision")
    require(all(row["calls_crossed"] == 0 for row in folded["allocation"]["global-alloc"]),
            "folded address incorrectly reported live across calls")
    retained = summarize(traces["member_retained"], [{"name": "p", "kind": "source_lifetime",
                         "source_value": "first"}], obj, obj)["p"]["value"]
    require(any(row["calls_crossed"] >= 2 for row in retained["allocation"]["global-alloc"]),
            "unknown struct pointer did not retain its real call-crossing lifetime")
    require(bool(traces["member_folded"].select("cse.constant"))
            and bool(traces["member_folded"].select("cse.invalidate"))
            and bool(traces["member_folded"].select("cse.remove")),
            "CSE quantity, invalidation or removal hooks are absent")

    common = traces["cse_equivalent"]
    def additions(stage):
        return sum(node.get("code") == "plus" for row in common.snapshot(stage)
                   for node in nodes((row["x"] or {}).get("pattern")))
    require(additions("expand") > additions("cse1") == 1,
            "CSE elimination does not agree with before/after RTL")
    require(any(row["reason"] == "equivalence" for row in common.select("cse.insert"))
            and bool(common.select("change.commit")), "CSE equivalence control missing decisions")

    for name, object_size, rounded in (("stack_small", 12, 16), ("stack_large", 36, 40)):
        trace = traces[name]
        require(trace.select("stack.acquire")[0]["values"][0] == object_size,
                "stack object size changed")
        require(trace.select("stack.allocate")[0]["values"][:2] == [rounded, 8],
                "stack rounding/alignment is not explained by the allocator")
    require(object_frame(obj, "stack_large")["size"]
            - object_frame(obj, "stack_small")["size"] == 24,
            "stack-size trace disagrees with the emitted frame")
    reused = traces["stack_reuse"]
    acquired = reused.select("stack.acquire")
    require(len(acquired) == 2 and acquired[0]["x"]["ops"] == acquired[1]["x"]["ops"]
            and len(reused.select("stack.reuse")) == 1
            and len(reused.select("stack.release")) == 2, "stack lifetime/reuse control failed")

    feature = [{"name": "phase", "kind": "constant_registers",
                "source_value": "phase", "constants": [1, 2]}]
    for name, equal in (("phase_one_spill", True), ("phase_two_spills", False)):
        trace = traces[name]
        summary = summarize(trace, feature, obj, obj)["phase"]["value"]
        require(summary["same_hard_register"] is equal and summary["tails_equal"] is equal,
                f"{name}: constant reload/cross-jump control failed")
        pressure = summarize(trace, [{"name": "spill", "kind": "spill_registers"}],
                             obj, obj)["spill"]["value"]
        required = 1 if equal else 2
        require(pressure["maximum_search_count"] == required
                and len(pressure["registers"]) == required
                and pressure["class_maxima"]["1"] == required,
                f"{name}: class requirement, reservation and actual search disagree")
        stream = words(obj, name)
        for literal, hard in summary["registers"].items():
            require(any(word >> 26 in {9, 13} and (word >> 21) & 31 == 0
                        and (word >> 16) & 31 == hard[0] and word & 65535 == int(literal)
                        for word in stream), "constant register trace disagrees with emitted MIPS")
        validate_reload(trace)
    competition = traces["constant_competition"]
    before = {row["uid"]: assignment(row["x"]) for row in competition.snapshot("global-alloc")}
    choices = [row["hard_reg"] for row in competition.select("reload.select")
               if before.get(row["uid"]) and constant(before[row["uid"]][1]) in {1, 2}]
    require(set(choices) == {8, 9}, "t0/t1 constant competition control failed")
    validate_reload(competition)
    # An incorrect selected register must not pass this decision-chain check.
    damaged = [dict(row) for row in competition.events]
    next(row for row in damaged if row["event"] == "reload.select")["hard_reg"] = 66
    try:
        validate_reload(Trace(competition.function, competition.source_sha256,
                              competition.compiler_source_sha256, damaged))
    except RuntimeError:
        pass
    else:
        raise RuntimeError("reload negative control unexpectedly passed")

    for name in ("schedule_chain", "schedule_independent"):
        trace = traces[name]
        for stage in ("sched1", "sched2"):
            position = {row["uid"]: index for index, row in enumerate(trace.snapshot(stage))}
            for edge in trace.select("schedule.graph"):
                if edge["pass"] != stage:
                    continue
                producer = (edge["y"] or {}).get("uid")
                if producer in position and edge["uid"] in position:
                    require(position[producer] < position[edge["uid"]],
                            "scheduled RTL violates a traced data dependence")
            selected = [row for row in trace.select("schedule.select") if row["pass"] == stage]
            require(bool(selected), "scheduling selection hook missing")
            for row in selected:
                ready = [event for event in trace.select("schedule.ready")
                         if event["pass"] == stage and event["values"][0] == 0
                         and event["values"][2] == row["values"][0]]
                require(any(event["uid"] == row["uid"] for event in ready),
                        "scheduled instruction was not first in the sorted ready list")
        require(any(row["values"][0] >= 2 for row in trace.select("schedule.cost")),
                "load dependency latency hook missing")
        require(words(obj, name)[-2:] == (0x03e00008, 0x24420003),
                "MIPS return delay slot no longer contains the traced final add")
        placed = trace.select("delay.place")
        require(bool(placed), "delay-slot placement hook missing")
        sequences = [node for row in trace.snapshot("delay") for node in nodes(row["x"])
                     if node.get("code") == "sequence"]
        require(any(any(insn.get("uid") == event["y"]["uid"]
                        for insn in sequence["ops"][0])
                    for event in placed for sequence in sequences),
                "placed delay instruction absent from final sequence")
    print("controlled allocation, reload, CSE, cross-jump, stack and scheduling facts validated",
          flush=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--debug", required=True, type=Path)
    parser.add_argument("--instrumented", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--open-units", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    args.output.mkdir(parents=True, exist_ok=False)
    control_source = root / "tests/fixtures/gcc257_trace_controls.c"
    target_dir = args.output / "control-delink/open"
    target_dir.mkdir(parents=True)
    (target_dir / "objects.tsv").write_text("object\tscope\ncontrols.o\tmodule\n")
    corpus = [("controls", control_source, "controls.o", target_dir.parent, (), None)]
    if args.open_units:
        manifest = load()
        for name in ("open.render_init", "open.opening_scenes"):
            unit = manifest.by_name()[name]
            profile = manifest.profiles[unit.profile]
            if (profile.compiler, profile.optimization, profile.small_data, profile.cc1_flags) != (
                    "gcc257-native", "O2", 0, ("-mcpu=r2000",)):
                raise RuntimeError(f"unexpected target profile: {profile}")
            corpus.append((name, root / unit.source, unit.object_name,
                           root / "build/delink", unit.defines, unit))
    includes = (root / "include",)
    if os.environ.get("PSYQ_INCLUDE"):
        includes += (Path(os.environ["PSYQ_INCLUDE"]),)
    report = []
    for name, source, object_name, delink, defines, unit in corpus:
        reference = None
        trace_reference = None
        records = []
        for mode, probe in (("native", None), ("debug", args.debug),
                            ("disabled", args.instrumented),
                            ("enabled-1", args.instrumented),
                            ("enabled-2", args.instrumented)):
            output = args.output / name / mode / object_name
            trace = output.with_suffix(".jsonl") if mode.startswith("enabled") else None
            compile_source(source, "OPEN.EXE", output, delink, "O2", 0, "1.07",
                           includes, ("-mcpu=r2000",), "gcc257-native", defines=defines,
                           cc1_override=probe, trace_path=trace)
            blob = output.read_bytes()
            if reference is None:
                reference = blob
            if blob != reference:
                raise RuntimeError(f"{name}: {mode} changes object bytes")
            if trace:
                records = [json.loads(line) for line in trace.read_text().splitlines()]
                if trace_reference is None:
                    trace_reference = trace.read_bytes()
                elif trace.read_bytes() != trace_reference:
                    raise RuntimeError(f"{name}: nondeterministic trace")
                expected_hash = hashlib.sha256(source.read_bytes()).hexdigest()
                if not records or any(row["source_sha256"] != expected_hash for row in records):
                    raise RuntimeError(f"{name}: invalid source provenance")
        row = {"unit": name, "object_sha256": hashlib.sha256(reference).hexdigest(),
               "trace_sha256": hashlib.sha256(trace_reference).hexdigest(),
               "events": len(records), "strict_scores": {}}
        if unit:
            target = delink / "open/modules" / object_name
            for function in unit.functions:
                score, size, error = strict_score(target, output, function.symbol)
                if error:
                    raise RuntimeError(error)
                row["strict_scores"][function.symbol] = {"score": score, "size": size}
        else:
            validate_controls(trace, output)
        report.append(row)
        print(f"{name}: five byte-identical objects; {len(records)} deterministic events", flush=True)
    (args.output / "parity.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
