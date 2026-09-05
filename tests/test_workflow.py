from __future__ import annotations

import json
import os
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.kf import graph, progress
from scripts.kf.cli import _bank_functions
from scripts.kf.delink import Function
from scripts.kf.graph import _build_line, _script_inputs, _write_generator
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.objdiff import generate_report
from scripts.kf.progress import (
    Current,
    Target,
    _bank_rows,
    _eligible_rows,
    _report_scores,
    _summary,
    classifications,
)
from scripts.kf.readme import END, START, render_block, write_block


def sample_current(percent: float | None) -> Current:
    function = Function(
        "GAME.EXE", 0x80010000, 16, 16, 1, "sample", "test", "test"
    )
    unit = Unit(
        "game_sample",
        "GAME.EXE",
        "src/game/sample.c",
        "probe-gcc260-o2-g0",
        (function,),
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
        self.assertIn("probe-gcc257-o2-g0", manifest.profiles)
        probe_257 = manifest.profiles["probe-gcc257-o2-g0"]
        self.assertEqual(probe_257.compiler, "gcc257-native")
        self.assertEqual(probe_257.maspsx_flags, ("--expand-div",))
        self.assertEqual(manifest.profiles["probe-gcc260-o2-g0"].maspsx_flags, ())
        self.assertGreater(len(manifest.units), 0)
        self.assertEqual(
            len({unit.unit for unit in manifest.units}),
            len(manifest.units),
        )
        self.assertTrue(all(unit.source_path.is_file() for unit in manifest.units))
        self.assertTrue(all(
            function.scope == unit.scope
            for unit in manifest.units for function in unit.functions
        ))
        vendor_units = {
            unit.unit: unit
            for unit in manifest.units
            if unit.scope == "vendored"
        }
        self.assertEqual(
            set(vendor_units),
            {"game.intr_tail", "game.pad", "open.pad"},
        )
        self.assertTrue(all(
            function.scope == "vendored"
            for unit in vendor_units.values()
            for function in unit.functions
        ))
        # Units follow the linked order inside each image, and every unit is a
        # contiguous run of ascending claims.
        for image in {unit.image for unit in manifest.units}:
            starts = [unit.va for unit in manifest.units if unit.image == image]
            self.assertEqual(starts, sorted(starts))
        for unit in manifest.units:
            for previous, following in zip(unit.functions, unit.functions[1:]):
                self.assertEqual(previous.va + previous.size, following.va)

    def test_tmd_projection_and_vertex_selection_share_an_owner(self) -> None:
        manifest = load_manifest()
        owners = manifest.by_identity()
        family = (
            0x8001C114,  # object lookup
            0x8001C138,  # vertex cursor setter
            0x8001C148,  # object vertex selection
            0x8001C5EC,  # immediately preceding TMD lifetime operation
            0x8001C60C,  # perspective projection
            0x8001C6A8,  # shifted-depth projection
            0x8001C754,  # non-perspective transform
        )
        self.assertEqual(
            {owners[("GAME.EXE", va)].unit for va in family},
            {"game.render"},
        )
        self.assertNotIn("game.tmd_project", manifest.by_name())


class ProgressTests(unittest.TestCase):
    def test_vendored_verification_rows_do_not_enter_progress(self) -> None:
        row = sample_current(100.0)
        vendored = Current(
            row.target,
            Unit(
                row.unit.unit,
                row.unit.image,
                row.unit.source,
                row.unit.profile,
                row.unit.functions,
                scope="vendored",
            ),
            row.input_sha256,
            row.pct,
            row.compiled,
            row.scored,
        )
        self.assertEqual(_eligible_rows([row, vendored]), [row])

    def test_exact_is_strict_unless_loose_is_requested(self) -> None:
        row = sample_current(99.997)
        universe = {(row.target.image, row.target.va): row.target}
        strict = _summary(("GAME.EXE",), universe, [row], loose=False)
        loose = _summary(("GAME.EXE",), universe, [row], loose=True)
        self.assertEqual(strict["GAME.EXE"]["exact_functions"], 0)
        self.assertEqual(loose["GAME.EXE"]["exact_functions"], 1)

    def test_default_check_always_gates_data(self) -> None:
        with (
            mock.patch.object(progress, "current_state", return_value=(None, {}, [], [])),
            mock.patch.object(progress, "load_baseline", return_value={}),
            mock.patch.object(progress, "print_status", return_value=0),
            mock.patch.object(progress, "_report_cleanliness"),
            mock.patch("scripts.kf.data_match.run", return_value=1) as data_gate,
            mock.patch("scripts.kf.data_reachability.run", return_value=0),
            mock.patch("scripts.kf.readme.refresh", return_value=False),
            mock.patch("builtins.print"),
        ):
            self.assertEqual(progress.check(("GAME.EXE",)), 1)
        data_gate.assert_called_once()

    def test_default_check_rejects_unowned_data_even_when_objects_match(self) -> None:
        with (
            mock.patch.object(progress, "current_state", return_value=(None, {}, [], [])),
            mock.patch.object(progress, "load_baseline", return_value={}),
            mock.patch.object(progress, "print_status", return_value=0),
            mock.patch.object(progress, "_report_cleanliness"),
            mock.patch("scripts.kf.data_match.run", return_value=0),
            mock.patch("scripts.kf.data_reachability.run", return_value=1) as reachability,
            mock.patch("scripts.kf.readme.refresh", return_value=False),
            mock.patch("builtins.print"),
        ):
            self.assertEqual(progress.check(("GAME.EXE",)), 1)
        reachability.assert_called_once_with(("GAME.EXE",))

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

    def test_selective_bank_preserves_unrelated_rows(self) -> None:
        row = sample_current(100.0)
        unrelated = {
            "image": "OPEN.EXE",
            "va": "0x80020000",
            "unit": "open.unrelated",
            "name": "unrelated",
            "input_sha256": "b" * 64,
            "best_pct": "80.000000000",
            "hist_pct": "90.000000000",
            "banked_pct": "80.000000000",
            "code_size": "16",
        }
        old = {(unrelated["image"], int(unrelated["va"], 16)): unrelated}
        output = _bank_rows([row], old, (row.unit.unit,))
        by_identity = {
            (item["image"], int(str(item["va"]), 16)): item for item in output
        }
        self.assertEqual(by_identity[("OPEN.EXE", 0x80020000)], unrelated)
        banked = by_identity[(row.target.image, row.target.va)]
        self.assertEqual(banked["banked_pct"], "100.000000000")

    def test_selective_bank_rejects_nonexact_function(self) -> None:
        row = sample_current(99.999)
        with self.assertRaisesRegex(ValueError, "not exact"):
            _bank_rows([row], {}, (row.unit.unit,))

    def test_selective_function_bank_preserves_unrelated_rows(self) -> None:
        row = sample_current(100.0)
        unrelated = {
            "image": "OPEN.EXE",
            "va": "0x80020000",
            "unit": "open.unrelated",
            "name": "unrelated",
            "input_sha256": "b" * 64,
            "best_pct": "80.000000000",
            "hist_pct": "90.000000000",
            "banked_pct": "80.000000000",
            "code_size": "16",
        }
        old = {(unrelated["image"], int(unrelated["va"], 16)): unrelated}
        identity = row.target.image, row.target.va
        output = _bank_rows([row], old, selected_functions=(identity,))
        by_identity = {
            (item["image"], int(str(item["va"]), 16)): item for item in output
        }
        self.assertEqual(by_identity[("OPEN.EXE", 0x80020000)], unrelated)
        self.assertEqual(by_identity[identity]["banked_pct"], "100.000000000")

    def test_selective_function_bank_rejects_nonexact_function(self) -> None:
        row = sample_current(99.999)
        identity = row.target.image, row.target.va
        with self.assertRaisesRegex(ValueError, "not exact"):
            _bank_rows([row], {}, selected_functions=(identity,))

    def test_bank_function_selector_uses_image_and_address(self) -> None:
        self.assertEqual(
            _bank_functions(["game:0x80010000", "open:0x80020000"]),
            (("GAME.EXE", 0x80010000), ("OPEN.EXE", 0x80020000)),
        )

    def test_bank_function_selector_rejects_missing_image(self) -> None:
        with self.assertRaisesRegex(ValueError, "expected IMAGE:VA"):
            _bank_functions(["0x80010000"])


class GraphTests(unittest.TestCase):
    def test_reports_and_checks_depend_directly_on_toolchain_identity(self) -> None:
        with (
            mock.patch.object(graph, "configured_retail_dir", return_value=Path("/retail")),
            mock.patch.object(graph, "_prune_orphans", return_value=0),
            mock.patch.object(graph, "_write_if_changed"),
            mock.patch.object(graph, "_write_generator") as write,
        ):
            graph.emit()
        lines = write.call_args.args[1].splitlines()
        for rule in ("report", "check"):
            edges = [line for line in lines if f": {rule} " in line]
            self.assertEqual(len(edges), 3)
            for edge in edges:
                self.assertIn("build/gen/toolchain.id", edge.split(" | ")[1].split())

    def test_nested_semantic_evidence_is_a_build_dependency(self) -> None:
        self.assertIn("scripts/kf/sema/evidence.py", _script_inputs())
        self.assertIn("scripts/kf/data_reachability.py", _script_inputs())

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
