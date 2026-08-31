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
from scripts.kf.readme import END, START, render_block, write_block


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
    def test_repository_manifest_has_valid_reconstruction_units(self) -> None:
        manifest = load_manifest()
        self.assertIn("probe-gcc260-o2-g0", manifest.profiles)
        self.assertGreater(len(manifest.units), 0)
        self.assertEqual(
            len({unit.unit for unit in manifest.units}),
            len(manifest.units),
        )
        self.assertTrue(all(unit.source_path.is_file() for unit in manifest.units))
        self.assertTrue(all(unit.function.scope == "decomp" for unit in manifest.units))


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


class ReadmeTests(unittest.TestCase):
    def test_generated_block_contains_all_three_images(self) -> None:
        summary = {
            "eligible_functions": 1,
            "manifested_functions": 0,
            "exact_functions": 0,
            "coverage_percent": 0.0,
            "fuzzy_overall_percent": 0.0,
        }
        document = {
            "images": {key: dict(summary) for key in ("psx", "game", "open")},
            "total": dict(summary),
        }
        block = render_block(document)
        self.assertIn("`PSX.EXE`", block)
        self.assertIn("`GAME.EXE`", block)
        self.assertIn("`OPEN.EXE`", block)
        self.assertIn("Exact requires 100%", block)

    def test_replacement_preserves_everything_outside_markers(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "README.md"
            path.write_text(f"# Intro\n\n{START}\nold\n{END}\n\n## Tail\n")
            self.assertTrue(write_block(f"{START}\nnew\n{END}", path))
            self.assertEqual(
                path.read_text(),
                f"# Intro\n\n{START}\nnew\n{END}\n\n## Tail\n",
            )


if __name__ == "__main__":
    unittest.main()
