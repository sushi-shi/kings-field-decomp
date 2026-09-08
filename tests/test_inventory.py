from __future__ import annotations

import unittest
from unittest.mock import patch

from scripts.kf.inventory import (
    _data_access,
    _ghidra_type,
    _header_structure_layouts,
    _signature_hints,
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
    validate,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.index import index
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.mips import decode_control


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
    def test_storage_macro_requires_a_declared_enum_domain(self) -> None:
        declaration = """
            KF_ENUM_BEGIN(Floor, s32)
                FLOOR_FIRST = 1
            KF_ENUM_END(Floor)
        """
        carrier = "typedef struct Carrier { KF_ENUM_STORAGE(Floor, u8) floor; } Carrier;"
        with patch("pathlib.Path.read_text", lambda path:
                   declaration + carrier if path.name == "game_types.h" else ""):
            field = _header_structure_layouts()["Carrier"].fields[0]
            self.assertEqual(field.datatype, "KF_ENUM_STORAGE(Floor, u8)")
        with patch("pathlib.Path.read_text", lambda path:
                   carrier if path.name == "game_types.h" else ""):
            with self.assertRaisesRegex(ValueError, "undeclared enum domain 'Floor'"):
                _header_structure_layouts()

    def test_stored_enum_fields_keep_domain_names_widths_and_alignment(self) -> None:
        source = """
            KF_ENUM_BEGIN(ByteState, u8)
                BYTE_IDLE = 0, BYTE_COUNT = 3
            KF_ENUM_END(ByteState)
            KF_ENUM_BEGIN(HalfState, s16)
                HALF_IDLE = 0
            KF_ENUM_END(HalfState)
            typedef struct EnumCarrier {
                ByteState states[BYTE_COUNT];
                HalfState state;
                u8 tail;
            } EnumCarrier;
        """
        with patch("pathlib.Path.read_text",
                   lambda path: source if path.name == "game_types.h" else ""):
            carrier = _header_structure_layouts()["EnumCarrier"]
        self.assertEqual((carrier.size, carrier.alignment), (8, 2))
        self.assertEqual([(f.name, f.datatype, f.offset, f.size) for f in carrier.fields], [
            ("states", "ByteState[3]", 0, 3), ("state", "HalfState", 4, 2),
            ("tail", "u8", 6, 1),
        ])

    def test_unknown_enum_storage_is_rejected_before_layout_can_be_trusted(self) -> None:
        source = """
            KF_ENUM_BEGIN(BadState, size_t)
                BAD_IDLE = 0
            KF_ENUM_END(BadState)
            typedef struct BadCarrier { BadState state; } BadCarrier;
        """
        with patch("pathlib.Path.read_text",
                   lambda path: source if path.name == "game_types.h" else ""):
            with self.assertRaisesRegex(ValueError, "unsupported enum storage"):
                _header_structure_layouts()

    def test_named_union_uses_maximum_extent_and_alignment(self) -> None:
        source = """
            typedef union LayoutUnion { u8 bytes[5]; u32 word; } LayoutUnion;
            typedef struct LayoutCarrier {
                u8 prefix;
                union LayoutUnion payload;
                u8 suffix;
            } LayoutCarrier;
        """
        def header(path):
            return source if path.name == "game_types.h" else ""

        with patch("pathlib.Path.read_text", header):
            layouts = _header_structure_layouts()
        union = layouts["LayoutUnion"]
        self.assertEqual((union.size, union.alignment), (8, 4))
        self.assertEqual([(f.offset, f.size) for f in union.fields], [(0, 5), (0, 4)])
        carrier = layouts["LayoutCarrier"]
        self.assertEqual((carrier.size, carrier.alignment), (16, 4))
        self.assertEqual([(f.offset, f.size) for f in carrier.fields],
                         [(0, 1), (4, 8), (12, 1)])

    def test_named_union_rejects_unknown_member_type(self) -> None:
        def header(path):
            return ("typedef union BadLayout { Missing value; } BadLayout;"
                    if path.name == "game_types.h" else "")

        with patch("pathlib.Path.read_text", header):
            with self.assertRaisesRegex(ValueError, "unknown field type 'Missing'"):
                _header_structure_layouts()

    def test_effect_union_views_preserve_record_layout(self) -> None:
        layouts = _header_structure_layouts()
        for name, size, members in (
            ("KfEffectDirection", 8, 2), ("KfRotation", 8, 2),
            ("KfEffectControl", 2, 5),
        ):
            layout = layouts[name]
            self.assertEqual((layout.size, layout.alignment), (size, 2))
            self.assertEqual([f.offset for f in layout.fields], [0] * members)
        record = layouts["KfEffectRecord"]
        self.assertEqual((record.size, record.alignment), (60, 4))
        offsets = {f.name: f.offset for f in record.fields}
        self.assertEqual((offsets["rotation"], offsets["direction"],
                          offsets["control"], offsets["propagation"]),
                         (0x1c, 0x2c, 0x38, 0x3a))

    def test_named_array_extents_preserve_physical_layout_and_reject_unknowns(self) -> None:
        declarations = """
            enum { ROWS = 3, COLUMNS = 0x5, EMPTY = 0, EXPRESSION = ROWS + 1 };
            typedef struct ExampleGrid {
                u8 prefix;
                u16 cells[ROWS][COLUMNS];
                u32 suffix;
            } ExampleGrid;
        """

        def read_header(path):
            return declarations if path.name == "game_types.h" else ""

        with patch("scripts.kf.inventory.Path.read_text", read_header):
            layout = _header_structure_layouts()["ExampleGrid"]
        self.assertEqual((layout.size, layout.alignment), (36, 4))
        self.assertEqual([(f.offset, f.size, f.datatype) for f in layout.fields],
                         [(0, 1, "u8"), (2, 30, "u16[3][5]"), (32, 4, "u32")])
        for bound, error in (("MISSING", "unresolved"), ("EXPRESSION", "unresolved"),
                             ("EMPTY", "nonpositive")):
            with self.subTest(bound=bound):
                broken = declarations.replace("cells[ROWS]", f"cells[{bound}]")
                with patch("scripts.kf.inventory.Path.read_text",
                           lambda path: broken if path.name == "game_types.h" else ""):
                    with self.assertRaisesRegex(ValueError, error + " array bound"):
                        _header_structure_layouts()

    def test_implicit_enum_array_count_starts_at_zero_and_follows_known_resets(self) -> None:
        declarations = """
            typedef enum TileIndex { FIRST, SECOND, THIRD, FOURTH, TILE_COUNT } TileIndex;
            enum { RESET = 0x7, NEXT, ROW_COUNT };
            typedef struct TileGrid { u16 tiles[ROW_COUNT][TILE_COUNT]; } TileGrid;
        """
        with patch("scripts.kf.inventory.Path.read_text",
                   lambda path: declarations if path.name == "game_types.h" else ""):
            layout = _header_structure_layouts()["TileGrid"]
        self.assertEqual((layout.size, layout.alignment), (72, 2))
        self.assertEqual(layout.fields[0].datatype, "u16[9][4]")

    def test_implicit_enum_bound_does_not_guess_after_unknown_expression_or_overflow(self) -> None:
        for value in ("EXTERNAL + 1", "0x7fffffff"):
            with self.subTest(value=value):
                declarations = f"""
                    enum {{ FIRST = {value}, UNKNOWN_COUNT, RESET = 2, KNOWN_COUNT }};
                    typedef struct UnknownGrid {{ u8 cells[UNKNOWN_COUNT]; }} UnknownGrid;
                """
                with patch("scripts.kf.inventory.Path.read_text",
                           lambda path: declarations if path.name == "game_types.h" else ""):
                    with self.assertRaisesRegex(ValueError, "unresolved array bound 'UNKNOWN_COUNT'"):
                        _header_structure_layouts()
                declarations = declarations.replace("cells[UNKNOWN_COUNT]", "cells[KNOWN_COUNT]")
                with patch("scripts.kf.inventory.Path.read_text",
                           lambda path: declarations if path.name == "game_types.h" else ""):
                    self.assertEqual(_header_structure_layouts()["UnknownGrid"].size, 3)

    def test_union_storage_overlaps_and_rounds_up_for_enclosing_struct(self) -> None:
        declarations = """
            typedef union ExamplePayload {
                u8 bytes[5];
                u32 word;
                u16 pair[2];
            } ExamplePayload;
            typedef struct ExampleEnvelope {
                u8 prefix;
                union ExamplePayload payload;
                u8 suffix;
            } ExampleEnvelope;
            typedef union UninventoriedView {
                u8 bytes[5];
                struct { u16 first; u16 second; } nested;
            } UninventoriedView;
        """

        def read_header(path):
            return declarations if path.name == "game_types.h" else ""

        with patch("scripts.kf.inventory.Path.read_text", read_header):
            layouts = _header_structure_layouts()
        self.assertEqual(set(layouts), {"ExamplePayload", "ExampleEnvelope"})
        payload = layouts["ExamplePayload"]
        self.assertEqual((payload.size, payload.alignment), (8, 4))
        self.assertEqual([(f.offset, f.size) for f in payload.fields],
                         [(0, 5), (0, 4), (0, 4)])
        envelope = layouts["ExampleEnvelope"]
        self.assertEqual((envelope.size, envelope.alignment), (16, 4))
        self.assertEqual([(f.offset, f.size) for f in envelope.fields],
                         [(0, 1), (4, 8), (12, 1)])

    def test_packed_screen_word_uses_target_long_layout(self) -> None:
        layout = _header_structure_layouts()["KfScreenVertex"]
        self.assertEqual((layout.size, layout.alignment), (8, 4))
        self.assertEqual([(f.offset, f.size, f.name, f.datatype) for f in layout.fields],
                         [(0, 4, "sxy", "long"), (4, 2, "sz", "s16"),
                          (6, 2, "p2", "s16")])

    def test_curated_inventories_cover_the_wip_universe(self) -> None:
        counts = validate(RETAIL_CONFIG)
        self.assertEqual(counts["functions"], 471)
        self.assertEqual(counts["signatures_started"], 471)
        self.assertEqual(counts["typed_returns"], 471)
        self.assertEqual(counts["parameterized"], 306)
        self.assertEqual(counts["data"], 2920)
        self.assertGreaterEqual(counts["functions_named"], 240)
        self.assertGreaterEqual(counts["data_named"], 100)
        self.assertEqual(counts["structures"], 108)
        self.assertEqual(counts["structure_fields"], 815)
        self.assertEqual(counts["structure_fields_named"], 727)

    def test_animation_cache_slots_share_one_pointer_type_without_layout_changes(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        slots = (
            ("KfActor", 0x48, 0x34, "animation_cache"),
            ("KfEffectRecord", 0x3C, 0x34, "animation_cache"),
            ("KfMapEvent", 0x44, 0x3C, "animation_cache"),
            ("KfPlayerState", 0xE0, 0x74, "weapon_animation_cache"),
            ("KfEffectSprite", 0x1C, 0x18, "animation_cache"),
        )
        for structure, size, offset, name in slots:
            with self.subTest(structure=structure):
                self.assertEqual(structures[structure].size, size)
                self.assertEqual(_structure_field(structure, offset),
                                 (name, "KfPoolRecord *", 4))
        # The rotation view occupies 0x34..0x3b, not the cache pointer.
        self.assertEqual(_structure_field("KfMapEvent", 0x34),
                         ("rotation", "SVECTOR", 8))
        self.assertEqual(_structure_field("KfMapEvent", 0x40),
                         ("rotation_target", "s16", 2))

    def test_animation_binder_slot_api_belongs_to_pool_header(self) -> None:
        identity = load_function_identities(RETAIL_CONFIG)[("GAME.EXE", 0x800205D4)]
        self.assertEqual(identity.parameters,
                         "KfPoolRecord ** owner_slot;u16 asset_index;u16 clip_index;"
                         "u16 phase;u16 vertex_count")
        pool_header = (REPO / "include/kf/pool.h").read_text()
        render_header = (REPO / "include/kf/game_render.h").read_text()
        self.assertIn("extern u16 *render_bind_animated_instance(\n"
                      "    KfPoolRecord **owner_slot", pool_header)
        self.assertNotIn("extern u16 *render_bind_animated_instance", render_header)
        self.assertIn("#include <kf/pool.h>", render_header)

    def test_animation_slot_renderers_use_direct_internal_and_vendor_headers(self) -> None:
        for name in ("entity_render", "map_event_render", "geometry_render"):
            with self.subTest(unit=name):
                source = (REPO / f"src/game/{name}.c").read_text()
                self.assertNotIn("#include <kf/game.h>", source)
                self.assertIn("#include <kf/game_asset.h>", source)
                self.assertIn("#include <kf/psyq.h>", source)

    def test_animation_pool_record_fields_and_complete_owner(self) -> None:
        fields = (
            (0x00, "state", "KfAnimationCacheState", 2),
            (0x02, "asset_index", "u16", 2),
            (0x04, "clip_index", "u16", 2),
            (0x06, "keyframe_index", "u16", 2),
            (0x08, "rest_morph", "KfMorphObject *", 4),
            (0x0C, "cached_vertices", "SVECTOR *", 4),
            (0x10, "owner_slot", "KfPoolRecord **", 4),
        )
        for offset, name, datatype, size in fields:
            self.assertEqual(_structure_field("KfPoolRecord", offset),
                             (name, datatype, size))
        game = index("GAME.EXE")
        datum = game.data_owner(0x800910C0)
        self.assertEqual((datum.name, datum.datatype, datum.size),
                         ("game_graphics_runtime", "KfGraphicsRuntimeGame", 0x249CC))
        self.assertEqual(_structure_field('KfGraphicsRuntimeGame', 0x20228),
                         ('pool_records', 'KfPoolRecord[12]', 0xF0))
        claims = load_manifest().by_name()["game.pool"].data
        self.assertEqual(claims, ())
        self.assertEqual(game.data_owner(0x800911AF), datum)
        self.assertEqual(game.data_owner(0x800911B0), datum)

    def test_animation_binder_and_cache_lifecycle_share_contiguous_ownership(self) -> None:
        manifest = load_manifest()
        unit = manifest.by_name()["game.pool"]
        self.assertNotIn("game.render_bind_animated_instance", manifest.by_name())
        self.assertEqual(unit.source_path, REPO / "src/game/pool.c")
        self.assertEqual([function.va for function in unit.functions], [
            0x800205D4, 0x80020978, 0x800209A8, 0x800209E4,
            0x80020A2C, 0x80020A98, 0x80020B04,
        ])
        for first, second in zip(unit.functions, unit.functions[1:]):
            self.assertEqual(first.va + first.body_size, second.va)
        self.assertEqual(unit.functions[-1].va + unit.functions[-1].body_size,
                         0x80020B4C)

    def test_game_tmd_buffer_starts_belong_to_one_owner_without_capacity_claims(self) -> None:
        game = index("GAME.EXE")
        datum = game.datum(0x80070E98)
        for base, size in ((0x800911B0, 8), (0x800930F0, 24)):
            for offset in range(size):
                self.assertIsNone(game.datum(base + offset))
                self.assertEqual(game.data_owner(base + offset), datum)
        self.assertEqual(_structure_field('KfGraphicsRuntimeGame', 0x20318),
                         ('unknown_projection_morph_20318', 'u8[16008]', 0x3E88))
        self.assertIn('subobject extents remain unresolved', datum.note)
        opening = index("OPEN.EXE").data_owner(0x80069B80)
        self.assertEqual((opening.name, opening.datatype, opening.size),
                         ("open_graphics_runtime", "KfGraphicsRuntimeOpen", 0x24788))
        self.assertEqual(_structure_field("KfGraphicsRuntimeOpen", 0x20138),
                         ("tmd_projected_vertices", "KfScreenVertex[1000]", 0x1F40))

    def test_tmd_buffer_relocations_preserve_numeric_interior_targets(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        expected = {
            0x8001C624: (0x800911B0, "tmd_projected_vertices"),
            0x8001C6C4: (0x800911B0, "tmd_projected_vertices"),
            0x8001C770: (0x800911B0, "tmd_projected_vertices"),
            0x800208B0: (0x800930F8, "tmd_morph_scratch"),
            0x800208FC: (0x800930F0, "tmd_morph_scratch"),
        }
        found = {
            parse_int(row["site_va"]): row for row in rows
            if row["image"] == "GAME.EXE" and parse_int(row["site_va"]) in expected
        }
        self.assertEqual(set(found), set(expected))
        for site, (target, name) in expected.items():
            row = found[site]
            self.assertEqual((parse_int(row["target_va"]), row["target_name"]),
                             (target, 'game_graphics_runtime'))
            self.assertEqual(parse_int(row["paired_site_va"]), site + 4)
            self.assertEqual((row["kind"], row["opcode"], row["status"]),
                             ("mips_hi16_lo16", "lui+addiu", "reviewed"))

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
        self.assertEqual(actor_fields["spawn_chance"].meaning_confidence, "supported")
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
        self.assertEqual(player_state_fields["fire_defense_timer"].offset, 0x50)
        self.assertEqual(player_state_fields["illusion_staff_timer"].offset, 0x52)
        self.assertEqual(player_state_fields["map_variant"].offset, 0x0C)
        self.assertEqual(
            player_state_fields["illusion_staff_timer"].meaning_confidence,
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
        self.assertEqual(len(rows), 18)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = (
                f"{identity.return_type} {identity.name}({parameters})"
            )
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

    def test_reviewed_unresolved_functions_keep_address_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_unresolved_functions.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        expected = {
            ("GAME.EXE", 0x800365F8),
            ("GAME.EXE", 0x80036E30),
            ("GAME.EXE", 0x8003AC4C),
        }

        self.assertEqual(
            {(row["image"], parse_int(row["va"])) for row in rows}, expected
        )
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(identity.name_confidence, "address-only")
            self.assertEqual(identity.signature_confidence, "supported")
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

    def test_save_layouts_live_in_the_save_owner_header(self) -> None:
        save_header = (REPO / "include/kf/game_save.h").read_text()
        for structure in (
            "KfPsxSaveHeader",
            "KfSaveSlotSummary",
            "KfSaveDirectory",
            "KfSaveHeader",
            "KfSavePayload",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, save_header)

    def test_game_cd_layout_lives_in_the_game_cd_header(self) -> None:
        game_cd = (REPO / "include/kf/game_cd.h").read_text()
        declaration = "typedef struct KfCdFileEntry"
        self.assertIn(declaration, game_cd)

    def test_tmd_layouts_live_in_the_shared_tmd_header(self) -> None:
        tmd_header = (REPO / "include/kf/tmd.h").read_text()
        for structure in (
            "KfTmdHeader",
            "KfTmdObject",
            "KfTmdF3",
            "KfTmdG3",
            "KfTmdF4",
            "KfTmdG4",
            "KfTmdFt3",
            "KfTmdGt3",
            "KfTmdFt4",
            "KfTmdGt4",
            "KfScreenVertex",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, tmd_header)

    def test_common_tmd_api_has_one_owner_header(self) -> None:
        expected = {
            "tmd_get_object": ("KfTmdObject *", "u16 object_index"),
            "tmd_prepare_primitive_indices": ("void", ""),
            "tmd_project_vertices": ("void", "s32 count"),
            "tmd_register": ("void", "KfTmdSlot slot;u8 *tmd"),
            "tmd_release_last_allocation": ("void", "KF_ENUM_PARAM(KfTmdSlot, s32) slot"),
            "tmd_select": ("void", "KfTmdSlot slot"),
            "tmd_select_object_vertices": ("void", "u16 object_index"),
            "tmd_set_current_vertices": ("void", "SVECTOR *vertices"),
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        common = (REPO / "include/kf/tmd.h").read_text()
        for image, filename in (
            ("GAME.EXE", "game_render.h"),
            ("OPEN.EXE", "open_render.h"),
        ):
            header = (REPO / "include/kf" / filename).read_text()
            self.assertIn("#include <kf/tmd.h>", header)
            functions = {
                row.name: row for (owner_image, _va), row in identities.items()
                if owner_image == image
            }
            for name, (return_type, parameters) in expected.items():
                with self.subTest(image=image, name=name):
                    identity = functions[name]
                    self.assertEqual(identity.return_type, return_type)
                    self.assertEqual(identity.parameters, parameters)
                    arguments = parameters.replace(";", ", ") or "void"
                    separator = "" if return_type.endswith("*") else " "
                    declaration = f"extern {return_type}{separator}{name}({arguments});"
                    self.assertEqual(common.count(declaration), 1)
                    self.assertNotIn(f"{name}(", header)

    def test_audio_layouts_live_in_the_audio_owner_header(self) -> None:
        audio_header = (REPO / "include/kf/audio.h").read_text()
        for structure in (
            "SoundRef",
            "KfAudioVoiceSlots",
            "KfAudioState",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, audio_header)

    def test_math_layouts_live_in_the_game_math_header(self) -> None:
        math_header = (REPO / "include/kf/game_math.h").read_text()
        for structure in (
            "KfVecXZs",
            "KfVec3s",
            "KfVec3i",
            "KfEulerAngles",
        ):
            declaration = f"struct {structure} {{"
            self.assertIn(declaration, math_header)

    def test_actor_layouts_live_in_the_actor_owner_header(self) -> None:
        actor_header = (REPO / "include/kf/game_actor.h").read_text()
        for structure in (
            "KfActorDefinition",
            "KfActorActionProfile",
            "KfActorPlacement",
            "KfActor",
            "KfActorState",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, actor_header)

    def test_map_layouts_live_in_the_map_owner_header(self) -> None:
        map_header = (REPO / "include/kf/game_map.h").read_text()
        for structure in (
            "KfMapCell",
            "KfMapCopyRegion",
            "KfMapObjectLinkFields",
            "KfMapObjectHingedContainer",
            "KfMapObjectPlacement",
            "KfMapObjectDefinition",
            "KfMapObject",
            "KfCameraPathPoint",
            "KfCameraPathState",
            "KfDialoguePageLimits",
            "KfMapEventDefinition",
            "KfMapEvent",
            "KfMapObjectState",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, map_header)

        for union in ("KfMapObjectLink", "KfMapObjectSpawn"):
            self.assertIn(f"typedef union {union}", map_header)

    def test_floor_item_layouts_live_in_the_item_owner_header(self) -> None:
        item_header = (REPO / "include/kf/item.h").read_text()
        for structure in ("KfFloorItemPlacement", "KfFloorItem"):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, item_header)

    def test_magic_layout_lives_in_the_magic_owner_header(self) -> None:
        magic_header = (REPO / "include/kf/magic.h").read_text()
        declaration = "typedef struct KfMagicRecord"
        self.assertIn(declaration, magic_header)

    def test_effect_layouts_live_in_the_effect_owner_header(self) -> None:
        effect_header = (REPO / "include/kf/game_effect.h").read_text()
        for structure in ("KfEffectRecord", "KfEffectState"):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, effect_header)

    def test_equipment_layouts_live_in_the_equipment_owner_header(self) -> None:
        equipment_header = (
            REPO / "include/kf/game_equipment.h"
        ).read_text()
        for structure in ("KfWeaponRecord", "KfArmorRecord"):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, equipment_header)

    def test_player_layouts_live_in_the_player_owner_header(self) -> None:
        player_header = (REPO / "include/kf/game_player.h").read_text()
        self.assertNotIn("#include <kf/semantic_types.h>", player_header)
        for structure in (
            "KfPlayerProgressState",
            "KfPlayerLevelGrowth",
            "KfPlayerVitals",
            "KfPlayerAttackChargeState",
            "KfPlayerMotionState",
            "KfPlayerState",
            "KfFloorEntryCell",
        ):
            declaration = f"typedef struct {structure}"
            self.assertIn(declaration, player_header)

    def test_collision_layout_lives_in_the_collision_owner_header(self) -> None:
        collision_header = (
            REPO / "include/kf/game_collision.h"
        ).read_text()
        declaration = "typedef struct KfCollisionTarget"
        self.assertIn(declaration, collision_header)
        self.assertIn("extern KfCollisionTarget collision_target;", collision_header)

    def test_render_layouts_live_in_their_owner_headers(self) -> None:
        owners = {
            "render_types.h": (
                "KfPrimitiveBuffer", "KfOrderingTable", "KfCellWindow", "KfSpriteQuad",
            ),
            "game_asset.h": ("KfAssetHeader",),
            "game_render.h": (
                "KfHudSprite",
                "KfEffectSprite",
                "KfDisplayState",
                "KfTmdState",
                "KfRenderState",
            ),
            "notify.h": ("KfNotificationSprite",),
            "open_render.h": (
                "KfDisplayStateOpen",
                "KfTmdStateOpen",
                "KfRenderStateOpen",
            ),
        }
        for header_name, structures in owners.items():
            owner_header = (REPO / "include/kf" / header_name).read_text()
            for structure in structures:
                declaration = f"typedef struct {structure}"
                self.assertIn(declaration, owner_header)

    def test_sources_include_semantic_owner_headers_directly(self) -> None:
        self.assertFalse((REPO / "include/kf/semantic_types.h").exists())
        for directory, pattern in (("src", "*.c"), ("include", "*.h")):
            for source in (REPO / directory).rglob(pattern):
                self.assertNotIn("kf/semantic_types.h", source.read_text())

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
            "weapon_records_load_and_mirror_angles",
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

        data_identities = load_data_identities(RETAIL_CONFIG)
        expected_state = {
            0x80057B30: ("player_previous_input", "load", "u32"),
            0x80057E68: ("player_movement_velocity_limit", "bss", "s32"),
            0x80057E70: ("player_turn_step_limit", "bss", "s32"),
        }
        for va, (name, storage, datatype) in expected_state.items():
            datum = data_identities[("GAME.EXE", va)]
            self.assertEqual(
                (
                    datum.name,
                    datum.scope,
                    datum.storage,
                    datum.datatype,
                    datum.owner,
                    datum.confidence,
                ),
                (name, "static", storage, datatype, "player", "supported"),
            )

        _, structural_rows = read_tsv(RETAIL_CONFIG / "data.tsv")
        structural_by_va = {
            parse_int(row["va"]): row
            for row in structural_rows
            if row["image"] == "GAME.EXE"
        }
        self.assertEqual(structural_by_va[0x80057E68]["kind"], "bss")
        self.assertEqual(structural_by_va[0x80057E70]["kind"], "bss")

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
        expected_player_state = {
            0x80018934: ("player_previous_input", "load"),
            0x80018A5C: ("player_movement_velocity_limit", "bss"),
            0x80018A7C: ("player_turn_step_limit", "bss"),
        }
        for site, (name, region) in expected_player_state.items():
            self.assertEqual(
                (by_site[site]["target_name"], by_site[site]["target_region"]),
                (name, region),
            )
        self.assertEqual(by_site[0x8001882C]["channel"], "instruction-word")
        self.assertEqual(
            by_site[0x80023600]["target_name"],
            "player_apply_fire_defense_boost",
        )

    def test_player_warp_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_semantic_player_warp.tsv"
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

    def test_actor_definition_transition_call_is_named(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        call = next(
            row
            for row in rows
            if row["image"] == "GAME.EXE"
            and parse_int(row["site_va"]) == 0x8002FF30
        )
        self.assertEqual(
            call["target_name"],
            "actor_transform_definition5_to6",
        )
        self.assertEqual(call["status"], "reviewed")

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

    def test_equipment_tu_is_gapless_and_supported(self) -> None:
        evidence_path = CONFIG / "evidence/game_tu_equipment.tsv"
        _, rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(rows), 3)

        spans = [
            (parse_int(row["va"]), parse_int(row["extent"]))
            for row in rows
        ]
        self.assertEqual(
            spans,
            [
                (0x800150A8, 0x54),
                (0x800150FC, 0x2C),
                (0x80015128, 0x3C),
            ],
        )
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
        self.assertEqual(units.count('source = "src/game/equipment.c"'), 1)
        self.assertNotIn('source = "src/game/player_weapon.c"', units)
        self.assertNotIn('source = "src/game/asset_aux_block.c"', units)

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
        release = load_function_identities(RETAIL_CONFIG, required=True)[
            ("GAME.EXE", 0x8002AF0C)
        ]
        self.assertEqual(release.name, "menu_release_item_model")
        self.assertEqual(release.owner, "menu")
        pending = load_data_identities(RETAIL_CONFIG)[
            ("GAME.EXE", 0x80057B6C)
        ]
        self.assertEqual(
            (pending.name, pending.datatype, pending.owner),
            ("menu_item_model_allocation_pending", "s32", "menu"),
        )

        _, relocations = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        calls = [
            row
            for row in relocations
            if row["image"] == "GAME.EXE"
            and row["target_va"] == "0x8002af0c"
        ]
        self.assertEqual(len(calls), 8)
        self.assertEqual(
            {row["target_name"] for row in calls},
            {"menu_release_item_model"},
        )
        state_references = [
            row
            for row in relocations
            if row["image"] == "GAME.EXE"
            and row["target_va"] == "0x80057b6c"
        ]
        self.assertEqual(len(state_references), 3)
        self.assertEqual(
            {row["target_name"] for row in state_references},
            {"menu_item_model_allocation_pending"},
        )
        self.assertEqual(
            {row["status"] for row in state_references},
            {"reviewed"},
        )

    def test_pad_vendor_units_own_private_state_and_literals(self) -> None:
        campaigns = (
            ("GAME.EXE", "game_vendor_pad.tsv", 0x800500B8, 0x8005023C),
            ("OPEN.EXE", "open_vendor_pad.tsv", 0x8002FE8C, 0x80030010),
        )
        campaign_addresses = {}
        for image, filename, start, end in campaigns:
            _, rows = read_tsv(CONFIG / f"evidence/{filename}")
            self.assertEqual(len(rows), 6)
            spans = [
                (parse_int(row["va"]), parse_int(row["extent"]))
                for row in rows
            ]
            self.assertEqual(spans[0][0], start)
            self.assertEqual(spans[-1][0] + spans[-1][1], end)
            for (va, extent), (next_va, _next_extent) in zip(spans, spans[1:]):
                self.assertEqual(va + extent, next_va)
            campaign_addresses[image] = tuple(va for va, _extent in spans)

        identities = load_function_identities(RETAIL_CONFIG, required=True)
        _, vendored_rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in vendored_rows
        }
        expected_names = (
            "PadInit",
            "PadRead",
            "PadStop",
            "pad_init_bad_identifier",
            "pad_read_bad_identifier",
            "pad_stop_bad_identifier",
        )
        for image, addresses in campaign_addresses.items():
            for va, name in zip(addresses, expected_names):
                self.assertNotIn((image, va), identities)
                vendor = vendored[(image, va)]
                self.assertEqual(vendor["name"], name)
                self.assertEqual(
                    (vendor["provider"], vendor["library"], vendor["confidence"]),
                    (
                        "Sony Computer Entertainment",
                        "LIBETC.LIB",
                        "sdk-lineage-supported",
                    ),
                )

        data_identities = load_data_identities(RETAIL_CONFIG)
        expected_state = {
            ("GAME.EXE", 0x80058020): ("pad_buf", "bss", "u32"),
            ("GAME.EXE", 0x80058028): ("pad_status", "bss", "u32"),
            ("GAME.EXE", 0x8006BD88): ("PadIdentifier", "bss", "int"),
            ("OPEN.EXE", 0x80037760): ("pad_buf", "bss", "u32"),
            ("OPEN.EXE", 0x80037768): ("pad_status", "bss", "u32"),
            ("OPEN.EXE", 0x80049528): ("PadIdentifier", "bss", "int"),
        }
        expected_references = {
            "pad_buf": 4,
            "pad_status": 1,
            "PadIdentifier": 6,
        }
        _, relocations = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        for identity, (name, storage, datatype) in expected_state.items():
            datum = data_identities[identity]
            self.assertEqual(
                (
                    datum.name,
                    datum.scope,
                    datum.storage,
                    datum.datatype,
                    datum.owner,
                    datum.confidence,
                ),
                (name, "global" if name == "PadIdentifier" else "static",
                 storage, datatype, "pad", "supported"),
            )
            references = [
                row
                for row in relocations
                if row["image"] == identity[0]
                and row["target_va"] == f"0x{identity[1]:08x}"
            ]
            self.assertEqual(len(references), expected_references[name])
            self.assertEqual({row["target_name"] for row in references}, {name})
            self.assertEqual({row["status"] for row in references}, {"reviewed"})

        sources = (
            (REPO / "src/vendor/game_libetc_pad.c").read_text(),
            (REPO / "src/vendor/open_libetc_pad.c").read_text(),
        )
        game_state = (REPO / "include/kf/game_state.h").read_text()
        vendor_header = (REPO / "include/kf/psyq_pad.h").read_text()
        for source in sources:
            self.assertIn("static u32 pad_buf", source)
            self.assertIn("static u32 pad_status", source)
            self.assertIn("\nint PadIdentifier;", source)
            self.assertNotIn("static s32 PadIdentifier;", source)
            self.assertNotIn("pad_buf = 0", source)
            self.assertNotIn("static u32 pad_status =", source)
            self.assertNotIn("DAT_", source)
            for literal in (
                "PAD_init: Bad PadIdentifier %d\\n",
                "PAD_dr  : Bad PadIdentifier %d\\n",
                "StopPAD : Bad PadIdentifier %d\\n",
            ):
                self.assertIn(literal, source)
        self.assertNotIn("DAT_80058020", game_state)
        self.assertNotIn("DAT_80058028", game_state)
        self.assertIn("extern u32 PadInit(s32 identifier);", vendor_header)
        self.assertIn("extern u32 PadRead();", vendor_header)
        self.assertIn("#include <LIBETC.H>", vendor_header)
        self.assertFalse((REPO / "include/kf/game_pad.h").exists())

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
        for va in (0x800583E8, 0x800583F4):
            self.assertNotIn(("GAME.EXE", va), data_identities)
        for offset, name in ((0x300, "number_atlas"), (0x30C, "glyph_atlas")):
            self.assertEqual(_structure_field("KfMenuAssets", offset),
                             (name, "MenuSpriteDef", 12))
        self.assertEqual(
            data_identities[("GAME.EXE", 0x800580E8)].datatype,
            "KfMenuAssets",
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
        self.assertEqual(len(rows), 7)
        for row in rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

        self.assertEqual(
            _structure_field("KfScreenVertex", 0x00), ("sxy", "long", 4)
        )
        self.assertEqual(
            _structure_field("KfScreenVertex", 0x04), ("sz", "s16", 2)
        )
        self.assertEqual(
            _structure_field("KfScreenVertex", 0x06), ("p2", "s16", 2)
        )

        enqueuers = rows[3:]
        spans = [
            (parse_int(row["va"]), parse_int(row["extent"]))
            for row in enqueuers
        ]
        self.assertEqual(
            spans,
            [
                (0x8001C7F8, 0xF38),
                (0x8001D730, 0x6E8),
                (0x8001DE18, 0x418),
                (0x8001E230, 0x250),
            ],
        )
        for (va, extent), (next_va, _next_extent) in zip(spans, spans[1:]):
            self.assertEqual(va + extent, next_va)

        data = load_data_identities(RETAIL_CONFIG)
        self.assertEqual(
            [
                (data[("GAME.EXE", va)].name, data[("GAME.EXE", va)].datatype)
                for va in (0x80057B58, 0x80057B5C, 0x80057B60, 0x80057B64)
            ],
            [
                ("tmd_textured_primitive_color", "CVECTOR"),
                ("model_textured_primitive_color", "CVECTOR"),
                ("map_textured_primitive_color", "CVECTOR"),
                ("render_sprite_light_normal", "SVECTOR"),
            ],
        )

        source = (REPO / "src/game/render_enqueuers.c").read_text()
        self.assertEqual(source.count("\nADDRESS("), 4)
        self.assertEqual(source.count("\nDATA("), 4)
        self.assertNotIn("DAT_80057b63", source)
        self.assertNotIn("DAT_80057b64", source)
        for obsolete in (
            "src/game/render_enqueue.c",
            "src/game/render_enqueue_tmd.c",
            "src/game/render_enqueue_model.c",
        ):
            self.assertFalse((REPO / obsolete).exists())

        _, relocations = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        by_site = {
            parse_int(row["site_va"]): row
            for row in relocations
            if row["image"] == "GAME.EXE"
        }
        self.assertEqual(
            by_site[0x8001DE98]["target_name"],
            "map_textured_primitive_color",
        )
        self.assertEqual(
            by_site[0x8001E41C]["target_name"],
            "render_sprite_light_normal",
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
            0x800375D8: ("cd_read_location", "bss", "CdlLOC", 0x04),
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
        self.assertEqual(len(campaign_rows), 42)
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

    def test_open_startup_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_startup.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 2)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)
            self.assertEqual(row["current_match"], "100.000000000% exact")

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_startup" in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 7)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})
        self.assertNotIn(
            0x8001376C,
            {
                parse_int(row["site_va"])
                for row in relocation_rows
                if row["image"] == "OPEN.EXE"
            },
        )

    def test_open_opening_render_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_opening_render.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 2)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)
            self.assertEqual(row["current_match"], "100.000000000% exact")

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_opening_render"
            in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 18)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})
        body_data = [
            row
            for row in campaign_rows
            if row["kind"] == "mips_hi16_lo16"
        ]
        self.assertEqual(
            {(row["target_name"], parse_int(row["target_va"])) for row in body_data},
            {("open_graphics_runtime", 0x80049A68), ("open_graphics_runtime", 0x80069A6C)},
        )

    def test_open_opening_helpers_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_opening_helpers.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 2)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)
            self.assertEqual(row["current_match"], "100.000000000% exact")

        _, vendored_rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in vendored_rows
        }
        pad_read = vendored[("OPEN.EXE", 0x8002FF00)]
        self.assertEqual(
            (pad_read["name"], pad_read["library"], pad_read["module"]),
            ("PadRead", "LIBETC.LIB", "PAD"),
        )
        self.assertNotIn(("OPEN.EXE", 0x8002FF00), identities)

        data = load_data_identities(RETAIL_CONFIG)
        action = data[("OPEN.EXE", 0x80043178)]
        self.assertEqual(
            (action.name, action.storage, action.datatype, action.size),
            ("opening_input_action", "bss", "KfOpeningInputAction", 4),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_opening_helpers"
            in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 24)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})
        action_rows = [
            row
            for row in campaign_rows
            if parse_int(row["target_va"]) == 0x80043178
        ]
        self.assertEqual(len(action_rows), 12)
        self.assertEqual(
            {row["target_name"] for row in action_rows}, {"opening_input_action"}
        )

    def test_open_libetc_pad_run_is_excluded_as_vendored(self) -> None:
        evidence_path = CONFIG / "evidence/open_vendor_libetc_pad.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        self.assertEqual(len(evidence_rows), 7)

        expected = {
            0x8002FE30: "critical_section_set",
            0x8002FE8C: "PadInit",
            0x8002FF00: "PadRead",
            0x8002FF44: "PadStop",
            0x8002FF80: "pad_init_bad_identifier",
            0x8002FFB0: "pad_read_bad_identifier",
            0x8002FFE0: "pad_stop_bad_identifier",
        }
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        _, vendored_rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        vendored = {
            (row["image"], parse_int(row["va"])): row
            for row in vendored_rows
        }
        for va, name in expected.items():
            self.assertNotIn(("OPEN.EXE", va), identities)
            row = vendored[("OPEN.EXE", va)]
            self.assertEqual(row["name"], name)
            self.assertEqual(
                (row["provider"], row["library"], row["confidence"]),
                (
                    "Sony Computer Entertainment",
                    "LIBETC.LIB",
                    "sdk-lineage-supported",
                ),
            )
        self.assertEqual(
            {parse_int(row["va"]) for row in evidence_rows}, set(expected)
        )
        self.assertEqual(
            {row["verdict"] for row in evidence_rows},
            {
                "Sony LIBETC INTR.OBJ static; exclude from OPEN game progress",
                "Sony LIBETC PAD.OBJ PadInit; exclude from OPEN game progress",
                "Sony LIBETC PAD.OBJ PadRead; exclude from OPEN game progress",
                "Sony LIBETC PAD.OBJ PadStop; exclude from OPEN game progress",
                "Sony version-skewed LIBETC PAD.OBJ static; exclude from OPEN game progress",
            },
        )

    def test_open_sprite_f4_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_sprite_f4.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(row["current_match"], "100.000000000% exact")

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_sprite_f4"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 5)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "open_graphics_runtime",
                "SetPolyF4",
                "AddPrim",
                "sprite_add_f4",
            },
        )

    def test_open_camera_path_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_camera_path.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 3)
        expected_scores = {
            0x80013CF4: "100.000000000% exact",
            0x80014004: "100.000000000% exact",
            0x80014100: "100.000000000% exact",
        }
        for row in evidence_rows:
            va = parse_int(row["va"])
            identity = identities[(row["image"], va)]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)
            self.assertEqual(row["current_match"], expected_scores[va])

        data = load_data_identities(RETAIL_CONFIG)
        state = data[("OPEN.EXE", 0x800757D8)]
        self.assertEqual(
            (state.name, state.storage, state.datatype, state.size),
            ("opening_camera_path_state", "bss", "KfCameraPathState", 0x64),
        )
        self.assertFalse(
            any(
                image == "OPEN.EXE" and 0x800757D8 < va < 0x8007583C
                for image, va in data
            )
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_camera_path" in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 71)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})
        state_rows = [
            row
            for row in relocation_rows
            if row["image"] == "OPEN.EXE"
            and 0x800757D8 <= parse_int(row["target_va"]) < 0x8007583C
        ]
        self.assertEqual(len(state_rows), 77)
        self.assertEqual(
            {row["target_name"] for row in state_rows},
            {"opening_camera_path_state"},
        )

    def test_open_opening_scene1_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_opening_scene1.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 2)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)
            self.assertEqual(row["current_match"], "100.000000000% exact")

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_opening_scene1"
            in row["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 21)
        self.assertEqual({row["status"] for row in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {row["target_name"] for row in campaign_rows})
        self.assertEqual(
            {
                row["target_name"]
                for row in campaign_rows
                if parse_int(row["target_va"]) in {0x800143DC, 0x8001455C}
            },
            {"opening_scene1_draw_fade", "opening_scene1_run"},
        )

    def test_open_entity_transition_campaign_matches_curated_evidence(self) -> None:
        evidence_path = (
            CONFIG / "evidence/open_semantic_opening_entity_transition.tsv"
        )
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";"))
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(row["current_match"], "99.921260000% complete C")

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_entity_transition"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 12)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {reloc["target_name"] for reloc in campaign_rows})
        state_rows = [
            reloc
            for reloc in campaign_rows
            if parse_int(reloc["target_va"]) in {0x800498F8, 0x80049912}
        ]
        self.assertEqual(len(state_rows), 3)
        self.assertEqual(
            {reloc["target_name"] for reloc in state_rows},
            {"opening_entity_state"},
        )

    def test_open_scene3_campaign_matches_curated_evidence(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_opening_scene3.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(row["current_match"], "99.931370000% complete C")

        self.assertEqual(
            identities[("OPEN.EXE", 0x80019600)].name,
            "lighting_set_color_matrix",
        )
        self.assertEqual(
            identities[("OPEN.EXE", 0x8001A1B0)].name,
            "audio_set_listener_transform",
        )

        data = load_data_identities(RETAIL_CONFIG)
        expected_data = {
            0x800356D0: ("opening_scene3_camera_path", "KfCameraPathPoint[3]", 0x54),
            0x80035878: ("opening_scene3_overlay_rects", "KfScreenRect[2]", 0x10),
            0x80037284: ("opening_scene3_overlay_uv", "u8[8]", 0x08),
            0x8003728C: ("opening_scene3_overlay_color", "CVECTOR", 0x04),
        }
        for va, expected in expected_data.items():
            datum = data[("OPEN.EXE", va)]
            self.assertEqual((datum.name, datum.datatype, datum.size), expected)

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_scene3"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 55)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {reloc["target_name"] for reloc in campaign_rows})
        self.assertEqual(
            {
                reloc["target_name"]
                for reloc in campaign_rows
                if parse_int(reloc["target_va"])
                in {0x80014804, 0x80019600, 0x8001A1B0}
            },
            {
                "opening_scene3_run",
                "lighting_set_color_matrix",
                "audio_set_listener_transform",
            },
        )

    def test_open_ending_scene_campaign_matches_curated_evidence(self) -> None:
        evidence_path = (
            CONFIG / "evidence/open_semantic_opening_ending_scene.tsv"
        )
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(row["current_match"], "100.000000000% exact")

        data = load_data_identities(RETAIL_CONFIG)
        path = data[("OPEN.EXE", 0x80035724)]
        self.assertEqual(
            (path.name, path.storage, path.datatype, path.size),
            ("opening_ending_camera_path", "load", "KfCameraPathPoint[9]", 0xFC),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_ending_scene"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 46)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {reloc["target_name"] for reloc in campaign_rows})
        self.assertEqual(
            {
                reloc["target_name"]
                for reloc in campaign_rows
                if parse_int(reloc["target_va"])
                in {0x80014B34, 0x80035724, 0x80069A89, 0x80069AE7}
            },
            {
                "opening_ending_scene_run",
                "opening_ending_camera_path",
                "open_graphics_runtime",
            },
        )

    def test_open_opening_controller_campaign_is_exactly_modeled(self) -> None:
        evidence_path = (
            CONFIG / "evidence/open_semantic_opening_controller.tsv"
        )
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)

        data = load_data_identities(RETAIL_CONFIG)
        path = data[("OPEN.EXE", 0x800372D4)]
        self.assertEqual(
            (path.name, path.storage, path.datatype, path.size),
            ("opening_initial_tim_path", "load", "char[6]", 0x6),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_controller"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 48)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        calls = [reloc for reloc in campaign_rows if reloc["opcode"] == "jal"]
        self.assertNotIn("", {reloc["target_name"] for reloc in calls})
        self.assertEqual(
            {
                reloc["target_name"]
                for reloc in campaign_rows
                if parse_int(reloc["target_va"])
                in {0x800156BC, 0x800372D4, 0x80049A4C, 0x80075850}
            },
            {
                "opening_run",
                "opening_initial_tim_path",
                "open_graphics_runtime",
                "memory_arena_cursor",
            },
        )

    def test_open_opening_scene0_campaign_is_exactly_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_opening_scene0.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(
            identities[("OPEN.EXE", 0x800194D8)].name,
            "opening_scene0_render_frame",
        )
        self.assertEqual(
            identities[("OPEN.EXE", 0x8001A220)].name,
            "sound_ref_play",
        )

        data = load_data_identities(RETAIL_CONFIG)
        path = data[("OPEN.EXE", 0x800354F4)]
        sound = data[("OPEN.EXE", 0x80035874)]
        self.assertEqual(
            (path.name, path.storage, path.datatype, path.size),
            (
                "opening_scene0_camera_path",
                "load",
                "KfCameraPathPoint[17]",
                0x1DC,
            ),
        )
        self.assertEqual(
            (sound.name, sound.storage, sound.datatype, sound.size),
            ("opening_scene0_sound", "load", "SoundRef", 0x3),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_scene0"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 24)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        calls = [reloc for reloc in campaign_rows if reloc["opcode"] == "jal"]
        self.assertNotIn("", {reloc["target_name"] for reloc in calls})
        self.assertEqual(
            {
                reloc["target_name"]
                for reloc in campaign_rows
                if parse_int(reloc["target_va"])
                in {0x80014268, 0x800194D8, 0x8001A220, 0x800354F4, 0x80035874}
            },
            {
                "opening_scene0_run",
                "opening_scene0_render_frame",
                "sound_ref_play",
                "opening_scene0_camera_path",
                "opening_scene0_sound",
            },
        )

    def test_open_opening_scene0_render_is_exactly_modeled(self) -> None:
        evidence_path = (
            CONFIG / "evidence/open_semantic_opening_scene0_render.tsv"
        )
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_scene0_render"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 6)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "SetGeomScreen",
                "display_begin_frame",
                "display_present_frame",
                "opening_render_entities_and_items",
                "opening_render_map_cells",
                "render_set_view_transform",
            },
        )

    def test_open_opening_render_entities_is_exactly_modeled(self) -> None:
        evidence_path = (
            CONFIG / "evidence/open_semantic_opening_render_entities.tsv"
        )
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(
            identities[("OPEN.EXE", 0x80018ECC)].name,
            "opening_entity_render",
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_opening_render_entities"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 6)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "opening_entity_render",
                "opening_entity_state",
                "opening_render_entities",
                "tmd_select",
            },
        )

    def test_open_matrix_campaign_is_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_matrix.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 5)
        self.assertEqual(
            {parse_int(row["va"]) for row in evidence_rows},
            {0x80019598, 0x80019600, 0x8001962C, 0x80019658, 0x8001969C},
        )
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertIn(evidence_path.name, identity.evidence)

        self.assertEqual(
            {row["current_match"] for row in evidence_rows},
            {"100.000000000% exact"},
        )

        self.assertEqual(
            {
                identities[("OPEN.EXE", va)].name
                for va in {
                    0x80019598,
                    0x80019600,
                    0x8001962C,
                    0x80019658,
                    0x8001969C,
                }
            },
            {
                "matrix_interpolate",
                "lighting_set_color_matrix",
                "lighting_set_light_matrix",
                "fog_interpolate_near",
                "fog_set_near",
            },
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_matrix" in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 8)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "matrix_interpolate",
                "SetColorMatrix",
                "SetLightMatrix",
                "open_graphics_runtime",
                "SetFogNear",
            },
        )

    def test_open_color_lerp_campaign_is_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_color_lerp.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 2)
        self.assertEqual(
            {parse_int(row["va"]) for row in evidence_rows},
            {0x800196C4, 0x8001974C},
        )
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertEqual(row["current_match"], "100.000000000% exact")
            self.assertIn(evidence_path.name, identity.evidence)

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_color_lerp" in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 2)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {"color_lerp_cvector"},
        )

    def test_open_sound_ref_is_exactly_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_sound_ref.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)
        self.assertEqual(
            identities[("OPEN.EXE", 0x8001A268)].name,
            "audio_play_voice",
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_sound_ref"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 2)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {"audio_play_voice", "audio_state"},
        )

    def test_open_audio_play_voice_is_exactly_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_audio_play_voice.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 1)
        row = evidence_rows[0]
        identity = identities[(row["image"], parse_int(row["va"]))]
        parameters = ", ".join(identity.parameters.split(";")) or "void"
        signature = f"{identity.return_type} {identity.name}({parameters})"
        self.assertEqual(row["final_name"], identity.name)
        self.assertEqual(row["final_signature"], signature)
        self.assertEqual(row["current_match"], "100.000000000% exact")
        self.assertIn(evidence_path.name, identity.evidence)

        data = load_data_identities(RETAIL_CONFIG)
        slot_index = data[("OPEN.EXE", 0x80037304)]
        self.assertEqual(
            (
                slot_index.name,
                slot_index.storage,
                slot_index.datatype,
                slot_index.size,
            ),
            ("audio_voice_slot_index", "load", "s32", 0x4),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_audio_play_voice"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 12)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "SsUtKeyOff",
                "SsUtKeyOn",
                "audio_play_voice",
                "audio_state",
                "audio_voice_slot_index",
            },
        )

    def test_open_audio_spatial_run_is_exactly_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_audio_spatial.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 5)
        for row in evidence_rows:
            identity = identities[(row["image"], parse_int(row["va"]))]
            parameters = ", ".join(identity.parameters.split(";")) or "void"
            signature = f"{identity.return_type} {identity.name}({parameters})"
            self.assertEqual(row["final_name"], identity.name)
            self.assertEqual(row["final_signature"], signature)
            self.assertEqual(row["current_match"], "100.000000000% exact")
            self.assertIn(evidence_path.name, identity.evidence)

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            reloc
            for reloc in relocation_rows
            if "manual:open_semantic_audio_spatial"
            in reloc["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 16)
        self.assertEqual({reloc["status"] for reloc in campaign_rows}, {"reviewed"})
        self.assertNotIn("", {reloc["target_name"] for reloc in campaign_rows})
        self.assertEqual(
            {reloc["target_name"] for reloc in campaign_rows},
            {
                "SquareRoot0",
                "SsVoKeyOff",
                "audio_play_spatial",
                "audio_play_spatial+0x19c",
                "audio_play_spatial+0x1c0",
                "audio_play_voice",
                "audio_state",
                "rcos",
                "rsin",
                "vector_xz_to_angle",
            },
        )

    def test_open_resources_campaign_matches_curated_identities(self) -> None:
        evidence_path = CONFIG / "evidence/open_semantic_resources.tsv"
        _, evidence_rows = read_tsv(evidence_path)
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        self.assertEqual(len(evidence_rows), 18)
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
        self.assertEqual(len(campaign_rows), 115)
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
        self.assertNotIn(("OPEN.EXE", 0x80069A70), data)
        self.assertNotIn(("OPEN.EXE", 0x80069B28), data)
        self.assertEqual(_structure_field("KfGraphicsRuntimeOpen", 0x20028),
                         ("display_draw_environments", "DRAWENV[2]", 0xB8))
        self.assertEqual(_structure_field("KfGraphicsRuntimeOpen", 0x200E0),
                         ("display_disp_environments", "DISPENV[2]", 0x28))
        allocation_count = data[("OPEN.EXE", 0x80075928)]
        self.assertEqual(
            (allocation_count.name, allocation_count.storage, allocation_count.datatype),
            ("primitive_allocation_count", "bss", "u32"),
        )

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            row
            for row in relocation_rows
            if "manual:open_semantic_render_init"
            in row["provenance"].split(";")
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
        expected = {
            0x8001738C: ("tmd_project_vertices", "s32 count"),
            0x80017458: ("tmd_project_vertices_perspective_right", "s32 count"),
            0x800174FC: ("tmd_project_vertices_shift", "s32 count;u8 shift"),
            0x800175A8: ("tmd_transform_vertices", "s32 count"),
        }
        self.assertEqual({parse_int(row["va"]) for row in rows}, set(expected))
        functions = load_function_identities(RETAIL_CONFIG, required=True)
        for row in rows:
            va = parse_int(row["va"])
            identity = functions[("OPEN.EXE", va)]
            name, parameters = expected[va]
            self.assertEqual(
                (identity.name, identity.return_type, identity.parameters),
                (name, "void", parameters),
            )
            self.assertEqual(
                row["final_signature"],
                f"void {name}({parameters.replace(';', ', ')})",
            )
            self.assertIn(evidence_path.name, identity.evidence)

        data = load_data_identities(RETAIL_CONFIG)
        self.assertNotIn(("OPEN.EXE", 0x80069B80), data)
        self.assertEqual(_structure_field("KfGraphicsRuntimeOpen", 0x20138),
                         ("tmd_projected_vertices", "KfScreenVertex[1000]", 0x1F40))
        self.assertNotIn(("OPEN.EXE", 0x80069B84), data)
        self.assertNotIn(("OPEN.EXE", 0x80069B86), data)
        self.assertNotIn(("OPEN.EXE", 0x8006E1CC), data)
        self.assertEqual(_structure_field("KfGraphicsRuntimeOpen", 0x24784),
                         ("tmd_projection_shift", "s16", 2))

        _, relocation_rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign_rows = [
            item
            for item in relocation_rows
            if "manual:open_semantic_projection" in item["provenance"].split(";")
        ]
        self.assertEqual(len(campaign_rows), 24)
        self.assertEqual({item["status"] for item in campaign_rows}, {"reviewed"})
        by_site = {parse_int(item["site_va"]): item for item in campaign_rows}
        self.assertEqual(by_site[0x800173C8]["target_name"], "open_graphics_runtime")
        self.assertEqual(by_site[0x80017408]["target_name"], "open_graphics_runtime")
        self.assertEqual(by_site[0x8001741C]["target_name"], "ReadSZ2")
        for site in (0x80017470, 0x80017518, 0x800175C4):
            self.assertEqual(by_site[site]["target_name"], "open_graphics_runtime")
            self.assertEqual(parse_int(by_site[site]["target_va"]), 0x80069B80)
        self.assertEqual(by_site[0x800174AC]["target_name"], "open_graphics_runtime")
        self.assertEqual(by_site[0x80017544]["target_name"], "RotTransPers")
        self.assertEqual(by_site[0x80017564]["target_name"], "ReadSZ2")
        self.assertEqual(by_site[0x800175EC]["target_name"], "RotTrans")
        self.assertEqual(
            by_site[0x80019044]["target_name"],
            "tmd_project_vertices_perspective_right",
        )
        self.assertEqual(by_site[0x800190C0]["target_name"], "tmd_project_vertices")

    def test_game_open_diagnostic_sink_is_semantically_modeled(self) -> None:
        evidence_path = CONFIG / "evidence/game_open_semantic_diagnostic_sink.tsv"
        _, rows = read_tsv(evidence_path)
        self.assertEqual(len(rows), 2)

        functions = load_function_identities(RETAIL_CONFIG, required=True)
        expected = {
            ("GAME.EXE", 0x8003AC34),
            ("OPEN.EXE", 0x8001A814),
        }
        self.assertEqual(
            {(row["image"], parse_int(row["va"])) for row in rows}, expected
        )
        for row in rows:
            identity = functions[(row["image"], parse_int(row["va"]))]
            self.assertEqual(
                (identity.name, identity.return_type, identity.parameters),
                ("debug_printf_sink", "void", "const char *format;..."),
            )
            self.assertEqual(
                row["final_signature"],
                "void debug_printf_sink(const char *format, ...)",
            )
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
            "",
        )
        self.assertEqual(
            by_site[0x8001B460]["target_name"],
            "player_state",
        )
        game = index("GAME.EXE")
        path = game.datum(0x80055AF0)
        variant_owner = game.data_owner(0x8009DDAC)
        self.assertEqual(
            (path.name, path.datatype, path.size),
            ("map_resource_path", "char[12]", 0x0C),
        )
        self.assertEqual(
            (variant_owner.name, variant_owner.datatype, variant_owner.size),
            ("map_runtime_state", "KfMapRuntimeState", 0x2360),
        )
        self.assertIsNone(game.datum(0x8009DDAC))
        self.assertEqual(
            _structure_field("KfMapRuntimeState", 0x224),
            ("variant_asset_buffer", "u8 *", 4),
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

    def test_map_animation_calls_agree_with_retail(self) -> None:
        try:
            retail_dir = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail directory is not configured")
        if not (retail_dir / "GAME.EXE").is_file():
            self.skipTest("retail GAME.EXE is required")
        retail = RetailImage.load("GAME.EXE")
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        by_site = {
            parse_int(row["site_va"]): row
            for row in rows
            if row["image"] == "GAME.EXE"
        }
        for site, step in (
            (0x80035070, 200),
            (0x800350A4, 200),
            (0x800350D0, 400),
            (0x800350F4, 200),
            (0x80035110, 200),
        ):
            with self.subTest(site=hex(site)):
                control = decode_control(site, retail.u32(site))
                self.assertIsNotNone(control)
                self.assertTrue(control.call)
                self.assertEqual(control.target, 0x80033820)
                self.assertEqual(retail.u32(site + 4), 0x34060000 | step)
                self.assertEqual(parse_int(by_site[site]["target_va"]), control.target)
                self.assertEqual(by_site[site]["confidence"], "control-flow-reviewed")
                self.assertEqual(by_site[site]["status"], "reviewed")

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
        datum = game.data_owner(0x800910C0)
        self.assertEqual(
            (datum.name, datum.kind, datum.size),
            ("game_graphics_runtime", "bss", 0x249CC),
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
            ("game_exit_code", "KfGameExitCode", "game"),
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
        boss_owner = game.data_owner(0x8009F847)
        self.assertEqual(
            (boss_owner.name, boss_owner.datatype, boss_owner.owner_type),
            ("map_runtime_state", "KfMapRuntimeState", "map_runtime"),
        )
        self.assertIsNone(game.datum(0x8009F847))
        self.assertEqual(
            {
                data_identities[("GAME.EXE", va)].scope
                for va in (0x8005617C, 0x80057B80)
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
            ("map_copy_regions", "KfMapCopyRegion[5]", 0x1E),
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
        map_runtime = game.datum(0x8009DB88)
        interior_event = game.data_owner(0x8009DC8A)
        self.assertEqual(
            (map_runtime.name, map_runtime.datatype, map_runtime.size),
            ("map_runtime_state", "KfMapRuntimeState", 0x2360),
        )
        self.assertEqual(interior_event, map_runtime)
        self.assertIsNone(game.datum(0x8009DDA8))
        self.assertEqual(
            _structure_field("KfMapRuntimeState", 0x220),
            ("current_event", "KfMapEvent *", 4),
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
                "cutting_defense",
                "striking_defense",
                "piercing_defense",
                "poison_resistance",
                "magic_defense",
                "fire_defense",
                "curse_timer",
                "darkness_timer",
                "poison_timer",
                "slowed_timer",
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
                    0x80058060,
                    0x80057E78,
                )
            },
            {"unknown"},
        )
        self.assertEqual(data_identities[("GAME.EXE", 0x8009DB88)].scope, "global")
        self.assertNotIn(("GAME.EXE", 0x8009DDA8), data_identities)


if __name__ == "__main__":
    unittest.main()
