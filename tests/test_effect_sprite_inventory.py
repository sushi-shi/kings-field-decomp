from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    load_data_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class EffectSpriteInventoryTests(unittest.TestCase):
    def test_complete_effect_sprite_layout(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        fields = load_structure_field_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfEffectSprite"].size, 0x1C)
        actual = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfEffectSprite"
        }
        self.assertEqual(
            actual,
            {
                "state": (0x00, 1, "KfEffectSpriteState"),
                "animation_clip": (0x01, 1, "KfAnimationClip"),
                "asset_variant": (0x02, 2, "u16"),
                "scale": (0x04, 2, "u16"),
                "translation_x": (0x06, 2, "s16"),
                "translation_y": (0x08, 2, "s16"),
                "translation_z": (0x0A, 2, "s16"),
                "unknown_0c": (0x0C, 2, "u8[2]"),
                "rotation": (0x0E, 8, "SVECTOR"),
                "unknown_16": (0x16, 2, "u8[2]"),
                "animation_cache": (0x18, 4, "KfPoolRecord *"),
            },
        )

    def test_effect_sprite_array_owns_all_interior_identities(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        row = identities[("GAME.EXE", 0x80055D74)]
        self.assertEqual(
            (row.name, row.size, row.storage, row.datatype, row.owner),
            ("effect_sprites", 0x38, "load", "KfEffectSprite[2]", "render_frame"),
        )
        for va in (
            0x80055D75,
            0x80055D76,
            0x80055D78,
            0x80055D7A,
            0x80055D7C,
            0x80055D7E,
            0x80055D86,
            0x80055D90,
            0x80055D96,
            0x80055D98,
            0x80055D9A,
        ):
            self.assertNotIn(("GAME.EXE", va), identities)

    def test_effect_sprite_relocations_name_the_array_owner(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign = [
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_effect_sprites"
        ]
        self.assertEqual(len(campaign), 3)
        self.assertEqual({row["target_name"] for row in campaign}, {"effect_sprites"})
        self.assertEqual({row["status"] for row in campaign}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): parse_int(row["target_va"]) for row in campaign}
        self.assertEqual(by_site[0x8001F8E4], 0x80055D74)
        self.assertEqual(by_site[0x80020114], 0x80055D74)
        self.assertEqual(by_site[0x80020124], 0x80055D86)

    def test_evidence_rows_track_both_exact_consumers(self) -> None:
        _fields, rows = read_tsv(CONFIG / "evidence/game_semantic_effect_sprites.tsv")
        self.assertEqual(
            [(parse_int(row["va"]), row["current_match"]) for row in rows],
            [
                (0x8001F8B0, "100.000000% exact"),
                (0x8001FDE4, "100.000000% exact"),
            ],
        )


if __name__ == "__main__":
    unittest.main()
