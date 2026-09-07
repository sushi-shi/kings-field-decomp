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


class NotificationSpriteInventoryTests(unittest.TestCase):
    def test_payloads_and_control_fields_share_one_nonoverlapping_owner(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        state = identities[("GAME.EXE", 0x80095076)]
        self.assertEqual(
            (state.name, state.size, state.datatype, state.storage),
            ("notification_state", 0x16, "KfNotificationState", "bss"),
        )
        interior = {
            va for image, va in identities
            if image == "GAME.EXE" and 0x80095076 <= va < 0x8009508C
        }
        self.assertEqual(interior, {0x80095076})
        fields = {
            row.name: (row.offset, row.size, row.datatype)
            for row in load_structure_field_identities(RETAIL_CONFIG)
            if row.structure == "KfNotificationState"
        }
        self.assertEqual(fields, {
            "message_payloads": (0x00, 0x10, "u16[8]"),
            "control": (0x10, 6, "KfNotificationControl"),
        })
        control = {
            row.name: (row.offset, row.size, row.datatype)
            for row in load_structure_field_identities(RETAIL_CONFIG)
            if row.structure == "KfNotificationControl"
        }
        self.assertEqual(control, {
            "queue_tail": (0x00, 1, "u8"),
            "queue_head": (0x01, 1, "u8"),
            "effect_phase": (0x02, 1, "KfNotificationPhase"),
            "hold_frames": (0x03, 1, "u8"),
            "effect_angle_x": (0x04, 2, "u16"),
        })
        structures = load_structure_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfNotificationState"].size, 0x16)
        self.assertEqual(structures["KfNotificationControl"].size, 6)
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        references = [
            row for row in rows
            if row["image"] == "GAME.EXE"
            and 0x80095076 <= parse_int(row["target_va"]) < 0x8009508C
        ]
        self.assertEqual(len(references), 17)
        self.assertEqual({row["target_name"] for row in references}, {state.name})
        self.assertEqual(
            identities[("GAME.EXE", 0x8009506E)].name,
            "notification_message_ids",
        )

    def test_enqueue_signature_preserves_the_optional_promoted_argument(self) -> None:
        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("GAME.EXE", 0x8001FA44)
        ]
        self.assertEqual(identity.parameters, "KfNotificationArgument message_id;...")

    def test_complete_sprite_layouts(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        fields = load_structure_field_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfSpriteQuad"].size, 0x0C)
        self.assertEqual(structures["KfNotificationSprite"].size, 0x0E)

        sprite = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfSpriteQuad"
        }
        self.assertEqual(
            sprite,
            {
                "u": (0x00, 1, "u8"),
                "v": (0x01, 1, "u8"),
                "u_span": (0x02, 1, "u8"),
                "v_span": (0x03, 1, "u8"),
                "x": (0x04, 2, "u16"),
                "y": (0x06, 2, "u16"),
                "w": (0x08, 2, "u16"),
                "h": (0x0A, 2, "u16"),
            },
        )
        notification = {
            row.name: (row.offset, row.size, row.datatype)
            for row in fields
            if row.structure == "KfNotificationSprite"
        }
        self.assertEqual(
            notification,
            {
                "active": (0x00, 1, "KfNotificationSpriteState"),
                "unknown_01": (0x01, 1, "u8"),
                "sprite": (0x02, 0x0C, "KfSpriteQuad"),
            },
        )

    def test_notification_array_owns_all_former_field_identities(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        row = identities[("GAME.EXE", 0x80055D20)]
        self.assertEqual(
            (row.name, row.size, row.storage, row.datatype, row.owner),
            (
                "notification_sprites",
                0x54,
                "load",
                "KfNotificationSprite[6]",
                "notify",
            ),
        )
        for va in (
            0x80055D22,
            0x80055D23,
            0x80055D2E,
            0x80055D30,
            0x80055D31,
            0x80055D3C,
            0x80055D4A,
            0x80055D58,
            0x80055D66,
        ):
            self.assertNotIn(("GAME.EXE", va), identities)

    def test_all_interior_relocations_name_the_array_owner(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign = [
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_notification_sprites"
        ]
        self.assertEqual(len(campaign), 24)
        self.assertEqual(
            {row["target_name"] for row in campaign}, {"notification_sprites"}
        )
        self.assertEqual({row["status"] for row in campaign}, {"reviewed"})
        self.assertEqual(
            {parse_int(row["target_va"]) for row in campaign},
            {
                0x80055D20,
                0x80055D22,
                0x80055D23,
                0x80055D2E,
                0x80055D30,
                0x80055D31,
                0x80055D3C,
                0x80055D4A,
                0x80055D58,
                0x80055D66,
            },
        )

    def test_evidence_tracks_producer_and_exact_consumer(self) -> None:
        _fields, rows = read_tsv(
            CONFIG / "evidence/game_semantic_notification_sprites.tsv"
        )
        self.assertEqual(
            [(parse_int(row["va"]), row["current_match"]) for row in rows],
            [
                (0x8001FAFC, "98.296090% fuzzy"),
                (0x8001FDE4, "100.000000% exact"),
            ],
        )


if __name__ == "__main__":
    unittest.main()
