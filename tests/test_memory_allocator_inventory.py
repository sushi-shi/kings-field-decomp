from __future__ import annotations

import unittest

from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


PAIRS = (
    ("GAME.EXE", 0x8001AAB0, "memory_malloc_checked", "void *", "s32 size"),
    ("GAME.EXE", 0x8001AAE8, "memory_allocation_reset", "void", ""),
    ("GAME.EXE", 0x8001AB08, "memory_set_allocation_mode", "void", "s32 mode"),
    ("GAME.EXE", 0x8001ABB0, "memory_capture_system_heap_start", "void", ""),
    ("GAME.EXE", 0x8001ABD0, "memory_reset_system_heap", "void", ""),
    ("GAME.EXE", 0x8001AC0C, "memory_allocate", "void *", "s32 size"),
    ("GAME.EXE", 0x8001AC8C, "memory_release_last", "void", ""),
    ("OPEN.EXE", 0x80015DD4, "memory_malloc_checked", "void *", "s32 size"),
    ("OPEN.EXE", 0x80015E0C, "memory_allocation_reset", "void", ""),
    ("OPEN.EXE", 0x80015E2C, "memory_set_allocation_mode", "void", "s32 mode"),
    ("OPEN.EXE", 0x80015ED4, "memory_capture_system_heap_start", "void", ""),
    ("OPEN.EXE", 0x80015EF4, "memory_reset_system_heap", "void", ""),
    ("OPEN.EXE", 0x80015F30, "memory_allocate", "void *", "s32 size"),
    ("OPEN.EXE", 0x80015FB0, "memory_release_last", "void", ""),
)

STATE = {
    "GAME.EXE": {
        0x800A01F0: ("memory_arena_start", 0x04, "u8 *"),
        0x800A01F4: ("memory_arena_end", 0x04, "u8 *"),
        0x800A01F8: ("memory_arena_cursor", 0x04, "u8 *"),
        0x800A01FC: ("memory_allocation_depth", 0x04, "s32"),
        0x800A0200: ("memory_allocation_entries", 0x40, "u32[16]"),
        0x800A0240: ("memory_system_heap_start", 0x04, "u8 *"),
        0x800A0244: ("memory_system_heap_size", 0x04, "s32"),
    },
    "OPEN.EXE": {
        0x80075848: ("memory_arena_start", 0x04, "u8 *"),
        0x8007584C: ("memory_arena_end", 0x04, "u8 *"),
        0x80075850: ("memory_arena_cursor", 0x04, "u8 *"),
        0x80075854: ("memory_allocation_depth", 0x04, "s32"),
        0x80075858: ("memory_allocation_entries", 0x40, "u32[16]"),
        0x80075898: ("memory_system_heap_start", 0x04, "u8 *"),
        0x8007589C: ("memory_system_heap_size", 0x04, "s32"),
    },
}


class MemoryAllocatorInventoryTests(unittest.TestCase):
    def test_campaign_matches_function_identities(self) -> None:
        evidence_path = CONFIG / "evidence/game_open_semantic_memory_allocator.tsv"
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

    def test_allocator_state_has_image_qualified_extents(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        for image, expected in STATE.items():
            for va, shape in expected.items():
                row = identities[(image, va)]
                self.assertEqual((row.name, row.size, row.datatype), shape)
                self.assertEqual(row.owner, "memory")
        self.assertEqual(STATE["GAME.EXE"][0x800A0200][1], 16 * 4)
        self.assertEqual(STATE["OPEN.EXE"][0x80075858][1], 16 * 4)

    def test_malloc_wrappers_are_exact_release_25_vendored_members(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        by_key = {(row["image"], parse_int(row["va"])): row for row in rows}
        for key in (("GAME.EXE", 0x800502DC), ("OPEN.EXE", 0x80030050)):
            row = by_key[key]
            self.assertEqual(
                (row["name"], row["library"], row["module"]),
                ("malloc", "LIBAPI.LIB", "C51"),
            )
            self.assertEqual(row["confidence"], "exact-release25-complete")

        _fields, objects = read_tsv(
            CONFIG / "evidence/psyq_release_25_complete_objects.tsv"
        )
        object_keys = {
            (row["image"], parse_int(row["va"]), row["module"], row["name"])
            for row in objects
        }
        self.assertIn(("GAME.EXE", 0x800502DC, "C51", "malloc"), object_keys)
        self.assertIn(("OPEN.EXE", 0x80030050, "C51", "malloc"), object_keys)

    def test_all_body_relocations_and_direct_calls_are_reviewed(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        targets = {(image, va): name for image, va, name, _ret, _args in PAIRS}
        ranges = {
            "GAME.EXE": (0x8001AAB0, 0x8001ACF0),
            "OPEN.EXE": (0x80015DD4, 0x80016014),
        }
        body_rows = [
            row
            for row in rows
            if row["image"] in ranges
            and ranges[row["image"]][0]
            <= parse_int(row["site_va"])
            < ranges[row["image"]][1]
        ]
        self.assertEqual(len(body_rows), 58)
        self.assertTrue(all(row["status"] == "reviewed" for row in body_rows))

        calls = [
            row
            for row in rows
            if row["kind"] == "mips26"
            and (row["image"], parse_int(row["target_va"])) in targets
        ]
        self.assertEqual(len(calls), 57)
        for row in calls:
            key = row["image"], parse_int(row["target_va"])
            self.assertEqual(row["target_name"], targets[key])
            self.assertEqual(row["status"], "reviewed")


if __name__ == "__main__":
    unittest.main()
