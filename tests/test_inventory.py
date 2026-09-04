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
        self.assertEqual(counts["functions"], 499)
        self.assertEqual(counts["signatures_started"], 499)
        self.assertEqual(counts["typed_returns"], 499)
        self.assertEqual(counts["parameterized"], 316)
        self.assertEqual(counts["data"], 3165)
        self.assertGreaterEqual(counts["functions_named"], 240)
        self.assertGreaterEqual(counts["data_named"], 100)
        self.assertEqual(counts["structures"], 71)
        self.assertEqual(counts["structure_fields"], 635)
        self.assertEqual(counts["structure_fields_named"], 521)

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
        self.assertEqual(structures["KfScreenVertex"].size, 0x08)
        self.assertEqual(structures["KfCellWindow"].size, 0xCC)
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
        self.assertEqual(
            (actor_fields["position"].size, actor_fields["position"].datatype),
            (0x10, "VECTOR"),
        )
        self.assertEqual(actor_fields["unknown_07"].meaning_confidence, "opaque")
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
        self.assertEqual(len(rows), 6)
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
        self.assertEqual(len(rows), 8)
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
        self.assertEqual(len(campaign_rows), 153)
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

    def test_menu_frame_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_menu_frame.tsv"
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

    def test_menu_frame_cursor_owner_is_queryable(self) -> None:
        cursor = index("GAME.EXE").datum(0x80057E88)
        self.assertEqual(
            (cursor.name, cursor.datatype, cursor.size),
            ("current_poly_ft4", "POLY_FT4 *", 4),
        )

    def test_render_map_cells_tu_is_gapless_and_supported(self) -> None:
        evidence_path = CONFIG / "evidence/game_tu_render_map_cells.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 2)

        spans = [
            (parse_int(row["va"]), parse_int(row["extent"]))
            for row in rows
        ]
        self.assertEqual(spans, [(0x8001E5EC, 0x250), (0x8001E83C, 0x168)])
        self.assertEqual(spans[0][0] + spans[0][1], spans[1][0])

        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertEqual(identity.signature_confidence, "supported")
            self.assertIn(evidence_path.name, identity.evidence)

        units = (CONFIG / "units.toml").read_text()
        self.assertEqual(units.count('source = "src/game/render_map_cells.c"'), 1)
        self.assertNotIn('source = "src/game/render_map_cell.c"', units)

    def test_menu_runtime_tu_is_gapless_and_uses_one_unit(self) -> None:
        presentation_path = CONFIG / "evidence/game_tu_menu_presentation.tsv"
        runtime_path = CONFIG / "evidence/game_tu_menu_runtime.tsv"
        _, presentation_rows = read_tsv(presentation_path)
        _, runtime_rows = read_tsv(runtime_path)
        rows = presentation_rows + runtime_rows
        self.assertEqual(len(rows), 16)
        spans = [
            (parse_int(row["va"]), parse_int(row["extent"]))
            for row in rows
        ]
        self.assertEqual(spans[0][0], 0x800291EC)
        self.assertEqual(spans[-1][0] + spans[-1][1], 0x8002B078)
        for (va, extent), (next_va, _next_extent) in zip(spans, spans[1:]):
            self.assertEqual(va + extent, next_va)

        units = (CONFIG / "units.toml").read_text()
        self.assertEqual(units.count('source = "src/game/menu_runtime.c"'), 1)
        for old_source in (
            "src/game/menu_frame.c",
            "src/game/menu_list_util.c",
            "src/game/game_state.c",
            "src/game/menu_texture_load.c",
            "src/game/menu_presentation.c",
        ):
            self.assertNotIn(old_source, units)

        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("GAME.EXE", 0x8002ADF8)
        ]
        self.assertEqual(
            identity.parameters,
            "s32 value;s32 count;s32 pad_zero;s16 * out",
        )

    def test_menu_presentation_tu_and_interfaces_are_curated(self) -> None:
        evidence_path = CONFIG / "evidence/game_tu_menu_presentation.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 7)

        spans = [
            (parse_int(row["va"]), parse_int(row["extent"]))
            for row in rows
        ]
        self.assertEqual(spans[0][0], 0x800291EC)
        self.assertEqual(spans[-1][0] + spans[-1][1], 0x8002ABB4)
        for (va, extent), (next_va, _next_extent) in zip(spans, spans[1:]):
            self.assertEqual(va + extent, next_va)

        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

        units = (CONFIG / "units.toml").read_text()
        self.assertEqual(units.count('source = "src/game/menu_runtime.c"'), 1)
        for old_source in (
            "src/game/menu_two_option_dispatch.c",
            "src/game/menu_draw_item_name_frame.c",
            "src/game/menu_sprite_blit.c",
            "src/game/menu_draw_number.c",
            "src/game/menu_window_backdrop.c",
            "src/game/menu_presentation.c",
        ):
            self.assertNotIn(old_source, units)

        data_identities = load_data_identities(RETAIL_CONFIG)
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800583E8)].datatype,
            "MenuSpriteDef",
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800583F4)].datatype,
            "MenuSpriteDef",
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800580E8)].datatype,
            "POLY_FT4[2][4]",
        )

    def test_menu_list_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_menu_list.tsv"
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

    def test_projection_scratch_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_projection_scratch.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 6)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

        self.assertEqual(
            _structure_field("KfScreenVertex", 0x00), ("sxy", "DVECTOR", 4)
        )
        self.assertEqual(
            _structure_field("KfScreenVertex", 0x04), ("sz", "s16", 2)
        )
        self.assertEqual(
            _structure_field("KfScreenVertex", 0x06), ("p2", "s16", 2)
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

    def test_open_cd_file_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_cd_file.tsv"
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

        data = load_data_identities(RETAIL_CONFIG)
        expected = {
            0x800372DC: ("cd_path_prefix", "load", "char[5]", 0x05),
            0x800372E4: ("cd_version_suffix", "load", "char[3]", 0x03),
            0x800375D8: ("cd_read_location", "load", "CdlLOC", 0x04),
            0x800377A0: ("cd_search_file", "bss", "CdlFILE", 0x18),
            0x800377B8: ("cd_path_buffer", "bss", "char[80]", 0x50),
        }
        for va, shape in expected.items():
            identity = data[("OPEN.EXE", va)]
            self.assertEqual(
                (identity.name, identity.storage, identity.datatype, identity.size),
                shape,
            )

    def test_open_cd_file_relocations_are_named_and_reviewed(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in rows
            if "manual:open_cd_file" in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 43)
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})

        calls = [row for row in campaign_rows if row["kind"] == "mips26"]
        body_data = [
            row
            for row in campaign_rows
            if 0x80016014 <= parse_int(row["site_va"]) < 0x80016298
        ]
        self.assertEqual(len(calls), 16)
        self.assertEqual(len(body_data), 26)
        self.assertEqual({row["status"] for row in calls + body_data}, {"reviewed"})

    def test_open_resources_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_resources.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 14)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_resources" in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 73)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        calls = [row for row in campaign_rows if row["opcode"] == "jal"]
        self.assertNotIn("", {row["target_name"] for row in calls})

        loader_rows = [
            row
            for row in relocation_rows
            if row["image"] == "OPEN.EXE"
            and 0x80016348 <= parse_int(row["site_va"]) < 0x800168DC
        ]
        self.assertEqual(len(loader_rows), 90)
        self.assertEqual({row["status"] for row in loader_rows}, {"reviewed"})
        loader_calls = [row for row in loader_rows if row["opcode"] == "jal"]
        self.assertNotIn("", {row["target_name"] for row in loader_calls})

        data = load_data_identities(RETAIL_CONFIG)
        expected_grids = {
            0x800446C8: "map_collision_flag_grid",
            0x80046DF8: "map_cell_orientation_grid",
            0x8006E260: "map_floor_height_grid",
            0x80070978: "map_collision_grid",
            0x800730A0: "map_cell_attribute_grid",
        }
        for va, name in expected_grids.items():
            identity = data[("OPEN.EXE", va)]
            self.assertEqual(
                (identity.name, identity.storage, identity.datatype, identity.size),
                (name, "bss", "u8[100][100]", 0x2710),
            )

    def test_open_render_init_campaign_is_exactly_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_render_init.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 3)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            self.assertEqual(
                row["final_signature"],
                f"{identity.return_type} {identity.name}({parameters})",
            )
            self.assertIn(evidence_path.name, identity.evidence)

        data = load_data_identities(RETAIL_CONFIG)
        table = data[("OPEN.EXE", 0x80035944)]
        self.assertEqual(
            (table.name, table.storage, table.datatype, table.size),
            ("color_matrix_table", "load", "MATRIX[5]", 0xA0),
        )
        draw_environments = data[("OPEN.EXE", 0x80069A70)]
        self.assertEqual(
            (
                draw_environments.name,
                draw_environments.storage,
                draw_environments.datatype,
                draw_environments.size,
            ),
            ("display_draw_environments", "bss", "DRAWENV[2]", 0xB8),
        )
        display_environments = data[("OPEN.EXE", 0x80069B28)]
        self.assertEqual(
            (
                display_environments.name,
                display_environments.storage,
                display_environments.datatype,
                display_environments.size,
            ),
            ("display_disp_environments", "bss", "DISPENV[2]", 0x28),
        )
        allocation_count = data[("OPEN.EXE", 0x80075928)]
        self.assertEqual(
            (allocation_count.name, allocation_count.storage, allocation_count.datatype),
            ("primitive_allocation_count", "bss", "u32"),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if row["provenance"] == "manual:open_semantic_render_init"
        ]
        self.assertEqual(len(campaign_rows), 62)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in campaign_rows}
        self.assertEqual(by_site[0x800168E8]["target_name"], "color_matrix_table")
        self.assertEqual(by_site[0x800168F0]["target_name"], "SetColorMatrix")
        self.assertEqual(
            by_site[0x80016C54]["target_name"],
            "lighting_set_active_color_matrix",
        )
        self.assertEqual(by_site[0x80015718]["target_name"], "display_initialize")
        self.assertEqual(
            by_site[0x80016D20]["target_name"], "primitive_allocation_count"
        )
        self.assertEqual(
            by_site[0x80018A80]["target_name"], "primitive_buffer_allocate"
        )

    def test_open_projection_campaign_is_semantically_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_projection.tsv"
        _, rows = read_tsv(evidence_path)
        self.assertEqual(len(rows), 1)
        row = rows[0]

        functions = load_function_identities(RETAIL_CONFIG, required=True)
        identity = functions[("OPEN.EXE", 0x8001738C)]
        self.assertEqual(
            (identity.name, identity.return_type, identity.parameters),
            ("tmd_project_vertices", "void", "s32 count"),
        )
        self.assertEqual(row["final_signature"], "void tmd_project_vertices(s32 count)")
        self.assertIn(evidence_path.name, identity.evidence)

        data = load_data_identities(RETAIL_CONFIG)
        scratch = data[("OPEN.EXE", 0x80069B80)]
        self.assertEqual(
            (scratch.name, scratch.storage, scratch.datatype, scratch.size),
            ("tmd_projected_vertices", "bss", "KfScreenVertex[1000]", 0x1F40),
        )
        self.assertNotIn(("OPEN.EXE", 0x80069B84), data)
        self.assertNotIn(("OPEN.EXE", 0x80069B86), data)
        shift = data[("OPEN.EXE", 0x8006E1CC)]
        self.assertEqual(
            (shift.name, shift.storage, shift.datatype, shift.size),
            ("tmd_projection_shift", "bss", "s16", 2),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            item
            for item in relocation_rows
            if item["provenance"] == "manual:open_semantic_projection"
        ]
        self.assertEqual(len(campaign_rows), 11)
        self.assertEqual({item["status"] for item in campaign_rows}, {"reviewed"})
        by_site = {parse_int(item["site_va"]): item for item in campaign_rows}
        self.assertEqual(by_site[0x800173C8]["target_name"], "tmd_projected_vertices")
        self.assertEqual(by_site[0x80017408]["target_name"], "tmd_projection_shift")
        self.assertEqual(by_site[0x8001741C]["target_name"], "ReadSZ2")
        self.assertEqual(by_site[0x800190C0]["target_name"], "tmd_project_vertices")

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
            "",
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

    def test_vmanager_auto_controls_are_order_resolved_in_both_overlays(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in rows
        }
        expected = {
            ("GAME.EXE", 0x80045F7C): ("SsUtAutoVol", 0x39BC),
            ("GAME.EXE", 0x80045F98): ("SsUtAutoPan", 0x39D8),
            ("OPEN.EXE", 0x80025D9C): ("SsUtAutoVol", 0x39BC),
            ("OPEN.EXE", 0x80025DB8): ("SsUtAutoPan", 0x39D8),
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for key, (name, member_offset) in expected.items():
            row = vendored[key]
            self.assertEqual(row["name"], name)
            self.assertEqual(row["library"], "LIBSND.LIB")
            self.assertEqual(row["module"], "VMANAGER")
            self.assertEqual(parse_int(row["member_offset"]), member_offset)
            self.assertEqual(row["confidence"], "fid-release25-ambiguous")
            self.assertNotIn(key, identities)

    def test_libgpu_graph_state_accessors_are_vendored_in_both_overlays(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in rows
        }
        expected = {
            ("GAME.EXE", 0x8005081C): ("GetGraphType", 0x308),
            ("GAME.EXE", 0x8005082C): ("GetGraphDebug", 0x318),
            ("GAME.EXE", 0x8005083C): ("DrawSyncCallback", 0x328),
            ("OPEN.EXE", 0x80030500): ("GetGraphType", 0x308),
            ("OPEN.EXE", 0x80030510): ("GetGraphDebug", 0x318),
            ("OPEN.EXE", 0x80030520): ("DrawSyncCallback", 0x328),
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for key, (name, member_offset) in expected.items():
            row = vendored[key]
            self.assertEqual(row["name"], name)
            self.assertEqual(row["library"], "LIBGPU.LIB")
            self.assertEqual(row["module"], "SYS")
            self.assertEqual(parse_int(row["member_offset"]), member_offset)
            self.assertEqual(row["confidence"], "sdk-lineage-supported")
            self.assertNotIn(key, identities)

    def test_libsnd_sequence_control_is_vendored_in_both_overlays(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in rows
        }
        expected = {
            ("GAME.EXE", 0x8004A128): ("SsPlayBack", "SSPLAY"),
            ("GAME.EXE", 0x8004A21C): ("Snd_SetPlayMode", "SSPLAY"),
            ("GAME.EXE", 0x8004A344): ("SsSeqStop", "STOP"),
            ("GAME.EXE", 0x8004A374): ("SsSepStop", "STOP"),
            ("GAME.EXE", 0x8004A3C8): ("Snd_stop", "STOP"),
            ("OPEN.EXE", 0x80029EFC): ("SsPlayBack", "SSPLAY"),
            ("OPEN.EXE", 0x80029FF0): ("Snd_SetPlayMode", "SSPLAY"),
            ("OPEN.EXE", 0x8002A118): ("SsSeqStop", "STOP"),
            ("OPEN.EXE", 0x8002A148): ("SsSepStop", "STOP"),
            ("OPEN.EXE", 0x8002A19C): ("Snd_stop", "STOP"),
            ("GAME.EXE", 0x8004A55C): ("SsSeqCalledTbyT", "SSCALL"),
            ("GAME.EXE", 0x8004A854): ("Snd_decrescendo", "DECRE"),
            ("GAME.EXE", 0x8004AD9C): ("Snd_replay", "REPLAY"),
            ("GAME.EXE", 0x8004AE30): ("Snd_crescendo", "CRES"),
            ("GAME.EXE", 0x8004B360): ("Snd_pause", "PAUSE"),
            ("GAME.EXE", 0x8004B3F4): ("Snd_nextpause", "PAUSE"),
            ("GAME.EXE", 0x8004B458): ("Snd_tempo", "TEMPO"),
            ("OPEN.EXE", 0x8002A330): ("SsSeqCalledTbyT", "SSCALL"),
            ("OPEN.EXE", 0x8002A628): ("Snd_decrescendo", "DECRE"),
            ("OPEN.EXE", 0x8002AB70): ("Snd_replay", "REPLAY"),
            ("OPEN.EXE", 0x8002AC04): ("Snd_crescendo", "CRES"),
            ("OPEN.EXE", 0x8002B134): ("Snd_pause", "PAUSE"),
            ("OPEN.EXE", 0x8002B1C8): ("Snd_nextpause", "PAUSE"),
            ("OPEN.EXE", 0x8002B22C): ("Snd_tempo", "TEMPO"),
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for key, (name, module) in expected.items():
            self.assertEqual(vendored[key]["name"], name)
            self.assertEqual(vendored[key]["module"], module)
            self.assertEqual(
                vendored[key]["confidence"],
                "sdk-lineage-supported",
            )
            self.assertNotIn(key, identities)

        self.assertEqual(vendored[("GAME.EXE", 0x8004B3F4)]["member_offset"], "0x8c")
        self.assertEqual(vendored[("OPEN.EXE", 0x8002B1C8)]["member_offset"], "0x8c")

        data_identities = load_data_identities(RETAIL_CONFIG)
        expected_data = {
            ("GAME.EXE", 0x8009FF00): "VBLANK_MINUS",
            ("GAME.EXE", 0x800A0770): "_snd_seq_s_max",
            ("OPEN.EXE", 0x800757D0): "VBLANK_MINUS",
            ("OPEN.EXE", 0x80075930): "_snd_seq_s_max",
        }
        for key, name in expected_data.items():
            self.assertEqual(data_identities[key].name, name)
            self.assertEqual(data_identities[key].confidence, "supported")

    def test_libsnd_sequence_open_init_and_close_are_vendored(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in rows
        }
        expected = {
            ("GAME.EXE", 0x800468D8): ("SsSeqOpen", "SSOPEN"),
            ("GAME.EXE", 0x80046988): ("SsSepOpen", "SSOPEN"),
            ("GAME.EXE", 0x80046A94): ("InitSoundSep", "SEPINIT"),
            ("GAME.EXE", 0x800471A4): ("InitSoundSeq", "SEQINIT"),
            ("GAME.EXE", 0x8004B6E0): ("SsSeqClose", "SSCLOSE"),
            ("GAME.EXE", 0x8004B848): ("SsSepClose", "SSCLOSE"),
            ("OPEN.EXE", 0x800266AC): ("SsSeqOpen", "SSOPEN"),
            ("OPEN.EXE", 0x8002675C): ("SsSepOpen", "SSOPEN"),
            ("OPEN.EXE", 0x80026868): ("InitSoundSep", "SEPINIT"),
            ("OPEN.EXE", 0x80026F78): ("InitSoundSeq", "SEQINIT"),
            ("OPEN.EXE", 0x8002B4B4): ("SsSeqClose", "SSCLOSE"),
            ("OPEN.EXE", 0x8002B61C): ("SsSepClose", "SSCLOSE"),
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for key, (name, module) in expected.items():
            self.assertEqual(vendored[key]["name"], name)
            self.assertEqual(vendored[key]["module"], module)
            self.assertEqual(
                vendored[key]["confidence"],
                "sdk-lineage-supported",
            )
            self.assertNotIn(key, identities)

        _, function_rows = read_tsv(RETAIL_CONFIG / "functions.tsv")
        open_functions = {
            parse_int(row["va"]): parse_int(row["size"])
            for row in function_rows
            if row["image"] == "OPEN.EXE"
        }
        self.assertEqual(open_functions[0x8002675C], 0x10C)
        self.assertNotIn(0x80026764, open_functions)

        _, data_rows = read_tsv(RETAIL_CONFIG / "data.tsv")
        self.assertFalse(
            any(
                row["image"] == "OPEN.EXE"
                and parse_int(row["va"]) == 0x8002675C
                for row in data_rows
            )
        )

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
            ("gameplay_sound_ref_0", "SoundRef", 0x3),
        )
        map_object_state = game.datum(0x8006E8E0)
        self.assertEqual(
            (map_object_state.name, map_object_state.datatype, map_object_state.size),
            ("map_object_state", "KfMapObjectState", 0x25A8),
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
            ("camera_position", "VECTOR", 0x10),
        )
        self.assertEqual(
            _structure_field("KfPlayerState", 0xB8),
            ("camera_rotation", "SVECTOR", 8),
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
            ("player_death_saved_color_matrix", "MATRIX", 0x20),
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
