from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    _data_access,
    _ghidra_type,
    _signature_hints,
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
    validate,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.index import index


def _structure_field(structure: str, offset: int) -> tuple[str, str, int]:
    """(name, datatype, size) of one checked layout field from structure_fields.tsv."""
    _, rows = read_tsv(RETAIL_CONFIG / "structure_fields.tsv")
    for row in rows:
        if row["structure"] == structure and parse_int(row["offset"]) == offset:
            return row["name"], row["datatype"], parse_int(row["size"])
    raise AssertionError(f"{structure} has no field at {offset:#x}")


def words(*values: int) -> bytes:
    return b"".join(value.to_bytes(4, "little") for value in values)


class FakeImage:
    def __init__(self, word: int):
        self.word = word

    def u32(self, _va: int) -> int:
        return self.word


class FakeReference:
    def __init__(self, kind: str = "address", paired_site: int | None = 0x1004):
        self.kind = kind
        self.paired_site = paired_site


class InventoryTests(unittest.TestCase):
    def test_curated_inventories_cover_the_wip_universe(self) -> None:
        counts = validate(RETAIL_CONFIG)
        self.assertEqual(counts["functions"], 734)
        self.assertEqual(counts["signatures_started"], 734)
        self.assertEqual(counts["typed_returns"], 734)
        self.assertEqual(counts["parameterized"], 495)
        self.assertEqual(counts["data"], 3322)
        self.assertGreaterEqual(counts["functions_named"], 240)
        self.assertGreaterEqual(counts["data_named"], 97)
        self.assertEqual(counts["structures"], 55)
        self.assertEqual(counts["structure_fields"], 478)
        self.assertEqual(counts["structure_fields_named"], 397)

    def test_structure_inventory_exposes_sizes_offsets_and_opaque_ranges(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        fields = load_structure_field_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfActor"].size, 0x48)
        self.assertEqual(structures["KfPlayerLevelGrowth"].size, 0x0C)
        self.assertEqual(structures["KfMapCell"].size, 0x02)
        self.assertEqual(structures["KfPlayerMotionState"].size, 0x0A)
        self.assertEqual(structures["KfWeaponRecord"].size, 0x2C)
        self.assertEqual(structures["KfCollisionTarget"].size, 0x20)
        self.assertEqual(structures["KfPlayerState"].size, 0xE0)
        self.assertEqual(structures["KfPrimitiveBuffer"].size, 0x0C)
        self.assertEqual(structures["KfTmdObject"].size, 0x1C)
        growth_fields = {
            row.name: (row.offset, row.size, row.datatype, row.meaning_confidence)
            for row in fields
            if row.structure == "KfPlayerLevelGrowth"
        }
        self.assertEqual(
            growth_fields["experience_threshold"],
            (0x08, 4, "u32", "supported"),
        )
        actor_fields = {
            row.name: row for row in fields if row.structure == "KfActor"
        }
        self.assertEqual(actor_fields["position"].offset, 0x1C)
        self.assertEqual(actor_fields["unknown_28"].meaning_confidence, "opaque")
        definition_fields = {
            row.name: row for row in fields if row.structure == "KfActorDefinition"
        }
        self.assertEqual(definition_fields["experience_reward"].offset, 0x84)
        motion_fields = {
            row.name: row for row in fields if row.structure == "KfPlayerMotionState"
        }
        self.assertEqual(motion_fields["movement_speed"].offset, 0x04)
        self.assertEqual(motion_fields["pitch_step"].offset, 0x08)
        weapon_fields = {
            row.name: row for row in fields if row.structure == "KfWeaponRecord"
        }
        self.assertEqual(weapon_fields["charge_rate"].offset, 0x01)
        self.assertEqual(weapon_fields["attack_components"].size, 0x0A)
        self.assertEqual(weapon_fields["attack_z_offset"].offset, 0x12)
        self.assertEqual(weapon_fields["unknown_14"].meaning_confidence, "opaque")
        self.assertEqual(weapon_fields["mirrored_angle"].offset, 0x26)
        self.assertEqual(
            weapon_fields["mirrored_angle"].meaning_confidence,
            "candidate",
        )
        collision_fields = {
            row.name: row for row in fields if row.structure == "KfCollisionTarget"
        }
        self.assertEqual(collision_fields["rotation"].offset, 0x10)
        self.assertEqual(collision_fields["radius"].offset, 0x18)
        self.assertEqual(
            collision_fields["unknown_1a"].meaning_confidence,
            "opaque",
        )
        player_state_fields = {
            row.name: row for row in fields if row.structure == "KfPlayerState"
        }
        self.assertEqual(player_state_fields["vitals"].offset, 0x10)
        self.assertEqual(player_state_fields["camera_position"].offset, 0xA4)
        self.assertEqual(player_state_fields["motion_state"].offset, 0xC0)
        self.assertEqual(player_state_fields["status_effect4_timer"].offset, 0x50)
        self.assertEqual(player_state_fields["light_effect_timer"].offset, 0x52)
        self.assertEqual(player_state_fields["map_variant"].offset, 0x0C)
        self.assertEqual(
            player_state_fields["light_effect_timer"].meaning_confidence,
            "supported",
        )
        self.assertEqual(player_state_fields["unknown_ce"].meaning_confidence, "opaque")
        primitive_fields = {
            row.name: row for row in fields if row.structure == "KfPrimitiveBuffer"
        }
        self.assertEqual(primitive_fields["end"].offset, 0x04)
        self.assertEqual(primitive_fields["cursor"].offset, 0x08)
        tmd_fields = {
            row.name: row for row in fields if row.structure == "KfTmdObject"
        }
        self.assertEqual(tmd_fields["primitive_offset"].offset, 0x10)
        self.assertEqual(tmd_fields["primitive_count"].offset, 0x14)
        self.assertEqual(tmd_fields["scale"].offset, 0x18)

    def test_static_signature_hint_tracks_live_arguments_and_result(self) -> None:
        parameters, result, shape = _signature_hints(words(
            0x8C820000,  # lw v0,0(a0)
            0x00451021,  # addu v0,v0,a1
            0x03E00008,  # jr ra
            0x00000000,
        ))
        self.assertEqual(parameters, "unknown *object;unknown arg1")
        self.assertEqual(result, "unknown")
        self.assertIn("loads=1", shape)

    def test_semantic_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_math_lifecycle.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 17)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = (
                f"{identity.return_type} {identity.name}({parameters})"
            )
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_data_access_uses_low_instruction_opcode(self) -> None:
        reference = FakeReference()
        self.assertEqual(_data_access(FakeImage(0x8C820000), reference), "read")
        self.assertEqual(_data_access(FakeImage(0xAC820000), reference), "write")
        self.assertEqual(
            _data_access(FakeImage(0), FakeReference(kind="pointer")),
            "initializer",
        )

    def test_ghidra_types_are_explicitly_candidate_project_widths(self) -> None:
        self.assertEqual(_ghidra_type("undefined4"), "u32")
        self.assertEqual(_ghidra_type("undefined2 *"), "u16 *")
        self.assertEqual(_ghidra_type("short *"), "s16 *")

    def test_save_system_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_save_system.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 22)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = (
                f"{identity.return_type} {identity.name}({parameters})"
            )
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_screen_talk_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_screen_talk.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 2)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_actor_core_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_actor_core.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 16)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_actor_ai_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_actor_ai.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 15)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_actor_actions_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_actor_actions.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 10)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_map_objects_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_map_objects.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 8)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_map_runtime_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_map_runtime.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 10)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_audio_control_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_audio_control.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 5)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_audio_spatial_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_audio_spatial.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 4)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_camera_event_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_camera_events.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 7)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_event_query_matrix_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_event_queries_matrix.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 9)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_death_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_death.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 8)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_combat_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_combat.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 4)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_stats_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_stats.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 7)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_motion_attack_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_motion_attack.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 8)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_motion_attack_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_player_motion_attack"
        )
        self.assertEqual(len(campaign_rows), 146)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x80016F80]["target_name"], "map_floor_height_grid")
        self.assertEqual(by_site[0x80017414]["target_name"], "map_collision_grid")
        self.assertEqual(by_site[0x80017B24]["target_name"], "map_cell_attribute_grid")
        self.assertEqual(by_site[0x80019134]["target_name"], "player_begin_weapon_attack")
        self.assertEqual(by_site[0x8002EF78]["target_name"], "player_distance_to_point_in_cone")

    def test_player_interaction_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_interactions.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 8)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_interaction_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_player_interactions"
        )
        self.assertEqual(len(campaign_rows), 248)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x80012048]["confidence"], "pointer-reviewed")
        self.assertEqual(
            by_site[0x80017D40]["target_name"],
            "floor_entry_cells",
        )
        self.assertEqual(by_site[0x80017E3C]["target_name"], "player_state")
        self.assertEqual(
            by_site[0x80017E94]["target_name"],
            "player_state",
        )
        self.assertEqual(
            by_site[0x80018104]["target_name"],
            "player_item_use_jump_table",
        )
        self.assertEqual(by_site[0x8001A7A8]["target_name"], "collision_target")
        self.assertEqual(
            by_site[0x8001B2A4]["target_name"],
            "player_weapon_load_records_and_mirror_angles",
        )
        self.assertEqual(
            by_site[0x8002E9D4]["target_name"],
            "collision_query_world",
        )

    def test_player_update_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_update.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 5)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_player_update_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_player_update"
        )
        self.assertEqual(len(campaign_rows), 26)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        for site in (0x800187B8, 0x80018804, 0x80018838):
            self.assertEqual(by_site[site]["target_name"], "color_matrix_table")
        self.assertEqual(by_site[0x8001882C]["channel"], "instruction-word")
        self.assertEqual(
            by_site[0x80023600]["target_name"],
            "player_status_apply_effect4",
        )

    def test_collision_grid_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_collision_grid.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 3)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_collision_grid_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_collision_grid"
        )
        self.assertEqual(len(campaign_rows), 17)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x8001A2A0]["target_name"], "map_floor_height_grid")
        self.assertEqual(by_site[0x8001A2E8]["target_name"], "map_cell_orientation_grid")
        self.assertEqual(
            by_site[0x8001A4D0]["target_name"],
            "map_floor_height_for_cell_position",
        )
        self.assertEqual(
            by_site[0x800346FC]["target_name"],
            "collision_adjust_cell_occupancy",
        )
        self.assertEqual(by_site[0x800346FC]["channel"], "instruction-word")

    def test_collision_grid_data_owners_are_queryable(self) -> None:
        game = index("GAME.EXE")
        orientation = game.datum(0x80069018)
        occupancy = game.datum(0x800668E8)
        self.assertEqual(
            (orientation.name, orientation.datatype, orientation.size),
            ("map_cell_orientation_grid", "u8[100][100]", 0x2710),
        )
        self.assertEqual(game.data_owner(0x8006B727), orientation)
        self.assertEqual(
            (occupancy.name, occupancy.datatype, occupancy.size),
            ("map_collision_flag_grid", "u8[100][100]", 0x2710),
        )

    def test_map_resources_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_map_resources.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 11)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_map_resources_relocations_and_data_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_map_resources"
        )
        self.assertEqual(len(campaign_rows), 28)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x8001B108]["target_name"], "OpenTIM")
        self.assertEqual(
            by_site[0x8001B428]["target_name"],
            "map_variant_filename_template",
        )
        self.assertEqual(
            by_site[0x8001B460]["target_name"],
            "player_state",
        )
        game = index("GAME.EXE")
        path = game.datum(0x80055AF0)
        variant_buffer = game.datum(0x8009DDAC)
        self.assertEqual(
            (path.name, path.datatype, path.size),
            ("map_resource_path", "char[12]", 0x0C),
        )
        self.assertEqual(
            (variant_buffer.name, variant_buffer.datatype, variant_buffer.size),
            ("map_variant_asset_buffer", "u8 *", 4),
        )
    def test_player_motion_data_owners_are_queryable(self) -> None:
        game = index("GAME.EXE")
        state = game.datum(0x800A0780)
        self.assertEqual(
            (state.name, state.datatype, state.size),
            ("player_state", "KfPlayerState", 0xE0),
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0xC0),
            ("motion_state", "KfPlayerMotionState", 0x0A),
        )
        self.assertEqual(game.data_owner(0x800A0848), state)
        self.assertEqual(
            _structure_field("KfPlayerState", 0xCA), ("map_cell", "KfMapCell", 2)
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0x68)[1], "KfWeaponRecord *"
        )
        weapon_records = game.datum(0x8009FF10)
        self.assertEqual(
            (weapon_records.name, weapon_records.datatype, weapon_records.size),
            ("weapon_records", "KfWeaponRecord[16]", 0x2C0),
        )
        self.assertEqual(game.data_owner(0x800A00DC), weapon_records)
        collision_target = game.datum(0x800A01D0)
        self.assertEqual(
            (
                collision_target.name,
                collision_target.datatype,
                collision_target.size,
            ),
            ("collision_target", "KfCollisionTarget", 0x20),
        )
        self.assertEqual(game.data_owner(0x800A01E8), collision_target)
        floor_grid = game.datum(0x80095900)
        collision_grid = game.datum(0x80098018)
        attribute_grid = game.datum(0x8009A748)
        self.assertEqual(
            tuple(datum.size for datum in (floor_grid, collision_grid, attribute_grid)),
            (0x2710, 0x2710, 0x2710),
        )
        self.assertEqual(game.data_owner(0x800968F5), floor_grid)
        self.assertEqual(game.data_owner(0x8009C6B4), attribute_grid)

    def test_player_stats_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_player_stats"
        )
        self.assertEqual(len(campaign_rows), 269)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x80015F30]["target_name"], "player_state")
        self.assertEqual(by_site[0x800160E4]["target_name"], "player_level_growth_table")
        self.assertEqual(by_site[0x80016864]["target_name"], "player_equipment_slot_jump_table")
        self.assertEqual(by_site[0x80016B0C]["target_name"], "player_recalculate_combat_stats")
        self.assertEqual(by_site[0x80016AAC]["target_name"], "weapon_image_path_template")
        self.assertEqual(by_site[0x80012000]["confidence"], "pointer-reviewed")

    def test_player_combat_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = tuple(
            row
            for row in rows
            if row["provenance"] == "manual:game_semantic_player_combat"
        )
        self.assertEqual(len(campaign_rows), 52)
        self.assertEqual({row["image"] for row in campaign_rows}, {"GAME.EXE"})
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(
            by_site[0x800164BC]["target_name"],
            "player_calculate_damage_component",
        )
        self.assertEqual(by_site[0x800167AC]["target_name"], "player_apply_damage")
        self.assertEqual(
            by_site[0x80039694]["target_name"],
            "player_apply_radial_damage",
        )
        self.assertEqual(
            by_site[0x8001711C]["target_name"],
            "player_state",
        )
        self.assertEqual(
            by_site[0x8001640C]["target_name"],
            "player_state",
        )

    def test_player_death_matrix_relocations_are_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        matrix_rows = {
            parse_int(row["site_va"]): row
            for row in rows
            if row["image"] == "GAME.EXE"
            and parse_int(row["target_va"]) == 0x80058060
        }
        self.assertEqual(
            set(matrix_rows),
            {0x80015198, 0x8001866C, 0x800186A4},
        )
        for row in matrix_rows.values():
            self.assertEqual(row["target_name"], "player_death_saved_color_matrix")
            self.assertEqual(row["confidence"], "paired-reviewed")
            self.assertEqual(row["status"], "reviewed")
            self.assertEqual(row["provenance"], "manual:game_semantic_player_death")

    def test_camera_event_false_calls_remain_rejected(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        rejected = {
            parse_int(row["site_va"]): row
            for row in rows
            if row["image"] == "GAME.EXE" and row["status"] == "rejected"
        }
        for site in (
            0x80035070,
            0x800350A4,
            0x800350D0,
            0x800350F4,
            0x80035110,
            0x800359A4,
        ):
            self.assertEqual(rejected[site]["confidence"], "not-control-flow")
            self.assertEqual(
                rejected[site]["provenance"],
                "manual:game_semantic_camera_events",
            )

    def test_vmanager_key_utilities_are_vendored_in_both_overlays(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in rows
        }
        expected = {
            ("GAME.EXE", 0x80044FAC): "SsUtKeyOn",
            ("GAME.EXE", 0x80045378): "SsUtKeyOff",
            ("OPEN.EXE", 0x80024DCC): "SsUtKeyOn",
            ("OPEN.EXE", 0x80025198): "SsUtKeyOff",
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for key, name in expected.items():
            self.assertEqual(vendored[key]["name"], name)
            self.assertEqual(
                vendored[key]["confidence"],
                "sdk-lineage-supported",
            )
            self.assertNotIn(key, identities)

    def test_semantic_function_and_bss_identity_are_queryable(self) -> None:
        game = index("GAME.EXE")
        function = game.function(0x80014E08)
        self.assertEqual(function.name, "vector2s_scale_shift11")
        self.assertEqual(function.link_name, "func_80014e08")
        self.assertEqual(
            (function.owner_type, function.action),
            ("vector2s", "scale_shift11"),
        )
        datum = game.datum(0x800910C0)
        self.assertEqual(
            (datum.name, datum.kind, datum.size),
            ("pool_records", "bss", 0xF0),
        )
        candidate = game.datum(0x80058000)
        self.assertEqual(
            (candidate.name, candidate.kind, candidate.size),
            ("DAT_80058000", "bss", 4),
        )
        matrix = game.function(0x80014CCC)
        self.assertEqual(
            (matrix.name, matrix.owner_type, matrix.action),
            ("matrix_set_rotation_yxz", "matrix", "set_rotation_yxz"),
        )
        frame_counter = game.datum(0x80057B0C)
        self.assertEqual(
            (frame_counter.name, frame_counter.datatype, frame_counter.owner_type),
            ("frame_pacer_vsync_count", "u32", "frame_pacer"),
        )
        exit_code = game.datum(0x800958F8)
        self.assertEqual(
            (exit_code.name, exit_code.datatype, exit_code.owner_type),
            ("game_exit_code", "u32", "game"),
        )
        save_writer = game.function(0x8002B73C)
        self.assertEqual(
            (save_writer.name, save_writer.owner_type, save_writer.action),
            ("save_file_write_slot", "save_file", "write_slot"),
        )
        save_header = game.datum(0x800668D8)
        self.assertEqual(
            (save_header.name, save_header.datatype, save_header.owner_type),
            ("save_header_buffer", "KfSaveHeader *", "save_workspace"),
        )
        card_error = game.datum(0x80057EA8)
        self.assertEqual(
            (card_error.name, card_error.datatype, card_error.owner_type),
            ("memory_card_error_event", "s32", "memory_card"),
        )
        save_path = game.datum(0x80056034)
        self.assertEqual(save_path.name, "save_main_file_path")
        # The actor definitions, pool, and current/target pointers form one
        # aggregate: game_main_loop clears the whole 0x2b48-byte block and the
        # actor routines address its members from one base register.
        actor_state = game.datum(0x8006BD98)
        self.assertEqual(
            (actor_state.name, actor_state.datatype, actor_state.size, actor_state.owner_type),
            ("actor_state", "KfActorState", 0x2B48, "actor"),
        )
        self.assertEqual(game.data_owner(0x8006C4B8), actor_state)
        self.assertEqual(
            _structure_field("KfActorState", 0x0), ("definitions", "KfActorDefinition[12]", 0x720)
        )
        self.assertEqual(
            _structure_field("KfActorState", 0x720), ("actors", "KfActor[128]", 0x2400)
        )
        self.assertEqual(_structure_field("KfActorState", 0x2B3C), ("current", "KfActor *", 4))
        self.assertEqual(
            _structure_field("KfActorState", 0x2B44), ("player_target", "KfActor *", 4)
        )
        action_profiles = game.datum(0x80056080)
        self.assertEqual(
            (
                action_profiles.name,
                action_profiles.datatype,
                action_profiles.size,
            ),
            (
                "actor_action_profiles",
                "KfActorActionProfile[25]",
                0xFA,
            ),
        )
        data_identities = load_data_identities(RETAIL_CONFIG)
        self.assertEqual(
            data_identities[("GAME.EXE", 0x80056080)].scope,
            "unknown",
        )
        phase_sounds = game.datum(0x8005617C)
        self.assertEqual(
            (phase_sounds.name, phase_sounds.datatype, phase_sounds.size),
            ("boss_death_phase_sounds", "SoundRef[4]", 0xC),
        )
        loop_sound = game.datum(0x80057B80)
        self.assertEqual(
            (loop_sound.name, loop_sound.datatype, loop_sound.size),
            ("boss_death_loop_sound", "SoundRef", 3),
        )
        boss_flag = game.datum(0x8009F847)
        self.assertEqual(
            (boss_flag.name, boss_flag.datatype, boss_flag.owner_type),
            ("boss_defeat_complete", "u8", "actor"),
        )
        self.assertEqual(
            {
                data_identities[("GAME.EXE", va)].scope
                for va in (0x8005617C, 0x80057B80, 0x8009F847)
            },
            {"unknown"},
        )
        map_copy_regions = game.datum(0x800561B0)
        self.assertEqual(
            (
                map_copy_regions.name,
                map_copy_regions.datatype,
                map_copy_regions.size,
            ),
            ("map_copy_regions", "KfMapCopyRegion[4]", 0x18),
        )
        action_jump_table = game.datum(0x80012888)
        self.assertEqual(
            (
                action_jump_table.name,
                action_jump_table.datatype,
                action_jump_table.size,
            ),
            ("map_object_action_jump_table", "code pointer[99]", 0x18C),
        )
        gameplay_sounds = game.datum(0x80056188)
        self.assertEqual(
            (gameplay_sounds.name, gameplay_sounds.datatype, gameplay_sounds.size),
            ("gameplay_sound_refs", "SoundRef[13]", 0x27),
        )
        map_object_definitions = game.datum(0x8006E8E0)
        self.assertEqual(
            (
                map_object_definitions.name,
                map_object_definitions.datatype,
                map_object_definitions.size,
            ),
            (
                "map_object_definitions",
                "KfMapObjectDefinition[160]",
                0x500,
            ),
        )
        map_object_pool = game.datum(0x8006EDE0)
        self.assertEqual(
            (map_object_pool.name, map_object_pool.datatype, map_object_pool.size),
            ("map_object_pool", "KfMapObject[190]", 0x20A8),
        )
        map_object_loader = game.function(0x80031008)
        self.assertEqual(
            (
                map_object_loader.name,
                map_object_loader.owner_type,
                map_object_loader.action,
            ),
            ("map_object_pool_load", "map_object_pool", "load"),
        )
        map_object_updater = game.function(0x80031CC8)
        self.assertEqual(
            (
                map_object_updater.name,
                map_object_updater.owner_type,
                map_object_updater.action,
            ),
            ("map_object_pool_update", "map_object_pool", "update"),
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800561B0)].scope,
            "unknown",
        )
        sequence_path = game.datum(0x80012A48)
        self.assertEqual(sequence_path.name, "audio_sequence_path_template")
        sequence_table = game.datum(0x80059738)
        self.assertEqual(
            (sequence_table.name, sequence_table.datatype, sequence_table.size),
            (
                "audio_sequence_table",
                "u8[SS_SEQ_TABSIZ * 2]",
                0x158,
            ),
        )
        # The audio runtime state is one aggregate: audio_initialize derives the
        # voice-id slot address from the sequence-buffer field.
        audio_state = game.datum(0x80095868)
        self.assertEqual(
            (audio_state.name, audio_state.datatype, audio_state.size),
            ("audio_state", "KfAudioState", 0x90),
        )
        self.assertEqual(_structure_field("KfPlayerState", 0x96)[0], "audio_effects_enabled")
        self.assertEqual(_structure_field("KfPlayerState", 0x97)[0], "audio_music_enabled")
        self.assertEqual(
            {
                data_identities[("GAME.EXE", va)].scope
                for va in (
                    0x80059738,
                    0x80057B84,
                    0x80095868,
                )
            },
            {"unknown"},
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x8006E8E0)].scope,
            "unknown",
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x8006EDE0)].scope,
            "global",
        )
        effect_sequences = tuple(
            game.datum(va) for va in (0x80070E92, 0x80070E94, 0x80070E96)
        )
        self.assertEqual(
            tuple(datum.name for datum in effect_sequences),
            (
                "map_object_effect_sequence_160",
                "map_object_effect_sequence_170",
                "map_object_effect_sequence_180",
            ),
        )
        self.assertEqual(
            {
                data_identities[("GAME.EXE", va)].scope
                for va in (0x80070E92, 0x80070E94, 0x80070E96)
            },
            {"unknown"},
        )
        object_action = game.function(0x80031784)
        self.assertEqual(
            (object_action.name, object_action.signature_confidence),
            ("map_object_start_action_if_idle", "proven"),
        )
        actor_update = game.function(0x80030818)
        self.assertEqual(
            (actor_update.name, actor_update.owner_type, actor_update.action),
            ("actor_pool_update", "actor_pool", "update"),
        )
        talk_path = game.datum(0x8005606C)
        self.assertEqual(talk_path.name, "talk_image_path_template")
        self.assertEqual(
            _structure_field("KfPlayerState", 0xA4),
            ("camera_position", "KfVec4i", 0x10),
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0xB8),
            ("camera_rotation", "KfVec4s", 8),
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0x08),
            ("progress_state", "KfPlayerProgressState", 4),
        )
        map_events = game.datum(0x8009DB88)
        interior_event = game.data_owner(0x8009DC8A)
        self.assertEqual(
            (map_events.name, map_events.datatype, map_events.size),
            ("map_event_pool", "KfMapEvent[8]", 0x220),
        )
        self.assertEqual(interior_event, map_events)
        current_event = game.datum(0x8009DDA8)
        self.assertEqual(
            (current_event.name, current_event.datatype, current_event.size),
            ("current_map_event", "KfMapEvent *", 4),
        )
        player_state = game.datum(0x800A0780)
        current_mp_owner = game.data_owner(0x800A0796)
        self.assertEqual(
            (player_state.name, player_state.datatype, player_state.size),
            ("player_state", "KfPlayerState", 0xE0),
        )
        self.assertEqual(current_mp_owner, player_state)
        self.assertEqual(
            _structure_field("KfPlayerState", 0x10), ("vitals", "KfPlayerVitals", 8)
        )
        saved_fog = game.datum(0x80057E78)
        saved_color_matrix = game.datum(0x80058060)
        self.assertEqual(
            (saved_fog.name, saved_fog.datatype, saved_fog.size),
            ("player_death_saved_fog_near", "s32", 4),
        )
        self.assertEqual(
            (
                saved_color_matrix.name,
                saved_color_matrix.datatype,
                saved_color_matrix.size,
            ),
            ("player_death_saved_color_matrix", "KfMatrix", 0x20),
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0xD8), ("death_camera_pitch_step", "u16", 2)
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0xDA), ("death_visual_blend", "s16", 2)
        )
        status_fields = tuple(
            _structure_field("KfPlayerState", offset)
            for offset in (0x2A, 0x3C, 0x3E, 0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E)
        )
        self.assertEqual(
            tuple(field[0] for field in status_fields),
            (
                "status_effect_flags",
                "damage_defense_component0",
                "damage_defense_component1",
                "damage_defense_component2",
                "status_effect2_resistance",
                "damage_defense_component3",
                "damage_defense_component4",
                "status_effect0_timer",
                "status_effect1_timer",
                "status_effect2_timer",
                "status_effect3_timer",
            ),
        )
        self.assertEqual(
            tuple(field[1] for field in status_fields),
            ("u16",) * 7 + ("s16",) * 4,
        )
        self.assertEqual(
            {
                data_identities[("GAME.EXE", va)].scope
                for va in (
                    0x8009DB88,
                    0x8009DDA8,
                    0x80058060,
                    0x80057E78,
                )
            },
            {"unknown"},
        )


if __name__ == "__main__":
    unittest.main()
