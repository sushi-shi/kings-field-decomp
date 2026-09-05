from __future__ import annotations

import struct
import unittest

from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


# OPEN object IDs 11 through 27, in their encoded table order.
SWITCH_TARGETS = (
    0x80019000, 0x80019000, 0x80019000, 0x80019000,
    0x800190A4, 0x800190A4, 0x800190A4, 0x800190A4, 0x800190A4,
    0x80019000, 0x80019008, 0x80019008, 0x80019014, 0x80019014,
    0x8001902C, 0x80019060, 0x80019068,
)


class OpenEntityRenderTests(unittest.TestCase):
    def test_entity_signature_and_switch_ownership(self) -> None:
        evidence = CONFIG / "evidence/open_semantic_entity_render.tsv"
        _, rows = read_tsv(evidence)
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual((row["image"], parse_int(row["va"])),
                         ("OPEN.EXE", 0x80018ECC))
        self.assertEqual(parse_int(row["size"]), 0x228)
        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("OPEN.EXE", 0x80018ECC)
        ]
        self.assertEqual(
            (identity.name, identity.return_type, identity.parameters),
            ("opening_entity_render", "void", "KfOpeningEntity *entity"),
        )
        self.assertIn(evidence.name, identity.evidence)
        self.assertEqual(row["final_signature"],
                         "void opening_entity_render(KfOpeningEntity *entity)")
        unit = load_manifest().by_name()["open.entity_render"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual(unit.rodata, (0x80012238, 0x44))
        data = load_data_identities(RETAIL_CONFIG)
        self.assertFalse(any(image == "OPEN.EXE" and 0x80012238 <= va < 0x8001227C
                             for image, va in data))

    def test_reviewed_table_and_body_referents(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        by_site = {parse_int(row["site_va"]): row for row in rows
                   if row["image"] == "OPEN.EXE"}
        for i, target in enumerate(SWITCH_TARGETS):
            row = by_site[0x80012238 + 4 * i]
            self.assertEqual(parse_int(row["target_va"]), target)
            self.assertEqual(row["status"], "reviewed")
            self.assertEqual(row["confidence"], "pointer-reviewed")
            self.assertIn("manual:open_semantic_entity_render", row["provenance"])

        body = {site: row for site, row in by_site.items()
                if 0x80018ECC <= site < 0x800190F4}
        self.assertEqual(len(body), 34)
        self.assertEqual({row["status"] for row in body.values()}, {"reviewed"})
        expected = {
            0x80018EE4: (0x8006E048, "render_state"),
            0x80018EF4: (0x8006E048, "render_state"),
            0x80018F04: (0x8006E0AC, "render_state"),
            0x80018F24: (0x8006E0B0, "render_state"),
            0x80018F3C: (0x8006E0B4, "render_state"),
            0x80018FE4: (0x80012238, ""),
            0x80019044: (0x80017458, "tmd_project_vertices_perspective_right"),
            0x80019050: (0x8001764C, "render_enqueue_tmd"),
            0x80019084: (0x8001738C, "tmd_project_vertices"),
            0x80019094: (0x80018344, "render_enqueue_unlit_triangles"),
            0x800190C0: (0x8001738C, "tmd_project_vertices"),
            0x800190D0: (0x8001764C, "render_enqueue_tmd"),
        }
        for site, target in expected.items():
            row = body[site]
            self.assertEqual((parse_int(row["target_va"]), row["target_name"]), target)

    def test_retail_switch_targets(self) -> None:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        path = retail / "OPEN.EXE"
        if not path.is_file():
            self.skipTest("retail OPEN.EXE is unavailable")
        data = path.read_bytes()
        load_address = struct.unpack_from("<I", data, 0x18)[0]
        self.assertEqual(load_address, 0x80012000)
        offset = 0x80012238 - load_address + 0x800
        self.assertEqual(struct.unpack_from("<17I", data, offset), SWITCH_TARGETS)


if __name__ == "__main__":
    unittest.main()
