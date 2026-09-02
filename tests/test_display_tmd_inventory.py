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
    ("GAME.EXE", 0x8001BCE0, "render_initialize", "void", ""),
    ("GAME.EXE", 0x8001BFB8, "display_begin_frame", "void", ""),
    ("GAME.EXE", 0x8001C050, "display_present_frame", "void", ""),
    ("GAME.EXE", 0x8001C0E8, "tmd_select", "void", "u16 slot"),
    ("GAME.EXE", 0x8001C114, "tmd_get_object", "KfTmdObject *", "u16 object_index"),
    ("GAME.EXE", 0x8001C138, "tmd_set_current_vertices", "void", "KfVec4s *vertices"),
    ("GAME.EXE", 0x8001C148, "tmd_select_object_vertices", "void", "u16 object_index"),
    (
        "GAME.EXE",
        0x8001C184,
        "render_set_view_transform",
        "void",
        "const KfVec4i *position;const KfVec4s *rotation",
    ),
    ("GAME.EXE", 0x8001C2B0, "tmd_prepare_primitive_indices", "void", ""),
    ("GAME.EXE", 0x8001C5B0, "tmd_register", "void", "u16 slot;u8 *tmd"),
    ("GAME.EXE", 0x8001C5EC, "tmd_release_last_allocation", "void", "s32 slot"),
    ("OPEN.EXE", 0x80016908, "render_initialize", "void", ""),
    ("OPEN.EXE", 0x80016D38, "display_begin_frame", "void", ""),
    ("OPEN.EXE", 0x80016DD0, "display_present_frame", "void", ""),
    ("OPEN.EXE", 0x80016E68, "tmd_select", "void", "u16 slot"),
    ("OPEN.EXE", 0x80016E94, "tmd_get_object", "KfTmdObject *", "u16 object_index"),
    ("OPEN.EXE", 0x80016EB8, "tmd_set_current_vertices", "void", "KfVec4s *vertices"),
    ("OPEN.EXE", 0x80016EC8, "tmd_select_object_vertices", "void", "u16 object_index"),
    (
        "OPEN.EXE",
        0x80016F04,
        "render_set_view_transform",
        "void",
        "const KfVec4i *position;const KfVec4s *rotation",
    ),
    ("OPEN.EXE", 0x80017030, "tmd_prepare_primitive_indices", "void", ""),
    ("OPEN.EXE", 0x80017330, "tmd_register", "void", "u16 slot;u8 *tmd"),
    ("OPEN.EXE", 0x8001736C, "tmd_release_last_allocation", "void", "s32 slot"),
)


STATE = {
    "GAME.EXE": {
        0x80070E98: ("display_state", 0x20028, "KfDisplayState"),
        0x80090EC0: ("display_draw_environments", 0xB8, "DRAWENV[2]"),
        0x80090F78: ("display_disp_environments", 0x28, "DISPENV[2]"),
        0x80090FA8: ("tmd_state", 0x24, "KfTmdState"),
        0x800910BC: ("current_tmd_vertices", 0x04, "KfVec4s *"),
        0x800956A0: ("render_state", 0x140, "KfRenderState"),
        0x800957E0: ("light_quadrant_matrices", 0x80, "KfMatrix[4]"),
    },
    "OPEN.EXE": {
        0x80049A50: ("primitive_buffers", 0x18, "KfPrimitiveBuffer[2]"),
        0x80049A68: ("primitive_buffer", 0x04, "KfPrimitiveBuffer *"),
        0x80069A70: ("display_draw_environments", 0xB8, "DRAWENV[2]"),
        0x80069B28: ("display_disp_environments", 0x28, "DISPENV[2]"),
        0x80069B58: ("tmd_slots", 0x08, "u8 *[2]"),
        0x80069B68: ("current_tmd_vertices", 0x04, "KfVec4s *"),
        0x8006E0AC: ("render_view_position", 0x10, "KfVec4i"),
        0x8006E0BC: ("render_view_rotation", 0x08, "KfVec4s"),
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
        self.assertEqual(tmd["primitive_count"], (0x14, 2, "u16"))
        self.assertEqual(tmd["primitive_count_high"], (0x16, 2, "u16"))
        self.assertEqual(tmd["scale"], (0x18, 4, "s32"))

    def test_image_qualified_state_owns_interior_addresses(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        for image, expected in STATE.items():
            for va, shape in expected.items():
                row = identities[(image, va)]
                self.assertEqual((row.name, row.size, row.datatype), shape)

    def test_decoded_relocations_and_direct_calls_are_reviewed(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        display_rows = [
            row for row in rows if row["provenance"] == "manual:game_semantic_display_tmd"
        ]
        self.assertEqual(len(display_rows), 229)
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
        self.assertEqual(len(calls), 70)
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
