from __future__ import annotations

import struct
import unittest

from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.relocations import (
    decode_hi_lo_target, decode_mips26_target, encode_hi_lo_addend,
)
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


START = 0x80016ADC
SIZE = 0x1D8
ARITY = {
    "ResetGraph": 1, "InitGeom": 0, "SetGeomOffset": 2,
    "SetDefDrawEnv": 5, "SetDefDispEnv": 5, "PutDispEnv": 1,
    "SetDispMask": 1, "PutDrawEnv": 1, "SetBackColor": 3,
    "lighting_set_active_color_matrix": 1, "SetFarColor": 3,
    "SetFogNear": 2, "render_initialize": 0,
}


def path_effects(words: list[int], calls: dict[int, str], jumps: dict[int, int],
                 mode: int) -> tuple[list, list]:
    """Constant-propagate this initializer only; never invoke its callees.

    All non-stack accesses in this body are stores. Calls discard their results;
    model caller-saved registers as unavailable, not as preserved by an SDK.
    This checks arguments/writes, not timing, GPU behavior, or byte exactness.
    """
    registers = [0] + [0xCC000000 + index for index in range(1, 32)]
    registers[4], registers[29], registers[31] = mode & 0xFFFFFFFF, 0x81001000, 0x81002000
    stack = {}
    trace, writes = [], []

    def ordinary(pc: int) -> None:
        word = words[pc // 4]
        op, rs, rt, rd = word >> 26, word >> 21 & 31, word >> 16 & 31, word >> 11 & 31
        imm = word & 0xFFFF
        signed = imm if imm < 0x8000 else imm - 0x10000
        if word == 0:
            return
        if op == 0 and word & 63 == 33:  # addu, including move
            registers[rd] = (registers[rs] + registers[rt]) & 0xFFFFFFFF
        elif op == 9:
            registers[rt] = (registers[rs] + signed) & 0xFFFFFFFF
        elif op == 13:
            registers[rt] = registers[rs] | imm
        elif op == 15:
            registers[rt] = imm << 16
        elif op == 35:
            registers[rt] = stack[registers[rs] + signed]
        elif op in (40, 41, 43):
            address = (registers[rs] + signed) & 0xFFFFFFFF
            size = {40: 1, 41: 2, 43: 4}[op]
            value = registers[rt] & ((1 << (8 * size)) - 1)
            if rs == 29:
                if size != 4:
                    raise AssertionError("unexpected non-word stack store")
                stack[address] = value
            else:
                writes.append((address, size, value))
        else:
            raise AssertionError(f"unsupported instruction {word:#x} at +{pc:#x}")
        registers[0] = 0

    pc = 0
    for _ in range(160):
        word = words[pc // 4]
        op, rs, rt = word >> 26, word >> 21 & 31, word >> 16 & 31
        if op in (2, 3, 5) or word == 0x03E00008:
            taken = op == 5 and registers[rs] != registers[rt]
            if op == 3:
                registers[31] = START + pc + 8
            ordinary(pc + 4)  # Every control transfer owns this delay slot.
            if op == 3:
                name = calls[pc]
                arity = ARITY[name]
                args = registers[4:4 + min(arity, 4)]
                if arity == 5:
                    args.append(stack[registers[29] + 16])
                trace.append((name, tuple(args)))
                for register in list(range(1, 16)) + [24, 25]:
                    registers[register] = None
                pc += 8
            elif op == 2:
                pc = jumps[pc]
            elif op == 5:
                displacement = word & 0xFFFF
                if displacement >= 0x8000:
                    displacement -= 0x10000
                pc = pc + 4 + displacement * 4 if taken else pc + 8
            else:
                if (registers[29], registers[31]) != (0x81001000, 0x81002000):
                    raise AssertionError("frame/return restoration changed")
                return trace, writes
        else:
            ordinary(pc)
            pc += 4
    raise AssertionError("initializer path exceeded its static bound")


class OpenDisplayInitTests(unittest.TestCase):
    def retail(self):
        try:
            blob = (configured_retail_dir() / "OPEN.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(START)
        words = list(struct.unpack_from(f"<{SIZE // 4}I", blob, offset))
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        rows = [row for row in rows if row["image"] == "OPEN.EXE"
                and START <= parse_int(row["site_va"]) < START + SIZE]
        calls = {parse_int(row["site_va"]) - START: row["target_name"]
                 for row in rows if row["opcode"] == "jal"}
        jumps = {parse_int(row["site_va"]) - START: parse_int(row["target_va"]) - START
                 for row in rows if row["opcode"] == "j"}
        return words, calls, jumps, rows

    def test_retail_contract_and_raw_targets(self) -> None:
        words, calls, jumps, rows = self.retail()
        self.assertEqual(len(rows), 36)
        self.assertEqual({row["status"] for row in rows}, {"reviewed"})
        self.assertEqual(len(calls), 18)
        self.assertEqual(len(jumps), 2)
        for row in rows:
            site = parse_int(row["site_va"])
            word = words[(site - START) // 4]
            if row["kind"] == "mips_hi16_lo16":
                low = words[(parse_int(row["paired_site_va"]) - START) // 4]
                actual = decode_hi_lo_target(word, low)
            else:
                actual = decode_mips26_target(site, word)
            self.assertEqual(actual, parse_int(row["target_va"]))
        self.assertEqual(words[(0x80016B20 - START) // 4], 0x341000F0)
        self.assertEqual(words[(0x80016BF8 - START) // 4], 0x2604FFEA)
        self.assertEqual(words[(0x80016C04 - START) // 4], 0x26120046)
        for mode in (0, 1, 2, 0xFE, 0x100FE, -2):
            trace, writes = path_effects(words, calls, jumps, mode)
            self.assertEqual(trace[0], ("ResetGraph", (3 if mode == 0xFE else 0,)))
            self.assertEqual(trace[1:7], [
                ("InitGeom", ()), ("SetGeomOffset", (160, 120)),
                ("SetDefDrawEnv", (0x80069A70, 0, 0, 320, 240)),
                ("SetDefDispEnv", (0x80069B28, 0, 240, 320, 240)),
                ("SetDefDrawEnv", (0x80069ACC, 0, 240, 320, 240)),
                ("SetDefDispEnv", (0x80069B3C, 0, 0, 320, 240)),
            ])
            self.assertEqual(writes[-2:], [(0x8006E0A8, 4, 11000), (0x8006E1CC, 2, 1)])

    def test_compiled_paths_preserve_arguments_and_ordered_writes(self) -> None:
        retail, calls, jumps, rows = self.retail()
        path = BUILD / "objdiff/open/base/800168dc_render_init.o"
        if not path.is_file():
            self.skipTest("compiled OPEN render-init object is required")
        obj = _load_object(path)
        fn = obj.named_symbol("display_initialize")
        self.assertEqual(fn.size, SIZE)
        compiled = list(struct.unpack_from(f"<{fn.size // 4}I", obj.sections[".text"], fn.value))
        data = {item.name: item.va for (image, _), item in
                load_data_identities(RETAIL_CONFIG).items() if image == "OPEN.EXE"}
        candidate_calls, candidate_jumps, addresses, pending = {}, {}, [], []
        for reloc in obj.relocations:
            if reloc.section != ".text" or not fn.value <= reloc.offset < fn.value + fn.size:
                continue
            offset = reloc.offset - fn.value
            index = offset // 4
            symbol = obj.symbol(reloc.symbol_index)
            if reloc.kind == 5:
                pending.append((index, symbol.name))
            elif reloc.kind == 6:
                self.assertEqual(len(pending), 1)
                high, name = pending.pop()
                self.assertEqual(name, symbol.name)
                target = data[name] + decode_hi_lo_target(compiled[high], compiled[index])
                addresses.append(target)
                compiled[high], compiled[index] = encode_hi_lo_addend(
                    compiled[high], compiled[index], target)
            elif reloc.kind == 4:
                if compiled[index] >> 26 == 3:
                    if symbol.section == '.text':
                        from tests.test_open_runtime_owner_probe import text_referent

                        name, interior = text_referent(obj, symbol, (compiled[index] & 0x3FFFFFF) * 4)
                        self.assertEqual(interior, 0)
                        candidate_calls[offset] = name
                    else:
                        candidate_calls[offset] = symbol.name
                else:
                    self.assertEqual(symbol.section, ".text")
                    candidate_jumps[offset] = (
                        (compiled[index] & 0x3FFFFFF) * 4 + symbol.value - fn.value)
            else:
                self.fail(f"unexpected text relocation {reloc.kind}")
        self.assertFalse(pending)
        self.assertEqual(list(candidate_calls.values()), list(calls.values()))
        self.assertEqual(addresses, [
            parse_int(row["target_va"]) for row in rows if row["kind"] == "mips_hi16_lo16"
        ])
        init = next(offset for offset, name in candidate_calls.items() if name == "InitGeom")
        self.assertEqual(compiled[(init + 4) // 4], 0x341000F0)
        for mode in (0, 1, 2, 0xFE, 0x100FE, -2):
            self.assertEqual(path_effects(compiled, candidate_calls, candidate_jumps, mode),
                             path_effects(retail, calls, jumps, mode))

    def test_argument_and_field_corruption_are_observable(self) -> None:
        words, calls, jumps, _ = self.retail()
        expected = path_effects(words, calls, jumps, 0)
        wrong_projection = words.copy()
        wrong_projection[(0x80016C80 - START) // 4] ^= 1
        self.assertNotEqual(path_effects(wrong_projection, calls, jumps, 0), expected)
        wrong_field = words.copy()
        wrong_field[(0x80016C20 - START) // 4] ^= 1  # dfe -> adjacent dtd
        self.assertNotEqual(path_effects(wrong_field, calls, jumps, 0), expected)
