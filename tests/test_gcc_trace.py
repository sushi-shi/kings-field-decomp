from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import compile_source
from scripts.kf.gcc_trace import (
    STAGES, frontier_key, object_frame, read_trace, summarize, validate_features,
)
from scripts.kf.hypotheses import trace_frontier
from scripts.kf.mips_elf import write_mips_elf


def trace_rows() -> list[dict]:
    return [{"schema": 1, "seq": index + 1, "function": "sample", "pass": stage,
             "event": "stage.end", "source_sha256": "a" * 64,
             "compiler_source_sha256": "b" * 64, "uid": 0, "pseudo": -1,
             "hard_reg": -1, "expression": 0, "parser_line": 1, "reason": stage,
             "context": None, "values": [0, 0, 0], "x": None, "y": None}
            for index, stage in enumerate(STAGES)]


def rtl(code, *ops):
    return {"code": code, "mode": "SI", "flags": [0] * 4, "ops": list(ops)}


def address_rows() -> list[dict]:
    rows = []
    root = rtl("const", rtl("plus", rtl("symbol_ref", "objects"), rtl("const_int", 16)))
    for end in trace_rows():
        stage = end["pass"]
        if stage in {"cse1", "cse2"}:
            # One selected member and one unrelated member of the same object.
            for uid, offset in ((20, 1), (21, 99)):
                addr = rtl("plus", rtl("reg", 71), rtl("const_int", offset))
                rows.append({**end, "event": "cse.address", "reason": "store",
                             "uid": uid, "pseudo": 71, "values": [100, 1, 1],
                             "x": addr, "y": root if stage == "cse2" else None})
            # A load in the selected instruction must not count as its store.
            rows.append({**rows[-2], "reason": "other"})
            patterns = [rtl("set", rtl("reg", 71), root)]
            patterns += [rtl("set", rtl("mem", rtl("plus", rtl("reg", 71),
                                                  rtl("const_int", offset))), rtl("const_int", 1))
                         for offset in (1, 99)]
            for uid, pattern in zip((10, 20, 21), patterns):
                insn = {**rtl("insn"), "uid": uid, "pattern": pattern}
                rows.append({**end, "event": "rtl.snapshot", "uid": uid, "x": insn})
        rows.append(end)
    return [{**row, "seq": index + 1} for index, row in enumerate(rows)]


class TraceTests(unittest.TestCase):
    def object(self, words):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = Path(directory.name) / "sample.o"
        data = struct.pack(f"<{len(words)}I", *words)
        path.write_bytes(write_mips_elf(data, "sample", len(data)))
        return path

    def test_frame_includes_branch_and_call_delay_slot_saves(self):
        for transfer in (0x14400022, 0x0c000000, 0x45000002):
            with self.subTest(transfer=hex(transfer)):
                # GAME 8002c510 saves ra in the first bnez delay slot.
                path = self.object([0x27bdffe8, transfer, 0xafbf0010,
                                    0xafb00014])
                self.assertEqual(object_frame(path, "sample"),
                                 {"status": "observed", "size": 24,
                                  "saved": [[31, 16]]})

    def test_divide_guard_before_frame_is_unknown(self):
        # GAME 8002c9d4: li v0,10; div a2,v0; bnez v0; nop; break 7.
        # This reader does not propagate the divisor and follow the guard.
        path = self.object([0x3402000a, 0x00c2001a, 0x14400002, 0,
                            0x0007000d, 0x27bdffe8, 0xafbf0010,
                            0x0c000000, 0])
        self.assertEqual(object_frame(path, "sample"),
                         {"status": "unavailable", "size": None, "saved": None})
        feature = summarize(self.read(trace_rows()), [{"name": "frame", "kind": "frame"}],
                            path, path)["frame"]["value"]
        self.assertIsNone(feature["frame_agrees"])
        self.assertIsNone(feature["saved_agree"])

    def test_complete_leaf_can_have_zero_frame(self):
        path = self.object([0x03e00008, 0x24020001])
        self.assertEqual(object_frame(path, "sample"),
                         {"status": "observed", "size": 0, "saved": []})

    def test_incomplete_or_conditional_slot_is_not_prologue_evidence(self):
        for words in ([0x27bdffe8], [0x27bdffe8, 0x0c000000],
                      [0x27bdffe8, 0x50400002, 0xafbf0010],
                      [0x27bdffe8, 0x14400002, 0x03e00008]):
            with self.subTest(words=words):
                self.assertEqual(object_frame(self.object(words), "sample")["status"],
                                 "unavailable")

    def test_rejects_trace_request_for_assembly_before_building(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "sample.s"
            source.write_text(".text\n")
            output = Path(directory) / "sample.o"
            with self.assertRaisesRegex(ValueError, "requires a C source"):
                compile_source(source, "GAME.EXE", output, compiler="gcc257-native",
                               cc1_override=Path("cc1psx-257-trace"),
                               trace_path=Path(directory) / "trace.jsonl")
            self.assertFalse(output.exists())

    def read(self, rows, *, source="a" * 64):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "trace.jsonl"
            path.write_text("".join(json.dumps(row) + "\n" for row in rows))
            return read_trace(path, "sample", source_sha256=source)

    def test_provenance_and_complete_stages(self):
        trace = self.read(trace_rows())
        self.assertEqual(trace.compiler_source_sha256, "b" * 64)
        self.assertEqual(trace.function, "sample")

    def test_rejects_mixed_compilers(self):
        rows = trace_rows()
        rows[-1]["compiler_source_sha256"] = "c" * 64
        with self.assertRaisesRegex(ValueError, "mixed source/compiler"):
            self.read(rows)

    def test_rejects_wrong_source(self):
        with self.assertRaisesRegex(ValueError, "source hash"):
            self.read(trace_rows(), source="d" * 64)

    def test_rejects_missing_final_stage(self):
        with self.assertRaisesRegex(ValueError, "incomplete trace"):
            self.read(trace_rows()[:-1])

    def test_rejects_reordered_or_duplicate_events(self):
        rows = trace_rows()
        rows[1]["seq"] = rows[0]["seq"]
        with self.assertRaisesRegex(ValueError, "non-increasing"):
            self.read(rows)

    def test_rejects_corrupt_rtl(self):
        rows = trace_rows()
        rows[0]["x"] = {"code": "reg", "mode": "SI", "flags": [0] * 4, "ops": []}
        with self.assertRaisesRegex(ValueError, "operand count"):
            self.read(rows)

    def test_requires_explicit_features_and_selectors(self):
        for invalid in (None, [], [{"name": "x", "kind": "made-up"}],
                        [{"name": "x", "kind": "address_lifetime"}],
                        [{"name": "x", "kind": "constant_registers",
                          "source_value": "phase", "constants": [1, 1]}]):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                validate_features(invalid)

    def test_frontier_ignores_event_number_churn(self):
        first = {"phase": {"value": {"same": False}, "evidence": [5, 7]}}
        second = {"phase": {"value": {"same": False}, "evidence": [55, 70]}}
        self.assertEqual(frontier_key(first), frontier_key(second))
        second["phase"]["value"]["same"] = True
        self.assertNotEqual(frontier_key(first), frontier_key(second))

    def address_summary(self, rows):
        feature = [{"name": "member", "kind": "address_lifetime", "symbol": "objects",
                    "offset": 16, "members": [17]}]
        return summarize(self.read(rows), feature, None, None)

    def test_address_inputs_select_member_stores_and_recorded_quantities(self):
        summary = self.address_summary(address_rows())["member"]["value"]
        for stage, quantity in (("cse1", "no-constant"), ("cse2", "constant")):
            self.assertEqual(summary["address_inputs"][stage], {
                "status": "observed", "stores_without_observation": 0,
                "observations": [{"form": "expression", "quantity": quantity,
                                  "mode_matches": True, "count": 1}],
            })

    def test_address_inputs_missing_is_not_no_constant(self):
        rows = [row for row in address_rows() if row["event"] != "cse.address"]
        summary = self.address_summary(rows)["member"]["value"]
        for result in summary["address_inputs"].values():
            self.assertEqual(result, {"status": "unavailable", "observations": [],
                                      "stores_without_observation": 1})

    def test_address_inputs_zero_constant_and_mode_mismatch_are_explicit(self):
        rows = address_rows()
        for row in rows:
            if row["event"] == "cse.address":
                row.update(x=rtl("reg", 71), y=rtl("const_int", 0), values=[100, 1, 0])
        summary = self.address_summary(rows)["member"]["value"]
        self.assertEqual(summary["address_inputs"]["cse1"]["observations"], [
            {"form": "register", "quantity": "constant", "mode_matches": False, "count": 1},
        ])

    def test_address_inputs_unassigned_quantity_is_distinct(self):
        rows = address_rows()
        for row in rows:
            if row["event"] == "cse.address":
                row.update(y=None, values=[71, 0, -1])
        summary = self.address_summary(rows)["member"]["value"]
        self.assertEqual(summary["address_inputs"]["cse1"]["observations"][0]["quantity"],
                         "unassigned")
        self.assertIsNone(summary["address_inputs"]["cse1"]["observations"][0]["mode_matches"])

    def test_address_frontier_ignores_pseudo_quantity_uid_and_sequence_churn(self):
        rows = address_rows()
        before = self.address_summary(rows)
        # Renumber serialized identities without changing the observed facts.
        def renumber(value):
            if isinstance(value, dict):
                if value.get("code") == "reg":
                    value["ops"][0] += 30
                if "uid" in value:
                    value["uid"] += 40
                for child in value.values():
                    renumber(child)
            elif isinstance(value, list):
                for child in value:
                    renumber(child)
        rows = json.loads(json.dumps(rows))
        renumber(rows)
        for row in rows:
            row["seq"] *= 3
            if row["event"] == "cse.address":
                row["pseudo"] += 30
                row["values"][0] += 50
        after = self.address_summary(rows)
        self.assertNotEqual(before["member"]["evidence"], after["member"]["evidence"])
        self.assertEqual(frontier_key(before), frontier_key(after))

    def test_retains_low_score_distinct_state_and_all_exact_results(self):
        rows = [{"index": index, "score": score, "frontier_key": key,
                 "error": error, "exact": score == 100}
                for index, score, key, error in (
                    (0, 95, "a", ""), (1, 93, "a", ""), (2, 60, "b", ""),
                    (3, 100, "c", ""), (4, 100, "c", ""), (5, 0, "d", "failure"))]
        self.assertEqual(trace_frontier(rows), {0, 2, 3, 4})


if __name__ == "__main__":
    unittest.main()
