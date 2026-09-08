from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import compile_source
from scripts.kf.gcc_trace import STAGES, frontier_key, read_trace, validate_features
from scripts.kf.hypotheses import trace_frontier


def trace_rows() -> list[dict]:
    return [{"schema": 1, "seq": index + 1, "function": "sample", "pass": stage,
             "event": "stage.end", "source_sha256": "a" * 64,
             "compiler_source_sha256": "b" * 64, "uid": 0, "pseudo": -1,
             "hard_reg": -1, "expression": 0, "parser_line": 1, "reason": stage,
             "context": None, "values": [0, 0, 0], "x": None, "y": None}
            for index, stage in enumerate(STAGES)]


class TraceTests(unittest.TestCase):
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

    def test_retains_low_score_distinct_state_and_all_exact_results(self):
        rows = [{"index": index, "score": score, "frontier_key": key,
                 "error": error, "exact": score == 100}
                for index, score, key, error in (
                    (0, 95, "a", ""), (1, 93, "a", ""), (2, 60, "b", ""),
                    (3, 100, "c", ""), (4, 100, "c", ""), (5, 0, "d", "failure"))]
        self.assertEqual(trace_frontier(rows), {0, 2, 3, 4})


if __name__ == "__main__":
    unittest.main()
