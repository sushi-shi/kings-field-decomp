"""Linked-instruction controls for the shared OPEN graphics owner."""
from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import IMAGE_LAYOUTS


FIXTURES = REPO / "tests/fixtures"
HEADER = REPO / "include/kf/open_render.h"


def linked_words(obj, unit, claim, data, functions):
    """Resolve this candidate's text only; do not execute it or mask addends."""
    fn = obj.named_symbol(claim.symbol)
    words = list(struct.unpack_from(f"<{fn.size // 4}I", obj.sections[".text"], fn.value))
    pending = []
    calls, addresses = [], []
    for reloc in obj.relocations:
        if reloc.section != ".text" or not fn.value <= reloc.offset < fn.value + fn.size:
            continue
        index = (reloc.offset - fn.value) // 4
        symbol = obj.symbol(reloc.symbol_index)
        if reloc.kind == 5:
            pending.append((index, symbol.index))
        elif reloc.kind == 6:
            assert len(pending) == 1
            high, symbol_index = pending.pop()
            assert symbol_index == symbol.index
            base = unit.rodata[0] if symbol.section == ".rodata" else data[symbol.name]
            target = (base + decode_hi_lo_target(words[high], words[index])) & 0xFFFFFFFF
            addresses.append(target)
            words[high], words[index] = encode_hi_lo_addend(words[high], words[index], target)
        elif reloc.kind == 4:
            addend = (words[index] & 0x3FFFFFF) * 4
            if symbol.name in functions:
                target = functions[symbol.name] + addend
            else:
                assert symbol.section == ".text"
                offset = symbol.value + addend
                assert fn.value <= offset < fn.value + fn.size
                target = claim.va + offset - fn.value
            if words[index] >> 26 == 3:
                calls.append(target)
            words[index] = words[index] & 0xFC000000 | (target >> 2 & 0x3FFFFFF)
        else:
            raise AssertionError(reloc)
    assert not pending
    return words, calls, addresses


class OpenRuntimeOwnerProbeTests(unittest.TestCase):
    def tools(self):
        cpp, cc1 = shutil.which("cpppsx-257"), shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and SDK headers are required")
        return cpp, cc1, sdk

    def test_complete_region_layout_and_wrong_gap_control(self):
        cpp, cc1, sdk = self.tools()
        original = HEADER.read_text()
        with TemporaryDirectory(prefix="kf-open-runtime-layout-") as directory:
            root = Path(directory)
            for gap in (0x1F68, 0x1F64):
                with self.subTest(gap=gap):
                    source = root / "layout.c"
                    source.write_text(original.replace("[0x1f68]", f"[{gap:#x}]"))
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", sdk, str(source)], capture_output=True, check=True,
                    )
                    intermediate = root / "layout.i"
                    intermediate.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(intermediate),
                         "-o", str(root / "layout.s")], capture_output=True, text=True,
                    )
                    if gap == 0x1F68:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("check_floor_item_state", result.stderr)

    def test_initializer_and_allocator_retain_every_linked_instruction(self):
        self.check_exact_consumers("open.render_init", {
            "lighting_set_active_color_matrix", "render_initialize", "primitive_buffer_allocate",
        })

    def test_display_tmd_and_projection_retain_every_linked_instruction(self):
        self.check_exact_consumers("open.render", {
            "display_begin_frame", "display_present_frame", "tmd_select", "tmd_get_object",
            "tmd_set_current_vertices", "tmd_select_object_vertices", "render_set_view_transform",
            "tmd_register", "tmd_release_last_allocation", "tmd_project_vertices",
            "tmd_project_vertices_perspective_right", "tmd_project_vertices_shift",
            "tmd_transform_vertices",
        })

    def test_remaining_banked_consumers_retain_every_linked_instruction(self):
        controls = {
            "open.opening_render": {"opening_render_frame", "sprite_add_g4"},
            "open.sprite_add_ft4": {"sprite_add_f4"},
            "open.opening_scenes": {
                "opening_scene1_draw_fade", "opening_scene1_run", "opening_ending_scene_run",
            },
            "open.opening_controller": {"opening_run"},
            "open.render_sprite": {"render_enqueue_sprite"},
            "open.entity_render": {"opening_entity_render"},
            "open.matrix": {
                "matrix_interpolate", "lighting_set_color_matrix", "lighting_set_light_matrix",
                "fog_set_near", "color_lerp_cvector", "color_lerp_rgb555",
            },
        }
        for unit_name, functions in controls.items():
            with self.subTest(unit=unit_name):
                self.check_exact_consumers(unit_name, functions)

    def check_exact_consumers(self, unit_name, controls):
        self.tools()
        try:
            retail = (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail OPEN image is required")
        manifest = load_manifest()
        unit = manifest.by_name()[unit_name]
        if not (BUILD / "delink/open/modules" / unit.object_name).is_file():
            self.skipTest(f"delinked {unit_name} target is required")
        profile = manifest.profiles[unit.profile]
        source = unit.source_path.read_text()
        data = {item.name: item.va for (image, _), item in
                load_data_identities(RETAIL_CONFIG).items() if image == "OPEN.EXE"}
        data["open_graphics_runtime"] = 0x80049A48
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["OPEN.EXE"]}
        with TemporaryDirectory(prefix="kf-open-runtime-code-") as directory:
            root = Path(directory)
            candidate = root / unit.source_path.name
            candidate.write_text(source)
            output = root / unit.object_name
            compile_source(
                candidate, unit.image, output, BUILD / "delink", profile.optimization,
                profile.small_data, profile.aspsx_version,
                (REPO / "include", FIXTURES, Path(os.environ["PSYQ_INCLUDE"])),
                profile.cc1_flags, profile.compiler, profile.maspsx_flags, defines=unit.defines,
            )
            obj = _load_object(output)
            self.assertTrue(controls <= {claim.symbol for claim in unit.functions})
            for claim in unit.functions:
                if claim.symbol not in controls:
                    continue  # Partial bodies have separate evidence, not exact assertions.
                with self.subTest(function=claim.symbol):
                    actual, _, _ = linked_words(obj, unit, claim, data, functions)
                    offset = IMAGE_LAYOUTS[unit.image].file_offset(claim.va)
                    expected = list(struct.unpack_from(
                        f"<{claim.body_size // 4}I", retail, offset))
                    self.assertEqual(actual, expected)
                    if claim.symbol in {"render_initialize", "display_begin_frame"}:
                        wrong_owner = dict(data, open_graphics_runtime=0x80049A4C)
                        corrupted, _, _ = linked_words(obj, unit, claim, wrong_owner, functions)
                        self.assertNotEqual(corrupted, expected)

    def test_map_traversal_and_shared_matrix_base(self):
        self.tools()
        try:
            retail = (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail OPEN image is required")
        manifest = load_manifest()
        unit = manifest.by_name()["open.render_map_cells"]
        if not (BUILD / "delink/open/modules" / unit.object_name).is_file():
            self.skipTest("delinked OPEN map target is required")
        profile = manifest.profiles[unit.profile]
        source = unit.source_path.read_text()
        data = {item.name: item.va for (image, _), item in
                load_data_identities(RETAIL_CONFIG).items() if image == "OPEN.EXE"}
        data["open_graphics_runtime"] = 0x80049A48
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["OPEN.EXE"]}
        with TemporaryDirectory(prefix="kf-open-runtime-map-") as directory:
            root = Path(directory)
            candidate = root / "render_map_cells.c"
            candidate.write_text(source)
            output = root / unit.object_name
            compile_source(
                candidate, unit.image, output, BUILD / "delink", profile.optimization,
                profile.small_data, profile.aspsx_version,
                (REPO / "include", FIXTURES, Path(os.environ["PSYQ_INCLUDE"])),
                profile.cc1_flags, profile.compiler, profile.maspsx_flags, defines=unit.defines,
            )
            obj = _load_object(output)
            self.assertEqual({claim.symbol for claim in unit.functions},
                             {"render_map_cell", "opening_render_map_cells"})
            for claim in unit.functions:
                with self.subTest(function=claim.symbol):
                    actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                    offset = IMAGE_LAYOUTS[unit.image].file_offset(claim.va)
                    expected = list(struct.unpack_from(
                        f"<{claim.body_size // 4}I", retail, offset))
                    if claim.symbol == "opening_render_map_cells":
                        self.assertEqual(actual, expected)
                        self.assertEqual(calls, [0x80016E68, 0x80018BBC])
                        wrong_owner = dict(data, open_graphics_runtime=0x80049A4C)
                        corrupted, _, _ = linked_words(obj, unit, claim, wrong_owner, functions)
                        self.assertNotEqual(corrupted, expected)
                    else:
                        # Only the matrix-base sequence and epilogue are exact.
                        self.assertNotEqual(actual, expected)
                        self.assertEqual(actual[-24:], expected[-24:])
                        self.assertEqual(addresses, [
                            0x800730A0, 0x80046DF8, 0x8006E0AC, 0x8006E0B4,
                            0x8006E260, 0x8006E0B0, 0x8006E048, 0x8006E048,
                            0x8006E0C8,
                        ])
                        self.assertEqual(calls, [
                            0x8002D558, 0x8002D5E8, 0x8002D8B0, 0x8002CBA8,
                            0x8002D558, 0x8002D5E8, 0x8002D588, 0x80016EC8, 0x800185E8,
                        ])


if __name__ == "__main__":
    unittest.main()
