"""Tests for the source cleanliness ratchet board (scripts.kf.cleanliness)."""

from __future__ import annotations

import unittest
from dataclasses import replace
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock

from scripts.kf import cleanliness
from scripts.kf.inventory import DataIdentity


class StripSourceTest(unittest.TestCase):
    def test_blanks_comments_and_strings(self) -> None:
        code = 'int x; /* func_12345678 */ char *s = "DAT_800a0780"; // extern y\n'
        stripped = cleanliness.strip_source(code)
        self.assertNotIn("func_12345678", stripped)
        self.assertNotIn("DAT_800a0780", stripped)
        self.assertNotIn("extern y", stripped)
        self.assertIn("int x;", stripped)


class ExternClassificationTest(unittest.TestCase):
    def test_declarator_name_variants(self) -> None:
        self.assertEqual(cleanliness._extern_symbol("KfPlayerState player_state"),
                         "player_state")
        self.assertEqual(cleanliness._extern_symbol("u8 grid[100][100]"), "grid")
        self.assertEqual(cleanliness._extern_symbol("void sound_play(const S *s)"),
                         "sound_play")
        self.assertEqual(cleanliness._extern_symbol("POLY_FT4 *(*hook)(int)"), "hook")

    def test_game_vs_sdk_extern(self) -> None:
        game = {"player_state", "menu_present_frame", "item_stock"}
        self.assertTrue(cleanliness._is_game_extern("player_state", game))
        self.assertTrue(cleanliness._is_game_extern("item_stock", game))
        self.assertTrue(cleanliness._is_game_extern("DAT_80012345", game))
        self.assertTrue(cleanliness._is_game_extern("func_8001fdc8", game))
        self.assertFalse(cleanliness._is_game_extern("memset", game))
        self.assertFalse(cleanliness._is_game_extern("rand", game))

    def test_count_game_externs_ignores_sdk(self) -> None:
        game = {"player_state", "item_stock"}
        code = ("extern KfPlayerState player_state;\n"
                "extern s32 rand(void);\n"
                "extern u8 item_stock[3][80];\n"
                "extern u8 DAT_80012345[];\n"
                "extern void memset();\n")
        self.assertEqual(cleanliness._count_game_externs(code, game), 3)


class MetricRegexTest(unittest.TestCase):
    def test_address_derived_refs(self) -> None:
        code = "func_8001fdc8(); x = DAT_8009d040[i]; y = notFunc_1234;"
        self.assertEqual(len(cleanliness.FUNC_REF.findall(code)), 1)
        self.assertEqual(len(cleanliness.DAT_REF.findall(code)), 1)

    def test_pointer_cast_but_not_scalar(self) -> None:
        self.assertTrue(cleanliness.POINTER_CAST.search("(u8 *)&player_state"))
        self.assertTrue(cleanliness.POINTER_CAST.search("(KfMapEvent *)base"))
        self.assertIsNone(cleanliness.POINTER_CAST.search("(s16)angle"))

    def test_byte_array_view(self) -> None:
        self.assertTrue(cleanliness.BYTE_ARRAY_VIEW.search("(u8 *)&DAT_8009ddb4"))
        self.assertTrue(cleanliness.BYTE_ARRAY_VIEW.search("(u8 *)(base + 4)"))

    def test_cpp_local_aggregate(self) -> None:
        self.assertTrue(cleanliness.CPP_LOCAL_AGGREGATE.search("struct Local { int a; };"))
        self.assertTrue(cleanliness.CPP_LOCAL_AGGREGATE.search("union View {\n u32 w;"))


class GateTest(unittest.TestCase):
    def test_representation_counts_are_visible_without_rewarding_union_views(self) -> None:
        rows = [("GAME extern decls", 0), ("pointer casts", 42), ("byte-array views", 4)]
        with TemporaryDirectory() as td:
            with mock.patch.object(cleanliness, "BASELINE", Path(td) / "baseline.tsv"):
                cleanliness.save_baseline(rows)
                self.assertEqual(cleanliness.load_baseline(), {"GAME extern decls": 0})
                exposed = [("GAME extern decls", 0), ("pointer casts", 50), ("byte-array views", 8)]
                self.assertFalse(cleanliness.gate(exposed))
                lines = cleanliness.report_lines(exposed)
                self.assertTrue(all("[informational]" in line for line in lines[-2:]))
                self.assertIn("50", lines[-2])
                self.assertIn("8", lines[-1])
                self.assertTrue(cleanliness.gate([("GAME extern decls", 1), *exposed[1:]]))

    def test_inventory_discoveries_are_informational_and_not_saved_as_floors(self) -> None:
        rows = [("GAME extern decls", 0), ("raw DAT_ identities", 2704),
                ("unresolved data ownership", 100)]
        with TemporaryDirectory() as td:
            with mock.patch.object(cleanliness, "BASELINE", Path(td) / "baseline.tsv"):
                cleanliness.save_baseline(rows)
                self.assertEqual(cleanliness.load_baseline(), {"GAME extern decls": 0})
                grown = [("GAME extern decls", 0), ("raw DAT_ identities", 3000),
                         ("unresolved data ownership", 200)]
                self.assertFalse(cleanliness.gate(grown))
                inventory_lines = cleanliness.report_lines(grown)[-2:]
                self.assertTrue(all("[informational]" in line for line in inventory_lines))
                self.assertFalse(any("NO FLOOR" in line for line in inventory_lines))

    def test_seeded_passes_rise_fails_missing_floor_fails(self) -> None:
        rows = [("src func_ refs", 10), ("GAME extern decls", 5)]
        with TemporaryDirectory() as td:
            baseline = Path(td) / "cleanliness-baseline.tsv"
            with mock.patch.object(cleanliness, "BASELINE", baseline):
                # no floor yet: every ratcheted metric is a finding
                self.assertTrue(cleanliness.gate(rows))
                cleanliness.save_baseline(rows)
                # seeded at current values: green
                self.assertEqual(cleanliness.load_baseline(),
                                 {"src func_ refs": 10, "GAME extern decls": 5})
                self.assertFalse(cleanliness.gate(rows))
                # a rise fails; a fall passes
                self.assertTrue(cleanliness.gate([("src func_ refs", 11),
                                                  ("GAME extern decls", 5)]))
                self.assertFalse(cleanliness.gate([("src func_ refs", 3),
                                                   ("GAME extern decls", 5)]))

    def test_baseline_roundtrip_ignores_comments(self) -> None:
        with TemporaryDirectory() as td:
            baseline = Path(td) / "cleanliness-baseline.tsv"
            with mock.patch.object(cleanliness, "BASELINE", baseline):
                cleanliness.save_baseline([("GAME extern decls", 0)])
                self.assertTrue(baseline.read_text().startswith("#"))
                self.assertEqual(cleanliness.load_baseline(), {"GAME extern decls": 0})


class DataOwnershipTest(unittest.TestCase):
    def setUp(self) -> None:
        self.row = DataIdentity("GAME.EXE", 0x80012004, 4, "DAT_80012004", "unknown",
                                "load", "pointer", "", "address-only", "seed", "")
        self.owner = cleanliness.DataOwner("GAME.EXE", 0x80012000, 8, "load",
                                           "game.example", "src/game/example.c:RODATA")

    def test_table_element_is_owned_only_when_fully_covered_in_the_same_image(self) -> None:
        other_image = replace(self.row, image="OPEN.EXE")
        crosses_end = replace(self.row, size=8)
        at_end = replace(self.row, va=0x80012008)
        other_storage = replace(self.row, storage="bss")
        result = cleanliness.classify_data_ownership(
            [self.row, other_image, crosses_end, at_end, other_storage], [self.owner])
        self.assertEqual([owner for _row, owner in result], [self.owner, None, None, None, None])

    def test_renaming_or_pointing_at_sdk_code_does_not_establish_an_owner(self) -> None:
        named = replace(self.row, name="callback", confidence="supported")
        sdk_target = replace(self.row, note="pointer targets an SDK function")
        self.assertTrue(all(owner is None for _row, owner in
                            cleanliness.classify_data_ownership([named, sdk_target], [])))

    def test_supported_owner_requires_evidence_and_candidate_is_not_promoted(self) -> None:
        known = replace(self.row, owner="libgpu_sys", confidence="supported", evidence="SDK member")
        candidate = replace(known, confidence="candidate")
        no_evidence = replace(known, evidence="")
        for row in [candidate, no_evidence]:
            self.assertIsNone(cleanliness.classify_data_ownership([row], [])[0][1])
        self.assertEqual(cleanliness.classify_data_ownership([known], [])[0][1].owner, "libgpu_sys")


class LiveCountTest(unittest.TestCase):
    def test_source_has_no_local_extern_declarations(self) -> None:
        self.assertEqual(cleanliness.source_extern_sites(), [])

    def test_committed_baseline_matches_live_counts(self) -> None:
        # the seeded floors must equal the live counts, so day-one is green.
        if not cleanliness.BASELINE.is_file():
            self.skipTest("no committed cleanliness baseline")
        self.assertFalse(cleanliness.gate(),
                         "committed cleanliness floors are below the live counts")

    def test_every_metric_is_either_informational_or_ratcheted_and_floored(self) -> None:
        labels = {label for label, _n in cleanliness.count()}
        self.assertEqual(labels, cleanliness.RATCHET | cleanliness.INFORMATIONAL)
        self.assertFalse(cleanliness.RATCHET & cleanliness.INFORMATIONAL)
        base = cleanliness.load_baseline()
        self.assertTrue(cleanliness.RATCHET.issubset(set(base)),
                        "a ratcheted metric has no committed floor")


if __name__ == "__main__":
    unittest.main()
