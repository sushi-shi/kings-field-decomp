from __future__ import annotations

import unittest

from scripts.kf.inventory import (
    _data_access,
    _ghidra_type,
    _signature_hints,
    load_data_identities,
    load_function_identities,
    validate,
)
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.index import index


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
        self.assertEqual(counts["functions"], 744)
        self.assertEqual(counts["signatures_started"], 744)
        self.assertEqual(counts["typed_returns"], 744)
        self.assertEqual(counts["parameterized"], 498)
        self.assertEqual(counts["data"], 3772)
        self.assertGreaterEqual(counts["functions_named"], 120)
        self.assertGreaterEqual(counts["data_named"], 54)

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
        actor_pool = game.datum(0x8006C4B8)
        self.assertEqual(
            (actor_pool.name, actor_pool.datatype, actor_pool.size),
            ("actor_pool", "KfActor[128]", 0x2400),
        )
        actor_definition = game.datum(0x8006BD98)
        self.assertEqual(
            (actor_definition.name, actor_definition.datatype),
            ("actor_definitions", "KfActorDefinition[12]"),
        )
        current_actor = game.datum(0x8006E8D4)
        self.assertEqual(
            (current_actor.name, current_actor.datatype),
            ("current_actor", "KfActor *"),
        )
        target_actor = game.datum(0x8006E8DC)
        self.assertEqual(
            (target_actor.name, target_actor.datatype, target_actor.owner_type),
            ("player_target_actor", "KfActor *", "actor"),
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
        map_object_metadata = game.datum(0x8006E8E0)
        self.assertEqual(
            (
                map_object_metadata.name,
                map_object_metadata.datatype,
                map_object_metadata.size,
            ),
            ("map_object_metadata", "u8[0x500]", 0x500),
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
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800561B0)].scope,
            "unknown",
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x8006E8E0)].scope,
            "unknown",
        )
        self.assertEqual(
            data_identities[("GAME.EXE", 0x8006EDE0)].scope,
            "global",
        )
        actor_update = game.function(0x80030818)
        self.assertEqual(
            (actor_update.name, actor_update.owner_type, actor_update.action),
            ("actor_pool_update", "actor_pool", "update"),
        )
        talk_path = game.datum(0x8005606C)
        self.assertEqual(talk_path.name, "talk_image_path_template")


if __name__ == "__main__":
    unittest.main()
