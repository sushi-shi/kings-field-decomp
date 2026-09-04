from __future__ import annotations

import struct
import unittest

from scripts.kf.paths import LOCAL_CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.image import RetailImage


TMD_JUMPS = {
    0x8001C374: 0x8001C584,
    0x8001C3B8: 0x8001C3E8,
    0x8001C3EC: 0x8001C584,
    0x8001C438: 0x8001C584,
    0x8001C478: 0x8001C584,
    0x8001C4DC: 0x8001C584,
    0x8001C51C: 0x8001C584,
}


class GameParserRelocationTests(unittest.TestCase):
    def test_tmd_packet_jumps_are_reviewed_instruction_relocations(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        selected = {
            parse_int(row["site_va"]): row for row in rows
            if row["image"] == "GAME.EXE" and parse_int(row["site_va"]) in TMD_JUMPS
        }
        self.assertEqual(set(selected), set(TMD_JUMPS))
        for site, target in TMD_JUMPS.items():
            row = selected[site]
            self.assertEqual(parse_int(row["target_va"]), target)
            self.assertEqual(row["kind"], "mips26")
            self.assertEqual(row["channel"], "reachable-code")
            self.assertEqual(row["status"], "reviewed")
            self.assertEqual(row["confidence"], "manual-control-flow")
            self.assertEqual(row["opcode"], "j")

    @unittest.skipUnless(LOCAL_CONFIG.is_file(), "local retail image required")
    def test_tmd_raw_targets_and_owned_halfword_store_delay_slots(self) -> None:
        retail = RetailImage.load("GAME.EXE")
        for site, target in TMD_JUMPS.items():
            with self.subTest(site=hex(site)):
                jump, delay = struct.unpack("<II", retail.require(site, 8))
                self.assertEqual(jump >> 26, 2)  # j, not jal or a data word
                decoded = ((site + 4) & 0xF0000000) | ((jump & 0x03FFFFFF) << 2)
                self.assertEqual(decoded, target)
                self.assertEqual(delay >> 26, 0x29)  # sh is owned by the jump


if __name__ == "__main__":
    unittest.main()
