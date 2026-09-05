from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.parser_machine import CandidateLinkError, GameSymbols
from scripts.kf.paths import LOCAL_CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.index import index
from scripts.kf.sema.image import RetailImage


class GameWorldStateLayoutTests(unittest.TestCase):
    def test_pinned_layout_and_negative_size_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        psyq_include = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or psyq_include is None:
            self.skipTest("pinned compiler unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/game_map_runtime_layout.c"
        with TemporaryDirectory(prefix="kf-map-runtime-layout-") as directory:
            root = Path(directory)
            for expected in (0x2360, 0x235F):
                with self.subTest(expected=expected):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", psyq_include,
                         f"-DEXPECTED_MAP_RUNTIME_SIZE={expected}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if expected == 0x2360:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("map_runtime_size", result.stderr)

    def test_one_complete_owner_replaces_interior_globals(self) -> None:
        game = index("GAME.EXE")
        owner = game.datum(0x8009DB88)
        self.assertEqual((owner.name, owner.datatype, owner.size),
                         ("map_runtime_state", "KfMapRuntimeState", 0x2360))
        for address in (0x8009DDA8, 0x8009DDAC, 0x8009DDB0, 0x8009DDB2,
                        0x8009DDB4, 0x8009DDB8, 0x8009EAFC):
            with self.subTest(address=address):
                self.assertIsNone(game.datum(address))
                self.assertEqual(game.data_owner(address), owner)
        self.assertEqual(game.data_owner(0x8009F847), owner)
        self.assertEqual(game.data_owner(0x8009FEE7), owner)
        self.assertNotEqual(game.data_owner(0x8009FEE8), owner)
        symbols = GameSymbols.load()
        self.assertEqual(symbols.datum("map_runtime_state"), (0x8009DB88, 0x2360))
        for name in ("map_event_pool", "current_map_event", "map_world_state_base",
                     "map_variant_asset_buffer", "boss_defeat_complete"):
            with self.subTest(name=name), self.assertRaises(CandidateLinkError):
                symbols.datum(name)

    @unittest.skipUnless(LOCAL_CONFIG.is_file(), "local retail image required")
    def test_all_owner_relocations_preserve_decoded_retail_targets(self) -> None:
        retail = RetailImage.load("GAME.EXE")
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        selected = [row for row in rows if row["image"] == "GAME.EXE"
                    and 0x8009DB88 <= parse_int(row["target_va"]) < 0x8009FEE8]
        self.assertEqual(len(selected), 85)
        for row in selected:
            site = parse_int(row["site_va"])
            with self.subTest(site=hex(site)):
                self.assertEqual(row["target_name"], "map_runtime_state")
                self.assertEqual(row["kind"], "mips_hi16_lo16")
                self.assertEqual(row["status"], "reviewed")
                hi = struct.unpack("<I", retail.require(site, 4))[0]
                lo = struct.unpack(
                    "<I", retail.require(parse_int(row["paired_site_va"]), 4)
                )[0]
                self.assertEqual(decode_hi_lo_target(hi, lo), parse_int(row["target_va"]))

    @unittest.skipUnless(LOCAL_CONFIG.is_file(), "local retail image required")
    def test_retail_clear_extents_independently_bound_the_owner(self) -> None:
        retail = RetailImage.load("GAME.EXE")
        self.assertEqual(struct.unpack("<I", retail.require(0x80014730, 4))[0], 0x34062360)
        self.assertEqual(struct.unpack("<I", retail.require(0x8001535C, 4))[0], 0x34022133)
        for site, expected in ((0x80014720, 0x8009DB88), (0x80015354, 0x8009DDB4)):
            hi, lo = struct.unpack("<II", retail.require(site, 8))
            self.assertEqual(decode_hi_lo_target(hi, lo), expected)
        self.assertEqual(0x8009DB88 + 0x2360, 0x8009DDB4 + 0x2134)


if __name__ == "__main__":
    unittest.main()
