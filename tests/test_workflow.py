from __future__ import annotations

import json
import os
import tempfile
import unittest
from pathlib import Path

from scripts.kf.delink import Function
from scripts.kf.graph import _build_line, _write_generator
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.objdiff import generate_report
from scripts.kf.progress import Current, Target, _report_scores, _summary, classifications


def sample_current(percent: float | None) -> Current:
    function = Function(
        "GAME.EXE", 0x80010000, 16, 16, 1, "sample", "test", "test"
    )
    unit = Unit(
        "game_sample",
        "GAME.EXE",
        function.va,
        "src/game/sample.c",
        "probe-gcc260-o2-g0",
        function,
    )
    return Current(
        Target("GAME.EXE", function.va, function.symbol, function.body_size),
        unit,
        "a" * 64,
        percent,
        True,
        percent is not None,
    )


class ManifestTests(unittest.TestCase):
    def test_repository_manifest_allows_an_empty_unit_list(self) -> None:
        manifest = load_manifest()
        self.assertEqual(manifest.units, ())
        self.assertIn("probe-gcc260-o2-g0", manifest.profiles)


class ProgressTests(unittest.TestCase):
    def test_exact_is_strict_unless_loose_is_requested(self) -> None:
        row = sample_current(99.997)
        universe = {(row.target.image, row.target.va): row.target}
        strict = _summary(("GAME.EXE",), universe, [row], loose=False)
        loose = _summary(("GAME.EXE",), universe, [row], loose=True)
        self.assertEqual(strict["GAME.EXE"]["exact_functions"], 0)
        self.assertEqual(loose["GAME.EXE"]["exact_functions"], 1)

    def test_missing_objdiff_fuzzy_field_means_zero(self) -> None:
        scores, failures = _report_scores({
            "units": [{
                "name": "game_sample",
                "measures": {"total_code": "16", "total_functions": 1},
                "functions": [{"name": "sample", "size": "16"}],
            }]
        })
        self.assertEqual(scores, {"game_sample": 0.0})
        self.assertEqual(failures, [])

    def test_explicit_zero_fuzzy_does_not_fall_through_to_other_metric(self) -> None:
        scores, failures = _report_scores({
            "units": [{
                "name": "game_sample",
                "measures": {"total_code": "16", "total_functions": 1},
                "functions": [{
                    "name": "sample", "size": "16",
                    "fuzzy_match_percent": 0.0, "match_percent": 100.0,
                }],
            }]
        })
        self.assertEqual(scores, {"game_sample": 0.0})
        self.assertEqual(failures, [])

    def test_zero_total_report_unit_is_rejected(self) -> None:
        scores, failures = _report_scores({
            "units": [{"name": "dummy", "measures": {}}]
        })
        self.assertEqual(scores, {})
        self.assertEqual(len(failures), 1)

    def test_changed_input_is_distinct_from_unchanged_regression(self) -> None:
        row = sample_current(80.0)
        baseline = {
            (row.target.image, row.target.va): {
                "image": row.target.image,
                "va": hex(row.target.va),
                "unit": row.unit.unit,
                "name": row.target.name,
                "input_sha256": "b" * 64,
                "best_pct": "90.0",
                "hist_pct": "95.0",
                "banked_pct": "90.0",
                "code_size": "16",
            }
        }
        buckets = classifications([row], baseline)
        self.assertEqual(len(buckets["CHANGED"]), 1)
        self.assertEqual(buckets["REGRESS"], [])


class GraphTests(unittest.TestCase):
    def test_ninja_paths_escape_spaces_and_colons(self) -> None:
        lines = _build_line("build/a b", "rule", inputs=["C:/retail/GAME.EXE"])
        self.assertEqual(lines, ["build build/a$ b: rule C$:/retail/GAME.EXE"])

    def test_empty_objdiff_project_gets_zero_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "objdiff.json").write_text(json.dumps({"units": []}))
            report = generate_report(root)
            document = json.loads(report.read_text())
            self.assertEqual(document["units"], [])
            self.assertEqual(document["measures"]["total_code"], "0")

    def test_generator_rewrite_advances_identical_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "build.ninja"
            path.write_text("same\n")
            os.utime(path, ns=(1_000_000_000, 1_000_000_000))
            _write_generator(path, "same\n")
            self.assertGreater(path.stat().st_mtime_ns, 1_000_000_000)


if __name__ == "__main__":
    unittest.main()
