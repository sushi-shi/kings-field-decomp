"""Run with `nix develop --command python3 -m unittest discover -s tests`."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class NativeRegressions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="kf-native-regressions-")
        cls.addClassCleanup(cls.directory.cleanup)
        for name in ["native_regressions", "lighting_regressions", "resource_failures",
                     "cutscene_resources", "collision_results", "map_grids", "menu_outcomes", "dialogue_compare",
                     "keyboard_controls", "cell_windows", "vertex_sources", "asset_lifetimes", "actor_definitions", "player_movement",
                     "effect_construction", "effect_exhaustion"]:
            subprocess.run(
                [
                    "clang++", "-std=c++20", "-O1", "-g", "-fno-rtti",
                    "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",
                    "-fsanitize=address,undefined", "-ftrivial-auto-var-init=pattern",
                    "-I", str(ROOT / "include"), str(ROOT / "tests" / f"{name}.cpp"),
                    *([str(ROOT / source) for source in ("src/platform/language.cpp", "src/lib/resources.cpp")]
                      if name == "native_regressions" else []),
                    *([str(ROOT / source) for source in ("src/audio/codec.cpp", "src/renderer/tim.cpp",
                                                        "src/lib/resource_decode.cpp")]
                      if name == "cutscene_resources" else []),
                    *([str(ROOT / "src/lib/resource_decode.cpp")]
                      if name in ("asset_lifetimes", "effect_exhaustion") else []),
                    *([str(ROOT / source) for source in ("src/lib/resource_file.cpp", "src/platform/files.cpp",
                                                        "src/platform/language_runtime.cpp", "src/platform/language.cpp")]
                      if name == "dialogue_compare" else []),
                    "-o", str(Path(cls.directory.name) / name),
                ],
                check=True, text=True,
            )
        cls.binary = Path(cls.directory.name) / "native_regressions"

    def test_effect_construction_contracts(self):
        subprocess.run([Path(self.directory.name) / "effect_construction"], check=True, timeout=10)

    def test_effect_pool_exhaustion(self):
        subprocess.run([Path(self.directory.name) / "effect_exhaustion"], check=True, timeout=10)

    def test_map_grid_copy_alignment_and_bounds(self):
        for scenario in ("aligned", "unaligned", "truncated", "zero-orientation", "high-orientation"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([Path(self.directory.name) / "map_grids", scenario],
                                        capture_output=True, text=True, timeout=10)
                invalid = scenario not in ("aligned", "unaligned")
                self.assertEqual(result.returncode, 77 if invalid else 0,
                                 result.stderr)
                error = "Truncated map grids\n" if scenario == "truncated" else "Invalid map cell orientation\n"
                self.assertEqual(result.stderr, error if invalid else "")

    def test_cell_window_resource_boundaries(self):
        for scenario in ("valid", "unaligned", "truncated", "zero-width", "zero-height",
                         "oversized", "wide", "origin", "visibility"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([Path(self.directory.name) / "cell_windows", scenario],
                                        capture_output=True, text=True, timeout=10)
                self.assertEqual(result.returncode, 0 if scenario in ("valid", "unaligned") else 77,
                                 result.stderr)

    def test_selected_vertex_source_bounds_and_lifetime(self):
        for scenario in ("valid", "short-source", "negative", "capacity", "select", "register", "release"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([Path(self.directory.name) / "vertex_sources", scenario],
                                        capture_output=True, text=True, timeout=10)
                self.assertEqual(result.returncode, 0 if scenario == "valid" else 77, result.stderr)

    def test_asset_replacement_and_floor_lifetimes(self):
        for scenario in ("valid", "truncated", "overflow"):
            with self.subTest(scenario=scenario):
                result = subprocess.run([Path(self.directory.name) / "asset_lifetimes", scenario],
                                        capture_output=True, text=True, timeout=10)
                self.assertEqual(result.returncode, 0 if scenario == "valid" else 77, result.stderr)

    def test_cutscene_resource_boundaries(self):
        binary = Path(self.directory.name) / "cutscene_resources"
        for consumer in ["lit", "map", "unlit", "placements"]:
            cases = ["valid", "truncated"]
            cases += (["unterminated", "outside-grid", "unaligned", "fixed-height"] if consumer == "placements"
                      else ["short-layout", "vertex"])
            if consumer in ["lit", "map"]:
                cases.append("normal")
            for case in cases:
                with self.subTest(consumer=consumer, case=case):
                    result = subprocess.run([binary, consumer, case], capture_output=True,
                                            text=True, timeout=10)
                    self.assertEqual(result.returncode, 0 if case in ("valid", "unaligned", "fixed-height") else 77,
                                     result.stderr)

    def test_menu_outcomes_and_session_restoration(self):
        subprocess.run([Path(self.directory.name) / "menu_outcomes"], check=True)

    def test_keyboard_layouts_and_remapped_keys(self):
        subprocess.run([Path(self.directory.name) / "keyboard_controls"], check=True)

    def test_dialogue_comparison_preserves_page_language_and_resource_root(self):
        subprocess.run([Path(self.directory.name) / "dialogue_compare", self.directory.name], check=True)

    def test_collision_results_and_door_probes(self):
        subprocess.run([Path(self.directory.name) / "collision_results"], check=True)

    def test_actor_definition_attachments_and_shared_parameters(self):
        subprocess.run([Path(self.directory.name) / "actor_definitions"], check=True)

    def test_player_movement_at_world_boundaries(self):
        subprocess.run([Path(self.directory.name) / "player_movement"], check=True)

    def test_lighting_preserves_translation_and_supports_aliasing(self):
        subprocess.run([Path(self.directory.name) / "lighting_regressions"], check=True)

    def test_required_file_failure_reaches_host_error_handler(self):
        result = subprocess.run(
            [Path(self.directory.name) / "resource_failures", "required", self.directory.name],
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(result.returncode, 77, result.stderr)
        self.assertEqual(result.stderr, "Resource missing.dat: not found\n"
                         "Cannot load required resource KF/missing.dat.\n")

    def test_optional_file_failure_still_returns_to_caller(self):
        result = subprocess.run(
            [Path(self.directory.name) / "resource_failures", "optional", self.directory.name],
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "Resource missing.dat: not found\n")

    def run_case(self, mode, code, message=""):
        result = subprocess.run([self.binary, mode], capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, code, result.stderr)
        self.assertEqual(result.stderr, message)

    def test_original_common_table_spans(self):
        self.run_case("valid", 0)

    def test_truncated_armor_file(self):
        self.run_case("armor-truncated", 1,
                      "Truncated COM/COM.DAT armor table: need 1176 bytes, have 1175.\n")

    def test_truncated_object_file(self):
        self.run_case("objects-truncated", 1,
                      "Truncated COM/COM.DAT map-object table: need 1280 bytes, have 1279.\n")

    def test_ordinary_records_remain_chunk_bounded(self):
        self.run_case("chunk-only", 1, "Truncated armor chunk: need 1176 bytes, have 756.\n")

    def test_alignment_check(self):
        self.run_case("unaligned", 1, "Unaligned test record: requires 4-byte alignment.\n")

    def test_stream_tail_still_validates_chunk_header(self):
        self.run_case("invalid-header", 1, "Truncated resource chunk payload\n")

    def test_reset_scalars_arrays_nested_arrays_and_pointers(self):
        self.run_case("reset", 0)


if __name__ == "__main__":
    unittest.main()
