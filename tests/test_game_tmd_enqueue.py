from __future__ import annotations

import struct
import unittest

from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.relocations import decode_mips26_target
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv, validate_config
from scripts.kf.sema.mips import branch_target


START = 0x8001C7F8
SIZE = 0xF38
SWITCH = 0x8001222C
ADVANCE = 0x8001D6D4
CASES = {
    0x20: 0x8001D204, 0x22: 0x8001D45C, 0x24: 0x8001C8B8,
    0x28: 0x8001CA44, 0x2A: 0x8001D5A8, 0x2C: 0x8001D0F8,
    0x30: 0x8001CAC8, 0x32: 0x8001CFCC, 0x34: 0x8001CCF8,
    0x38: 0x8001CBF0, 0x3A: 0x8001D344, 0x3C: 0x8001CE68,
}
JUMPS = {
    0x8001CA3C: 0x8001D6B0, 0x8001CAC0: 0x8001D630,
    0x8001CBE8: 0x8001D6B0, 0x8001CCF0: 0x8001D68C,
    0x8001CE60: 0x8001D6B0, 0x8001CFC4: 0x8001D68C,
    0x8001D0F0: 0x8001D6B0, 0x8001D1FC: 0x8001D664,
    0x8001D33C: 0x8001D6B0, 0x8001D454: 0x8001D68C,
    0x8001D5A0: 0x8001D6B0,
}
SET_POLY_OFFSETS = [
    0x134, 0x2C0, 0x344, 0x46C, 0x574, 0x6E4,
    0x848, 0x974, 0xA80, 0xBC0, 0xCD8, 0xE24,
]


class GameTmdEnqueueTests(unittest.TestCase):
    def retail_bytes(self) -> bytes:
        try:
            return (configured_retail_dir() / "GAME.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")

    def retail_words(self) -> list[int]:
        offset = IMAGE_LAYOUTS["GAME.EXE"].file_offset(START)
        return list(struct.unpack_from(f"<{SIZE // 4}I", self.retail_bytes(), offset))

    def test_switch_is_unit_rodata_not_standalone_globals(self) -> None:
        unit = load_manifest().by_name()["game.render_enqueue"]
        self.assertEqual(unit.rodata, (SWITCH, 116))
        self.assertEqual((unit.functions[0].va, unit.functions[0].body_size), (START, SIZE))
        identities = load_data_identities(RETAIL_CONFIG)
        _, data = read_tsv(RETAIL_CONFIG / "data.tsv")
        table = [row for row in data if row["image"] == "GAME.EXE"
                 and SWITCH <= parse_int(row["va"]) < SWITCH + 116]
        self.assertEqual(len(table), 1)
        self.assertEqual((parse_int(table[0]["va"]), parse_int(table[0]["size"]),
                          table[0]["kind"]), (SWITCH, 116, "data"))
        for va in range(SWITCH, SWITCH + 116, 4):
            self.assertNotIn(("GAME.EXE", va), identities)
        validate_config(RETAIL_CONFIG)

    def test_reviewed_jumps_and_switch_decode_to_literal_targets(self) -> None:
        blob = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", blob, IMAGE_LAYOUTS["GAME.EXE"].file_offset(va))[0]

        expected = dict(JUMPS)
        expected.update({SWITCH + (mode - 0x20) * 4: CASES.get(mode, ADVANCE)
                         for mode in range(0x20, 0x3D)})
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        reviewed = [row for row in rows if row["image"] == "GAME.EXE"
                    and "manual:game_tmd_enqueue" in row["provenance"]]
        self.assertEqual(len(reviewed), 40)
        self.assertEqual({parse_int(row["site_va"]) for row in reviewed}, set(expected))
        for row in reviewed:
            site = parse_int(row["site_va"])
            self.assertEqual(row["status"], "reviewed")
            self.assertEqual(parse_int(row["target_va"]), expected[site])
            if site in JUMPS:
                self.assertEqual(row["kind"], "mips26")
                self.assertEqual(word(site) >> 26, 2)
                actual = decode_mips26_target(site, word(site))
            else:
                self.assertEqual(row["kind"], "mips32_candidate")
                actual = word(site)
            self.assertEqual(actual, expected[site], hex(site))
        # Mode extraction, unsigned bound 28, word indexing, load delay and jr delay.
        for offset, instruction in {
            0x88: 0x00081602, 0x8C: 0x2443FFE0, 0x90: 0x3402001C,
            0x94: 0x0043102B, 0xA0: 0x00031080, 0xB0: 0x8C220000,
            0xB4: 0, 0xB8: 0x00400008, 0xBC: 0,
        }.items():
            self.assertEqual(word(START + offset), instruction, hex(offset))

    def assert_overflow_returns(self, words: list[int], calls: list[int]) -> None:
        # Specific allocation guards, not a general CFG-equivalence claim.
        self.assertEqual(len(calls), 12)
        epilogues = [i for i, word in enumerate(words) if word == 0x8FBF0054]
        self.assertEqual(len(epilogues), 1)
        epilogue = epilogues[0]
        self.assertEqual(words[epilogue:], [
            0x8FBF0054, 0x8FBE0050, 0x8FB7004C, 0x8FB60048,
            0x8FB50044, 0x8FB40040, 0x8FB3003C, 0x8FB20038,
            0x8FB10034, 0x8FB00030, 0x03E00008, 0x27BD0058,
        ])
        for call in calls:
            index = call // 4
            self.assertEqual(words[index - 6:index - 2],
                             [0x8C430008, 0x8C420004, 0, 0x0043102B])
            guard = words[index - 2]
            self.assertEqual(guard >> 16, 0x1440)  # bnez v0 after unsigned end < cursor
            self.assertEqual(branch_target(call - 8, guard), epilogue * 4, hex(call))

    def test_retail_and_compiled_overflows_return_from_all_twelve_modes(self) -> None:
        self.assert_overflow_returns(self.retail_words(), SET_POLY_OFFSETS)
        path = BUILD / "objdiff/game/base/8001c7f8_render_enqueue.o"
        if not path.is_file():
            self.skipTest("compiled GAME TMD renderer is required")
        obj = _load_object(path)
        function = obj.named_symbol("render_enqueue_tmd")
        text = obj.sections[".text"][function.value:function.value + function.size]
        calls = [reloc.offset - function.value for reloc in obj.relocations
                 if reloc.section == ".text" and reloc.kind == 4
                 and function.value <= reloc.offset < function.value + function.size
                 and obj.symbol(reloc.symbol_index).name.startswith("SetPoly")]
        self.assert_overflow_returns(list(struct.unpack(f"<{len(text) // 4}I", text)), calls)

    def test_packet_advance_cannot_replace_an_overflow_return(self) -> None:
        words = self.retail_words()
        for call in SET_POLY_OFFSETS:
            with self.subTest(packet_setup_offset=hex(call)):
                changed = words.copy()
                guard = call - 8
                displacement = ((ADVANCE - START) - (guard + 4)) // 4
                changed[guard // 4] = (changed[guard // 4] & 0xFFFF0000) | displacement
                with self.assertRaises(AssertionError):
                    self.assert_overflow_returns(changed, SET_POLY_OFFSETS)
