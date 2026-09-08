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
        unit = load_manifest().by_identity()["OPEN.EXE", START]
        self.assertEqual(unit.image, "OPEN.EXE")
        function = next(fn for fn in unit.functions if fn.va == START)
        self.assertEqual(function.body_size, 0x798)
        self.assertEqual(unit.rodata, (0x80012000, 32))
        catalog = load_catalog(RETAIL_CONFIG)
        self.assertEqual(catalog.function_starts["OPEN.EXE"][START].body_size, 0x798)
        self.assertNotIn(0x80015590, catalog.function_starts["OPEN.EXE"])
        identities = load_function_identities(RETAIL_CONFIG)
        identity = identities["OPEN.EXE", START]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("opening_ending_scroll_run", "void", ""))
        self.assertEqual([(item.va, item.size) for item in unit.data
                          if item.symbol.startswith("opening_ending_scroll_")], [
            (0x80035820, 84), (0x80035888, 72), (0x80037290, 16),
            (0x800372A0, 4), (0x800372A4, 4), (0x800372A8, 4),
            (0x800372AC, 4), (0x800372B0, 4), (0x800372B4, 4), (0x800372B8, 8),
        ])
        data = load_data_identities(RETAIL_CONFIG)
        for va in range(0x80012000, 0x80012020, 4):
            self.assertNotIn(("OPEN.EXE", va), data)
        for va in (0x8003588A, 0x80035892, 0x800358CA):
            self.assertNotIn(("OPEN.EXE", va), data)
        self.assertEqual(data["OPEN.EXE", 0x80035888].datatype, "KfScreenRect[9]")
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
        unit = load_manifest().by_identity()["OPEN.EXE", START]
        path = BUILD / "objdiff/open/base" / unit.object_name
        if not path.is_file():
            self.skipTest("compiled ending-scroll object is required")
        obj = _load_object(path)
        function = obj.named_symbol("opening_ending_scroll_run")
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
            if (reloc.section != ".text" or not
                    function.value <= reloc.offset < function.value + function.size):
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

    def test_lighting_completion_values_and_shared_retail_join(self) -> None:
        data = self.retail_bytes()
        offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(START)
        retail = list(struct.unpack_from(f"<{(END - START) // 4}I", data, offset))
        retail_calls = [(va - START) // 4 for va in (0x800151F8, 0x80015220)]

        def completion(words: list[int], call: int, phase: int,
                       matrix_target: int, increment: int,
                       base: int = START) -> tuple[int, int]:
            # This is a bounded instruction-pattern check, not an interpreter.
            self.assertEqual(words[call - 6], 0x00023403)  # signed short -> a2
            self.assertEqual(words[call - 5], 0x28C21001)  # blend < 4097
            branch = words[call - 4]
            self.assertEqual(branch >> 16, 0x1040)        # beqz v0
            phase_constant = words[call - 3]
            self.assertEqual(phase_constant >> 21, 0x1A0)  # ori rt,zero,phase
            self.assertEqual(phase_constant & 0xFFFF, phase)
            phase_register = phase_constant >> 16 & 31
            self.assertEqual(decode_hi_lo_target(words[call - 2], words[call - 1]),
                             matrix_target)
            self.assertEqual(words[call + 1], 0x24850000 | (64 if phase == 1 else 32))
            self.assertEqual(words[call + 2] >> 26, 2)    # jump after interpolation
            self.assertEqual(words[call + 3], 0x26940000 | increment)

            displacement = struct.unpack("<h", struct.pack("<H", branch & 0xFFFF))[0]
            target = call - 3 + displacement
            if words[target] >> 26 == 2:
                # The non-exact probe has a phase-1 store in a jump delay slot.
                store = target + 1
                reset = ((words[target] & 0x03FFFFFF) * 4 - (base & 0x0FFFFFFF)) // 4
            else:
                store, reset = target, target + 1
            self.assertEqual(words[store], 0xA7A000B0 | phase_register << 16)
            self.assertEqual(words[reset], 0x0000A021)    # lighting_blend = 0
            passing_target = ((words[call + 2] & 0x03FFFFFF) * 4
                              - (base & 0x0FFFFFFF)) // 4
            self.assertEqual(passing_target, reset + 1)  # interpolation skips reset
            return store, reset

        joins = [completion(retail, call, phase, matrix_target, increment)
                 for call, phase, matrix_target, increment in zip(
                     retail_calls, (1, 2), (0x80035964, 0x800359A4), (64, 3), strict=True)]
        self.assertEqual(joins, [((0x80015230 - START) // 4,
                                 (0x80015234 - START) // 4)] * 2)
        wrong_phase = retail.copy()
        wrong_phase[retail_calls[0] - 3] ^= 3
        with self.assertRaises(AssertionError):
            completion(wrong_phase, retail_calls[0], 1, 0x80035964, 64)
        wrong_reset = retail.copy()
        wrong_reset[joins[0][1]] ^= 0x800
        with self.assertRaises(AssertionError):
            completion(wrong_reset, retail_calls[0], 1, 0x80035964, 64)

        unit = load_manifest().by_identity()["OPEN.EXE", START]
        path = BUILD / "objdiff/open/base" / unit.object_name
        if not path.is_file():
            self.skipTest("compiled ending-scroll object is required")
        obj = _load_object(path)
        function = obj.named_symbol("opening_ending_scroll_run")
        compiled = list(struct.unpack(f"<{len(obj.sections['.text']) // 4}I",
                                      obj.sections[".text"]))
        calls = [reloc.offset // 4 for reloc in obj.relocations
                 if reloc.section == ".text" and reloc.kind == 4
                 and function.value <= reloc.offset < function.value + function.size
                 and obj.symbol(reloc.symbol_index).name == "lighting_set_color_matrix"]
        self.assertEqual(len(calls), 2)
        for call, phase, matrix_offset, increment in zip(
                calls, (1, 2), (32, 96), (64, 3), strict=True):
            # Relocatable internal jumps encode .text offsets, not linked VAs.
            completion(compiled, call, phase, matrix_offset, increment, base=0)
