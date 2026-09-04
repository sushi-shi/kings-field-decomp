"""Tests for the source cleanliness ratchet board (scripts.kf.cleanliness)."""

from __future__ import annotations

import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock

from scripts.kf import cleanliness
from scripts.kf.paths import REPO


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
        game = {"player_state", "menu_present_frame"}
        self.assertTrue(cleanliness._is_game_extern("player_state", game))
        self.assertTrue(cleanliness._is_game_extern("DAT_800652a8", game))
        self.assertTrue(cleanliness._is_game_extern("func_8001fdc8", game))
        self.assertFalse(cleanliness._is_game_extern("memset", game))
        self.assertFalse(cleanliness._is_game_extern("rand", game))

    def test_count_game_externs_ignores_sdk(self) -> None:
        game = {"player_state"}
        code = ("extern KfPlayerState player_state;\n"
                "extern s32 rand(void);\n"
                "extern u8 DAT_800652a8[240];\n"
                "extern void memset();\n")
        self.assertEqual(cleanliness._count_game_externs(code, game), 2)


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

    def test_source_has_no_compile_time_layout_assertions(self) -> None:
        forbidden = (
            "_Static_assert",
            "static_assert",
            "STATIC_ASSERT",
            "C_ASSERT",
            "BUILD_BUG_ON",
            "KF_OFFSET_OF",
            "offset_is_",
            "size_is_",
        )
        for root in (REPO / "src", REPO / "include"):
            for path in (*root.rglob("*.c"), *root.rglob("*.h")):
                source = path.read_text(encoding="utf-8", errors="replace")
                for spelling in forbidden:
                    self.assertNotIn(spelling, source, f"{path}: {spelling}")


class GateTest(unittest.TestCase):
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
                cleanliness.save_baseline([("pointer casts", 42)])
                self.assertTrue(baseline.read_text().startswith("#"))
                self.assertEqual(cleanliness.load_baseline(), {"pointer casts": 42})


class LiveCountTest(unittest.TestCase):
    def test_source_has_no_local_extern_declarations(self) -> None:
        self.assertEqual(cleanliness.source_extern_sites(), [])

    def test_committed_baseline_matches_live_counts(self) -> None:
        # the seeded floors must equal the live counts, so day-one is green.
        if not cleanliness.BASELINE.is_file():
            self.skipTest("no committed cleanliness baseline")
        self.assertFalse(cleanliness.gate(),
                         "committed cleanliness floors are below the live counts")

    def test_every_metric_is_ratcheted_and_floored(self) -> None:
        labels = {label for label, _n in cleanliness.count()}
        self.assertEqual(labels, cleanliness.RATCHET)
        base = cleanliness.load_baseline()
        self.assertTrue(labels.issubset(set(base)),
                        "a ratcheted metric has no committed floor")


if __name__ == "__main__":
    unittest.main()
