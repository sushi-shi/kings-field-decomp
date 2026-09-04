from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    load_data_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class HudSpriteInventoryTests(unittest.TestCase):
    def test_complete_hud_row_layout(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        fields = load_structure_field_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfHudSprite"].size, 0x0E)
        hud = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfHudSprite"
        }
        self.assertEqual(
            hud,
            {
                "state": (0x00, 1, "u8"),
                "unknown_01": (0x01, 1, "u8"),
                "sprite": (0x02, 0x0C, "KfSpriteQuad"),
            },
        )

    def test_hud_array_owns_all_former_field_identities(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        row = identities[("GAME.EXE", 0x80055C5C)]
        self.assertEqual(
            (row.name, row.size, row.storage, row.datatype, row.owner),
            ("hud_sprites", 0xC4, "load", "KfHudSprite[14]", "render"),
        )
        for va in (
            0x80055C66,
            0x80055C6A,
            0x80055C74,
            0x80055C78,
            0x80055C82,
            0x80055C86,
            0x80055C90,
            0x80055C94,
            0x80055CA2,
            0x80055CB0,
            0x80055CBE,
            0x80055CCC,
            0x80055CDA,
            0x80055CE8,
            0x80055CF6,
            0x80055D04,
        ):
            self.assertNotIn(("GAME.EXE", va), identities)

    def test_all_interior_relocations_name_the_array_owner(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign = [
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_hud_sprites"
        ]
        self.assertEqual(len(campaign), 28)
        self.assertEqual({row["target_name"] for row in campaign}, {"hud_sprites"})
        self.assertEqual({row["status"] for row in campaign}, {"reviewed"})
        self.assertTrue(
            all(
                0x80055C5C <= parse_int(row["target_va"]) <= 0x80055D04
                for row in campaign
            )
        )

    def test_evidence_tracks_both_exact_functions(self) -> None:
        _fields, rows = read_tsv(CONFIG / "evidence/game_semantic_hud_sprites.tsv")
        self.assertEqual(
            [(parse_int(row["va"]), row["current_match"]) for row in rows],
            [
                (0x8001F9D4, "100.000000% exact"),
                (0x8001FDE4, "100.000000% exact"),
            ],
        )


if __name__ == "__main__":
    unittest.main()
