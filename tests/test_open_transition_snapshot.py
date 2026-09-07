from __future__ import annotations

import struct
import unittest

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


START = 0x80014608
SIZE = 0x1FC
SYMBOL = "opening_entity_transition"


class OpenTransitionSnapshotTests(unittest.TestCase):
    def retail_bytes(self) -> bytes:
        try:
            return (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")

    def test_reviewed_boundary_calls_and_relocation_targets(self) -> None:
        unit = load_manifest().by_identity()["OPEN.EXE", START]
        self.assertEqual(unit.image, "OPEN.EXE")
        function = next(fn for fn in unit.functions if fn.va == START)
        self.assertEqual(function.body_size, SIZE)
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and START <= parse_int(row["site_va"]) < START + SIZE]
        self.assertEqual(len(body), 8)
        self.assertEqual({row["status"] for row in body}, {"reviewed"})
        self.assertEqual([row["target_name"] for row in body if row["opcode"] == "jal"],
                         ["opening_render_frame", "VSync"])
        blob = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", blob, IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va))[0]

        for row in body:
            site = parse_int(row["site_va"])
            if row["kind"] == "mips_hi16_lo16":
                actual = decode_hi_lo_target(word(site), word(parse_int(row["paired_site_va"])))
            else:
                actual = decode_mips26_target(site, word(site))
            self.assertEqual(actual, parse_int(row["target_va"]))
        self.assertEqual([parse_int(row["target_va"]) for row in body
                          if row["kind"] == "mips_hi16_lo16"],
                         [0x800498F8, 0x80049912, 0x800498F8])
        for site, slot in ((0x80014AA8, 24), (0x80014B0C, 24),
                           (0x80014BCC, 16), (0x80014C18, 16)):
            self.assertEqual(decode_mips26_target(site, word(site)), START)
            self.assertEqual(word(site + 4), 0x27A50000 | slot)  # a1 = sp + VECTOR offset

    def test_retail_snapshot_is_three_real_word_stores_with_load_delays(self) -> None:
        blob = self.retail_bytes()
        offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(START)
        words = struct.unpack_from(f"<{SIZE // 4}I", blob, offset)
        # These loads use the original input pointer in t1, not entity storage.
        # One independent instruction separates each load from its stack store.
        for load, store, lane, slot in ((0x7C, 0x84, 0, 16),
                                        (0x88, 0x90, 8, 24),
                                        (0x94, 0x9C, 4, 20)):
            self.assertEqual(words[load // 4], 0x8D220000 | lane)
            self.assertEqual(words[store // 4], 0xAFA20000 | slot)
            self.assertEqual(store - load, 8)
        self.assertEqual([words[index] for index in (0x80 // 4, 0x8C // 4, 0x98 // 4)],
                         [0x340A1000, 0x240CFFFF, 0x24E60022])
        # The entity's full VECTOR copy still reloads all four words from t1.
        self.assertEqual(words[0xA4 // 4:0xB4 // 4],
                         (0x8D220000, 0x8D230004, 0x8D240008, 0x8D25000C))
        self.assertEqual(words[-2:], (0x03E00008, 0x27BD0038))

    def test_compiled_body_matches_retail(self) -> None:
        unit = load_manifest().by_identity()["OPEN.EXE", START]
        target_path = BUILD / "delink/open/modules" / unit.object_name
        source_path = BUILD / "objdiff/open/base" / unit.object_name
        if not target_path.is_file() or not source_path.is_file():
            self.skipTest("delinked and compiled OPEN transition objects are required")
        target, source = _load_object(target_path), _load_object(source_path)
        target_symbol, source_symbol = target.named_symbol(SYMBOL), source.named_symbol(SYMBOL)
        self.assertEqual((target_symbol.size, source_symbol.size), (SIZE, SIZE))
        retail = struct.unpack_from(f"<{SIZE // 4}I", target.sections[".text"],
                                    target_symbol.value)
        compiled = struct.unpack_from(f"<{SIZE // 4}I", source.sections[".text"],
                                      source_symbol.value)
        self.assertEqual(compiled, retail)
        relocations = []
        for obj in (target, source):
            symbol = obj.named_symbol(SYMBOL)
            relocations.append([
                (rel.offset - symbol.value, rel.kind, obj.symbol(rel.symbol_index).name,
                 obj.symbol(rel.symbol_index).section, obj.symbol(rel.symbol_index).value)
                for rel in obj.relocations if rel.section == ".text"
                and symbol.value <= rel.offset < symbol.value + SIZE
            ])
        self.assertEqual(len(relocations[0]), 11)
        self.assertEqual(relocations[0], relocations[1])
