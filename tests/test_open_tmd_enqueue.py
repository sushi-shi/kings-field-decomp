from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from collections import Counter
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


START = 0x8001764C
END = 0x80018344
SWITCH = 0x800121C0
CASES = {
    0x20: 0x80017ED8, 0x22: 0x800180D0, 0x24: 0x8001770C,
    0x28: 0x80017864, 0x2A: 0x800181E8, 0x2C: 0x80017DF8,
    0x30: 0x800178BC, 0x32: 0x80017D00, 0x34: 0x80017A8C,
    0x38: 0x800179B0, 0x3A: 0x80017FE4, 0x3C: 0x80017BC8,
}


class OpenTmdEnqueueTests(unittest.TestCase):
    def retail_bytes(self) -> bytes:
        try:
            return (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")

    def test_packed_packet_views_preserve_all_sdk_layouts(self) -> None:
        cpp, cc1 = shutil.which("cpppsx-257"), shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and SDK headers are required")
        fixture = REPO / "tests/fixtures/open_tmd_enqueue_layout.c"
        with TemporaryDirectory(prefix="kf-tmd-enqueue-layout-") as directory:
            root = Path(directory)
            for offset in (16, 12):
                with self.subTest(ft3_x1=offset):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", sdk, f"-DEXPECTED_FT3_X1={offset}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")], capture_output=True, text=True,
                    )
                    if offset == 16:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("ft3_x1", result.stderr)

    def test_identity_switch_and_complete_color_owner(self) -> None:
        unit = load_manifest().by_name()["open.render_tmd"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([(f.va, f.body_size) for f in unit.functions], [(START, 0xCF8)])
        self.assertEqual(unit.rodata, (SWITCH, 116))
        self.assertEqual([(d.va, d.size) for d in unit.data], [(0x800372F0, 4)])
        identity = load_function_identities(RETAIL_CONFIG)["OPEN.EXE", START]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("render_enqueue_tmd", "void", "u16 object_index;s16 depth_bias"))
        data = load_data_identities(RETAIL_CONFIG)
        for va in range(SWITCH, SWITCH + 116, 4):
            self.assertNotIn(("OPEN.EXE", va), data)
        self.assertNotIn(("OPEN.EXE", 0x800372F3), data)
        color = data["OPEN.EXE", 0x800372F0]
        self.assertEqual((color.name, color.datatype, color.size, color.storage),
                         ("tmd_textured_primitive_color", "CVECTOR", 4, "load"))
        _, vendors = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        self.assertFalse(any(row["image"] == "OPEN.EXE" and parse_int(row["va"]) == START
                             for row in vendors))

    def test_all_reviewed_relocations_decode_to_literal_targets(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        reviewed = [row for row in rows if row["image"] == "OPEN.EXE" and (
            START <= parse_int(row["site_va"]) < END
            or SWITCH <= parse_int(row["site_va"]) < SWITCH + 116
            or parse_int(row["site_va"]) in (0x80019050, 0x800190D0))]
        self.assertEqual(len(reviewed), 110)
        self.assertEqual({row["status"] for row in reviewed}, {"reviewed"})
        self.assertTrue(all("manual:open_semantic_tmd_enqueue" in row["provenance"]
                            for row in reviewed))
        blob = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", blob, IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va))[0]

        for row in reviewed:
            site, expected = parse_int(row["site_va"]), parse_int(row["target_va"])
            if row["kind"] == "mips_hi16_lo16":
                actual = decode_hi_lo_target(word(site), word(parse_int(row["paired_site_va"])))
            elif row["kind"] == "mips26":
                actual = decode_mips26_target(site, word(site))
            else:
                actual = word(site)
            self.assertEqual(actual, expected)
        body = [row for row in reviewed if START <= parse_int(row["site_va"]) < END]
        self.assertEqual(len(body), 79)
        self.assertEqual(sum(row["opcode"] == "jal" for row in body), 57)
        self.assertEqual(sum(row["opcode"] == "j" for row in body), 11)
        pairs = [row for row in body if row["kind"] == "mips_hi16_lo16"]
        self.assertEqual([parse_int(row["target_va"]) for row in pairs],
                         [0x80069B60, SWITCH] + [0x800372F3] * 9)
        # These instructions derive real addresses without additional relocations.
        self.assertEqual(word(0x800176C0), 0x24B60020)
        self.assertEqual(word(0x800182D8), 0x8EC2FEEC)
        self.assertEqual(0x80069B60 + 32, 0x80069B80)
        self.assertEqual(0x80069B80 - 276, 0x80069A6C)

    def test_switch_value_chain_mode_allocations_and_signed_tail(self) -> None:
        blob = self.retail_bytes()

        def word(va: int) -> int:
            return struct.unpack_from("<I", blob, IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va))[0]

        targets = [word(SWITCH + index * 4) for index in range(29)]
        self.assertEqual(targets, [CASES.get(mode, 0x800182E8) for mode in range(0x20, 0x3D)])
        self.assertEqual(len(set(targets)), 13)
        # Mask-free mode extraction, unsigned bound 28, four-byte indexing, load delay, jr.
        for va, expected in {
            0x800176D4: 0x00081602, 0x800176D8: 0x2443FFE0,
            0x800176DC: 0x3402001C, 0x800176E0: 0x0043102B,
            0x800176EC: 0x00031080, 0x800176FC: 0x8C220000,
            0x80017700: 0, 0x80017704: 0x00400008, 0x80017708: 0,
            0x800182C0: 0x00021103, 0x800182C8: 0x28820005,
            0x800182D4: 0x30843FFF, 0x800182F4: 0x304203FC,
            0x8001833C: 0x03E00008, 0x80018340: 0x27BD0060,
        }.items():
            self.assertEqual(word(va), expected, hex(va))
        # All twelve paths allocate after positive clipping, with authentic packet sizes.
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        calls = {parse_int(row["site_va"]): row["target_name"] for row in rows
                 if row["image"] == "OPEN.EXE" and row["opcode"] == "jal"
                 and START <= parse_int(row["site_va"]) < END}
        starts = sorted(CASES.values())
        sizes = [32, 24, 28, 36, 40, 52, 28, 40, 20, 36, 20, 24]
        for start, end, size in zip(starts, starts[1:] + [END], sizes):
            sites = [(va, name) for va, name in calls.items() if start <= va < end]
            self.assertEqual([name for _, name in sites[:2]],
                             ["NormalClip", "primitive_buffer_allocate"])
            clip, allocate = sites[0][0], sites[1][0]
            self.assertEqual(word(clip + 8) >> 26, 6)  # blez: reject nonpositive faces
            size_instruction = word(allocate + 4)
            if size_instruction >> 26 != 13:
                size_instruction = word(allocate - 4)
            self.assertEqual(size_instruction, 0x34040000 | size)
        raw_color = blob[IMAGE_LAYOUTS["OPEN.EXE"].file_offset(0x800372F0):
                         IMAGE_LAYOUTS["OPEN.EXE"].file_offset(0x800372F0) + 4]
        self.assertEqual(raw_color, bytes((128, 128, 128, 0)))

    def test_compiled_mode_call_paths_and_resolved_data_referents(self) -> None:
        path = BUILD / "objdiff/open/base/8001764c_render_tmd.o"
        if not path.is_file():
            self.skipTest("compiled OPEN TMD renderer is required")
        obj = _load_object(path)
        raw = self.retail_bytes()
        offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(START)
        retail_text = raw[offset:offset + END - START]
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and START <= parse_int(row["site_va"]) < END]
        retail_calls = {parse_int(row["site_va"]): row["target_name"] for row in body
                        if row["opcode"] == "jal"}
        candidate_calls = {}
        actual_targets = []
        pending = []
        data = {item.name: item.va for (image, _), item in
                load_data_identities(RETAIL_CONFIG).items() if image == "OPEN.EXE"}
        for reloc in obj.relocations:
            if reloc.section != ".text":
                continue
            instruction = struct.unpack_from("<I", obj.sections[".text"], reloc.offset)[0]
            symbol = obj.symbol(reloc.symbol_index)
            if reloc.kind == 4 and instruction >> 26 == 3:
                candidate_calls[reloc.offset] = symbol.name
            elif reloc.kind == 5:
                pending.append((instruction, reloc.symbol_index))
            elif reloc.kind == 6:
                self.assertEqual(len(pending), 1)
                high, symbol_index = pending.pop()
                self.assertEqual(symbol_index, reloc.symbol_index)
                addend = decode_hi_lo_target(high, instruction)
                base = SWITCH if symbol.section == ".rodata" else data[symbol.name]
                actual_targets.append(base + addend)
        self.assertFalse(pending)
        self.assertEqual(list(candidate_calls.values()), list(retail_calls.values()))
        # Two legitimate absolute references replace unresolved graphics-wide base reuse.
        self.assertEqual(Counter(actual_targets), Counter(
            [parse_int(row["target_va"]) for row in body if row["kind"] == "mips_hi16_lo16"]
            + [0x80069B80, 0x80069A6C]))
        actual_without_separate_bases = [va for va in actual_targets
                                        if va not in (0x80069B80, 0x80069A6C)]
        self.assertEqual(actual_without_separate_bases,
                         [parse_int(row["target_va"]) for row in body
                          if row["kind"] == "mips_hi16_lo16"])
        table = []
        for reloc in obj.relocations:
            if reloc.section != ".rodata" or reloc.offset >= 116:
                continue
            self.assertEqual(reloc.kind, 2)
            symbol = obj.symbol(reloc.symbol_index)
            self.assertEqual(symbol.section, ".text")
            value = struct.unpack_from("<I", obj.sections[".rodata"], reloc.offset)[0]
            table.append((reloc.offset, symbol.value + value))
        self.assertEqual([offset for offset, _ in table], list(range(0, 116, 4)))
        targets = [value for _, value in table]
        candidate_next = Counter(targets).most_common(1)[0][0]

        def paths(text: bytes, base: int, entry: int, finish: int,
                  calls: dict[int, str]) -> set[tuple[str, ...]]:
            # Static call-path comparison only: no game or GTE execution. Follow
            # both conditional edges and discard paths ending at division traps.
            pending = [(entry, ())]
            visited = set()
            result = set()
            while pending:
                pc, trace = pending.pop()
                if pc == finish:
                    result.add(trace)
                    continue
                state = (pc, trace)
                if state in visited:
                    continue
                visited.add(state)
                self.assertLess(len(visited), 2000)
                self.assertTrue(base <= pc < base + len(text), hex(pc))
                instruction = struct.unpack_from("<I", text, pc - base)[0]
                opcode = instruction >> 26
                if opcode == 3:
                    pending.append((pc + 8, trace + (calls[pc],)))
                elif opcode == 2:
                    pending.append((decode_mips26_target(pc, instruction), trace))
                elif opcode in (1, 4, 5, 6, 7):
                    displacement = instruction & 0xFFFF
                    if displacement & 0x8000:
                        displacement -= 0x10000
                    pending.extend([(pc + 4 + 4 * displacement, trace), (pc + 8, trace)])
                elif opcode == 0 and instruction & 63 == 13:
                    continue
                else:
                    self.assertFalse(opcode == 0 and instruction & 63 in (8, 9))
                    pending.append((pc + 4, trace))
            return result

        for mode, entry in CASES.items():
            with self.subTest(mode=hex(mode)):
                expected = paths(retail_text, START, entry, 0x800182E8, retail_calls)
                actual = paths(obj.sections[".text"], 0, targets[mode - 0x20],
                               candidate_next, candidate_calls)
                self.assertEqual(actual, expected)
                self.assertTrue(any(trace[-1] == "AddPrim" for trace in actual if trace))
