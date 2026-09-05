from __future__ import annotations

import struct
import unittest

from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


START = 0x80014E28
END = 0x800155C0


def reviewed_body() -> list[dict[str, str]]:
    _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
    return [row for row in rows if row["image"] == "OPEN.EXE"
            and START <= parse_int(row["site_va"]) < END]


class OpenEndingScrollTests(unittest.TestCase):
    def retail_bytes(self) -> bytes:
        try:
            return (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")

    def test_complete_boundary_rodata_and_nonoverlapping_data_owners(self) -> None:
        unit = load_manifest().by_name()["open.opening_ending_scroll"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([(fn.va, fn.body_size) for fn in unit.functions], [(START, 0x798)])
        self.assertEqual(unit.rodata, (0x80012000, 32))
        catalog = load_catalog(RETAIL_CONFIG)
        self.assertEqual(catalog.function_starts["OPEN.EXE"][START].body_size, 0x798)
        self.assertNotIn(0x80015590, catalog.function_starts["OPEN.EXE"])
        identities = load_function_identities(RETAIL_CONFIG)
        identity = identities["OPEN.EXE", START]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("opening_ending_scroll_run", "void", ""))
        self.assertEqual([(item.va, item.size) for item in unit.data], [
            (0x80035820, 84), (0x80035888, 72), (0x80037290, 16),
            (0x800372A0, 4), (0x800372A4, 4), (0x800372A8, 4),
            (0x800372AC, 4), (0x800372B0, 4), (0x800372B4, 4), (0x800372B8, 8),
        ])
        data = load_data_identities(RETAIL_CONFIG)
        for va in range(0x80012000, 0x80012020, 4):
            self.assertNotIn(("OPEN.EXE", va), data)
        for va in (0x8003588A, 0x80035892, 0x800358CA):
            self.assertNotIn(("OPEN.EXE", va), data)
        self.assertEqual(data["OPEN.EXE", 0x80035888].datatype, "u16[9][4]")
        self.assertEqual(data["OPEN.EXE", 0x800372A0].datatype, "CVECTOR")

    def test_reviewed_calls_and_raw_relocation_targets(self) -> None:
        body = reviewed_body()
        self.assertEqual(len(body), 79)
        self.assertEqual({row["status"] for row in body}, {"reviewed"})
        self.assertTrue(all("manual:open_semantic_ending_scroll" in row["provenance"]
                            for row in body))
        calls = [row["target_name"] for row in body if row["opcode"] == "jal"]
        self.assertEqual(calls, ["opening_resources_load_ending_entities"]
                         + ["GetTPage", "GetClut"] * 9 + [
            "opening_entity_find_by_object_id", "opening_entity_find_by_object_id",
            "opening_camera_path_begin", "SetFogNear", "SetBackColor", "SetFarColor",
            "lighting_set_color_matrix", "lighting_set_color_matrix", "SsSetMVol",
            "opening_resources_load_ending_sequence", "opening_camera_path_step",
            "render_set_view_transform", "display_begin_frame", "SetGeomScreen",
            "opening_render_entities", "color_lerp_cvector", "color_lerp_cvector",
            "sprite_add_g4", "sprite_add_f4", "sprite_add_ft4", "display_present_frame",
        ])
        data = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", data, IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va))[0]

        for row in body:
            site, target = parse_int(row["site_va"]), parse_int(row["target_va"])
            if row["kind"] == "mips_hi16_lo16":
                actual = decode_hi_lo_target(word(site), word(parse_int(row["paired_site_va"])))
            else:
                actual = decode_mips26_target(site, word(site))
            self.assertEqual(actual, target)
        self.assertEqual(decode_mips26_target(0x80015890, word(0x80015890)), START)

    def test_static_section_referents_resolve_to_the_same_ordered_owners(self) -> None:
        path = BUILD / "objdiff/open/base/80014e28_opening_ending_scroll.o"
        if not path.is_file():
            self.skipTest("compiled ending-scroll object is required")
        unit = load_manifest().by_name()["open.opening_ending_scroll"]
        obj = _load_object(path)
        data = load_data_identities(RETAIL_CONFIG)
        external_data = {item.name: item.va for (image, _), item in data.items()
                         if image == "OPEN.EXE"}
        spans = []
        for item in unit.data:
            symbol = obj.named_symbol(item.symbol)
            spans.append((symbol.value, symbol.value + item.size, item.va))
        text = obj.sections[".text"]
        pending = []
        actual = []
        for reloc in obj.relocations:
            if reloc.section != ".text":
                continue
            if reloc.kind == 5:
                pending.append(reloc)
            elif reloc.kind == 6:
                self.assertEqual(len(pending), 1)
                high = pending.pop()
                self.assertEqual(high.symbol_index, reloc.symbol_index)
                addend = decode_hi_lo_target(
                    struct.unpack_from("<I", text, high.offset)[0],
                    struct.unpack_from("<I", text, reloc.offset)[0])
                symbol = obj.symbol(reloc.symbol_index)
                offset = symbol.value + addend
                if symbol.section == ".data":
                    span = next(span for span in spans if span[0] <= offset < span[1])
                    target = span[2] + offset - span[0]
                elif symbol.section == ".rodata":
                    target = unit.rodata[0] + offset
                else:
                    target = external_data[symbol.name] + addend
                actual.append(target)
        self.assertFalse(pending)
        self.assertEqual(actual, [parse_int(row["target_va"]) for row in reviewed_body()
                                  if row["kind"] == "mips_hi16_lo16"])

    def test_retail_panel_extent_and_unreachable_epilogue(self) -> None:
        data = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", data, IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va))[0]

        # Every saved register has the matching restore at exactly the same frame slot.
        saves = [word(START + offset) for offset in range(4, 44, 4)]
        restores = [word(0x80015590 + offset) for offset in range(0, 40, 4)]
        self.assertEqual([instruction >> 26 for instruction in saves], [43] * 10)
        self.assertEqual([instruction >> 26 for instruction in restores], [35] * 10)
        self.assertEqual([instruction & 0x03FFFFFF for instruction in saves],
                         [instruction & 0x03FFFFFF for instruction in restores])
        wrong_restore = restores.copy()
        wrong_restore[0] ^= 4
        self.assertNotEqual([instruction & 0x03FFFFFF for instruction in saves],
                            [instruction & 0x03FFFFFF for instruction in wrong_restore])
        self.assertEqual(word(0x800155B8), 0x03E00008)
        self.assertEqual(word(0x800155BC), 0x27BD0108)
        self.assertEqual(decode_mips26_target(0x80015588, word(0x80015588)), 0x800151B8)
        offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(0x80035888)
        panels = tuple(struct.iter_unpack("<4H", data[offset:offset + 72]))
        self.assertEqual([row[1] for row in panels],
                         [256, 512, 768, 1024, 1261, 1490, 1746, 2002, 2258])
        self.assertTrue(all((row[0], row[2], row[3]) == (32, 255, 254) for row in panels))
        self.assertEqual(0x80035888 + 8 * 8 + 2, 0x800358CA)
        self.assertEqual(word(0x800154B0) >> 26, 33)  # signed last-panel Y load
        self.assertEqual(word(0x80015480) >> 26, 6)   # signed scrolling flag test
        self.assertEqual(word(0x800154F8) & 0xFFFF, 0xFFFF)
        self.assertEqual(word(0x800154FC) & 0xFFFF, 495)
