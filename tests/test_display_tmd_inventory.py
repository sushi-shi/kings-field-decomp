from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


PAIRS = (
    (
        "GAME.EXE",
        0x8001B7B0,
        "display_show_error_screen",
        "void",
        "KfSystemScreen stage",
    ),
    (
        "GAME.EXE",
        0x8001BAB8,
        "lighting_set_active_color_matrix",
        "void",
        "KfGameColorPreset preset",
    ),
    (
        "GAME.EXE",
        0x8001BAE4,
        "effect5_texture_cache_prepare",
        "void",
        "KfFloorId floor",
    ),
    ("GAME.EXE", 0x8001BB94, "display_initialize", "void", ""),
    ("GAME.EXE", 0x8001BCE0, "render_initialize", "void", ""),
    ("GAME.EXE", 0x8001BFB8, "display_begin_frame", "void", ""),
    ("GAME.EXE", 0x8001C050, "display_present_frame", "void", ""),
    ("GAME.EXE", 0x8001C0E8, "tmd_select", "void", "KfTmdSlot slot"),
    ("GAME.EXE", 0x8001C114, "tmd_get_object", "KfTmdObject *", "u16 object_index"),
    ("GAME.EXE", 0x8001C138, "tmd_set_current_vertices", "void", "SVECTOR *vertices"),
    ("GAME.EXE", 0x8001C148, "tmd_select_object_vertices", "void", "u16 object_index"),
    (
        "GAME.EXE",
        0x8001C184,
        "render_set_view_transform",
        "void",
        "const VECTOR *position;const SVECTOR *rotation",
    ),
    ("GAME.EXE", 0x8001C2B0, "tmd_prepare_primitive_indices", "void", ""),
    ("GAME.EXE", 0x8001C5B0, "tmd_register", "void", "KfTmdSlot slot;u8 *tmd"),
    ("GAME.EXE", 0x8001C5EC, "tmd_release_last_allocation", "void", "KF_ENUM_PARAM(KfTmdSlot, s32) slot"),
    ("OPEN.EXE", 0x80016908, "render_initialize", "void", ""),
    ("OPEN.EXE", 0x80016D38, "display_begin_frame", "void", ""),
    ("OPEN.EXE", 0x80016DD0, "display_present_frame", "void", ""),
    ("OPEN.EXE", 0x80016E68, "tmd_select", "void", "KfTmdSlot slot"),
    ("OPEN.EXE", 0x80016E94, "tmd_get_object", "KfTmdObject *", "u16 object_index"),
    ("OPEN.EXE", 0x80016EB8, "tmd_set_current_vertices", "void", "SVECTOR *vertices"),
    ("OPEN.EXE", 0x80016EC8, "tmd_select_object_vertices", "void", "u16 object_index"),
    (
        "OPEN.EXE",
        0x80016F04,
        "render_set_view_transform",
        "void",
        "const VECTOR *position;const SVECTOR *rotation",
    ),
    ("OPEN.EXE", 0x80017030, "tmd_prepare_primitive_indices", "void", ""),
    ("OPEN.EXE", 0x80017330, "tmd_register", "void", "KfTmdSlot slot;u8 *tmd"),
    ("OPEN.EXE", 0x8001736C, "tmd_release_last_allocation", "void", "KF_ENUM_PARAM(KfTmdSlot, s32) slot"),
)


STATE = {
    "GAME.EXE": {
        0x80070E98: ("game_graphics_runtime", 0x249CC, "KfGraphicsRuntimeGame"),
    },
    "OPEN.EXE": {
        0x80049A48: ("open_graphics_runtime", 0x24788, "KfGraphicsRuntimeOpen"),
    },
}


class DisplayTmdInventoryTests(unittest.TestCase):
    def test_campaign_matches_function_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_display_tmd.tsv"
        _fields, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), len(PAIRS))
        for row, (image, va, name, return_type, parameters) in zip(rows, PAIRS):
            self.assertEqual((row["image"], parse_int(row["va"])), (image, va))
            identity = identities[(image, va)]
            self.assertEqual(
                (identity.name, identity.return_type, identity.parameters),
                (name, return_type, parameters),
            )
            rendered = ", ".join(parameters.split(";")) or "void"
            self.assertEqual(row["final_signature"], f"{return_type} {name}({rendered})")
            self.assertIn(evidence_path.name, identity.evidence)

    def test_complete_structures_have_checked_fields(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        fields = load_structure_field_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfPrimitiveBuffer"].size, 0x0C)
        self.assertEqual(structures["KfTmdObject"].size, 0x1C)
        primitive = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfPrimitiveBuffer"
        }
        self.assertEqual(
            primitive,
            {
                "start": (0x00, 4, "u8 *"),
                "end": (0x04, 4, "u8 *"),
                "cursor": (0x08, 4, "u8 *"),
            },
        )
        tmd = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfTmdObject"
        }
        self.assertEqual(tmd["vertex_offset"], (0x00, 4, "u32"))
        self.assertEqual(tmd["primitive_offset"], (0x10, 4, "u32"))
        self.assertEqual(tmd["primitive_count"], (0x14, 4, "u32"))
        self.assertNotIn("primitive_count_high", tmd)
        self.assertEqual(tmd["scale"], (0x18, 4, "s32"))
        header = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfTmdHeader"
        }
        self.assertEqual(header["object_count"], (0x08, 4, "u32"))
        self.assertNotIn("object_count_high", header)

    def test_image_qualified_state_owns_interior_addresses(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        for image, expected in STATE.items():
            for va, shape in expected.items():
                row = identities[(image, va)]
                self.assertEqual((row.name, row.size, row.datatype), shape)

    def test_textured_packet_colors_own_their_code_fields(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        expected = {
            0x80057B58: "tmd_textured_primitive_color",
            0x80057B5C: "model_textured_primitive_color",
        }
        for va, name in expected.items():
            row = identities[("GAME.EXE", va)]
            self.assertEqual((row.name, row.size, row.datatype), (name, 4, "CVECTOR"))
            self.assertEqual(row.storage, "load")
            self.assertEqual(row.confidence, "supported")
        self.assertNotIn(("GAME.EXE", 0x80057B5B), identities)
        self.assertNotIn(("GAME.EXE", 0x80057B5F), identities)

        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        references = [
            row
            for row in rows
            if row["image"] == "GAME.EXE"
            and parse_int(row["target_va"]) in (0x80057B5B, 0x80057B5F)
        ]
        self.assertEqual(len(references), 10)
        for row in references:
            owner_va = parse_int(row["target_va"]) - 3
            self.assertEqual(row["target_name"], expected[owner_va])
            self.assertEqual(row["status"], "reviewed")

    def test_active_material_names_preserve_sdk_widths_and_referents(self) -> None:
        expected = {
            0x80095058: ("active_render_clut", 2, "u16"),
            0x8009505A: ("active_render_tpage", 2, "u16"),
            0x8009505C: ("active_render_red", 1, "u8"),
            0x8009505D: ("active_render_green", 1, "u8"),
            0x8009505E: ("active_render_blue", 1, "u8"),
            0x8009505F: ("active_render_code", 1, "u8"),
        }
        identities = load_data_identities(RETAIL_CONFIG)
        fields = {0x80070E98 + row.offset: row
                  for row in load_structure_field_identities(RETAIL_CONFIG)
                  if row.structure == 'KfGraphicsRuntimeGame'}
        for va, shape in expected.items():
            self.assertNotIn(("GAME.EXE", va), identities)
            field = fields[va]
            self.assertEqual((field.name, field.size, field.datatype), shape)
            self.assertIn("game_semantic_render_material.tsv", field.evidence)

        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        references = [
            row for row in rows
            if row["image"] == "GAME.EXE"
            and 0x80095058 <= parse_int(row["target_va"]) < 0x80095060
        ]
        self.assertEqual(len(references), 27)
        for row in references:
            self.assertEqual(row["target_name"], "game_graphics_runtime")
            self.assertEqual(row["status"], "reviewed")

    def test_decoded_relocations_and_direct_calls_are_reviewed(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        display_rows = [
            row
            for row in rows
            if "manual:game_semantic_display_tmd" in row["provenance"].split(";")
        ]
        self.assertEqual(len(display_rows), 233)
        self.assertEqual({row["status"] for row in display_rows}, {"reviewed"})
        vsync_rows = [row for row in rows if row["provenance"] == "manual:vsync-runtime"]
        self.assertEqual(len(vsync_rows), 40)
        self.assertEqual({row["status"] for row in vsync_rows}, {"reviewed"})

        targets = {(image, va): name for image, va, name, _ret, _args in PAIRS}
        calls = [
            row
            for row in rows
            if row["kind"] == "mips26"
            and row["channel"] == "reachable-code"
            and (row["image"], parse_int(row["target_va"])) in targets
        ]
        self.assertEqual(len(calls), 85)
        for row in calls:
            key = row["image"], parse_int(row["target_va"])
            self.assertEqual(row["target_name"], targets[key])
            self.assertEqual(row["status"], "reviewed")

    def test_vsync_runtime_is_excluded_as_version_skewed_libgpu(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {(row["image"], parse_int(row["va"])): row for row in rows}
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        expected = (
            ("GAME.EXE", 0x800555E0, "VSync", 0x64),
            ("GAME.EXE", 0x80055644, "", 0x1BC),
            ("OPEN.EXE", 0x800352C4, "VSync", 0x64),
            ("OPEN.EXE", 0x80035328, "", 0x1BC),
        )
        for image, va, name, size in expected:
            row = vendored[(image, va)]
            self.assertEqual(
                (row["name"], parse_int(row["size"]), row["library"], row["module"]),
                (name, size, "LIBGPU.LIB", "VSYNC"),
            )
            self.assertEqual(row["confidence"], "sdk-lineage-supported")
            self.assertNotIn((image, va), identities)


if __name__ == "__main__":
    unittest.main()
