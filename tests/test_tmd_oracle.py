from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target
from scripts.kf.tmd_oracle import (
    CURRENT_ASSET_PTR_VA,
    GAME_EXTERNALS,
    INDEX_FIELDS,
    PARSER_VA,
    Relocation,
    apply_relocations,
    asset_archive_cases,
    execute_case,
    load_candidate_patch,
    reference_prepare,
    scan_tmd,
    single_asset_case,
    synthetic_cases,
)
from scripts.kf.paths import BUILD, LOCAL_CONFIG
from scripts.kf.parser_machine import LinkedFunction, LinkedProgram, MemoryRange, ParserMachine, RETURN_VA
from scripts.kf.sema.image import RetailImage


class TmdOracleTests(unittest.TestCase):
    def test_unknown_modes_advance_without_changing_their_payload(self) -> None:
        case = next(case for case in synthetic_cases() if "unknown-modes" in case.label)
        scan = scan_tmd(case.payload, 0, case.label)
        transformed = reference_prepare(case.payload, 0, case.label)
        self.assertEqual(scan.packets, 4)
        self.assertEqual(scan.modes, {0x21: 1, 0x10: 1, 0x40: 1, 0x20: 1})
        self.assertEqual(transformed[:64], case.payload[:64])
        self.assertNotEqual(transformed, case.payload)

    def test_asset_archive_walk_uses_header_sizes_and_tmd_offsets(self) -> None:
        first = bytearray(24)
        second = bytearray(28)
        struct.pack_into("<I", first, 0, len(first))
        struct.pack_into("<I", first, 8, 12)
        struct.pack_into("<I", second, 0, len(second))
        struct.pack_into("<I", second, 8, 16)
        archive = struct.pack("<HH", 2, 0) + first + second

        cases = asset_archive_cases(bytes(archive), "fixture")

        self.assertEqual(
            [(case.label, len(case.payload), case.tmd_offset) for case in cases],
            [("fixture/asset-00", 24, 12), ("fixture/asset-01", 28, 16)],
        )

    def test_synthetic_all_modes_cover_abe_and_expected_index_rewrites(self) -> None:
        case = synthetic_cases()[-1]
        scan = scan_tmd(case.payload, case.tmd_offset, case.label)
        transformed = reference_prepare(case.payload, case.tmd_offset, case.label)

        self.assertEqual(scan.objects, 1)
        self.assertEqual(scan.packets, 16)
        self.assertEqual(scan.modes, {mode: 2 for mode in INDEX_FIELDS})
        self.assertNotEqual(transformed, case.payload)

    def test_single_asset_requires_exact_declared_size(self) -> None:
        asset = bytearray(32)
        struct.pack_into("<I", asset, 0, len(asset))
        struct.pack_into("<I", asset, 8, 20)

        case = single_asset_case(bytes(asset), "one")

        self.assertEqual((len(case.payload), case.tmd_offset), (32, 20))
        struct.pack_into("<I", asset, 0, len(asset) - 4)
        with self.assertRaisesRegex(ValueError, "does not equal file size"):
            single_asset_case(bytes(asset), "bad")

    def test_mips_rel_relocations_apply_implicit_addends(self) -> None:
        data = bytearray(16)
        struct.pack_into("<I", data, 0, 0x0C000000)  # jal with zero addend
        struct.pack_into("<I", data, 4, 0x3C010000)  # lui at, 0
        struct.pack_into("<I", data, 8, 0x24211234)  # addiu at, at, 0x1234
        struct.pack_into("<I", data, 12, 0x00000B9C)  # .rodata table text addend
        relocated = apply_relocations(
            bytes(data),
            (
                Relocation(0, 4, "callee", 0x8002059C),
                Relocation(4, 5, "state", GAME_EXTERNALS["graphics_context"]),
                Relocation(8, 6, "state", GAME_EXTERNALS["graphics_context"]),
                Relocation(12, 2, ".text", 0x8001B7B4),
            ),
        )

        self.assertEqual(
            decode_mips26_target(PARSER_VA, struct.unpack_from("<I", relocated, 0)[0]),
            0x8002059C,
        )
        self.assertEqual(
            decode_hi_lo_target(
                struct.unpack_from("<I", relocated, 4)[0],
                struct.unpack_from("<I", relocated, 8)[0],
            ),
            GAME_EXTERNALS["graphics_context"] + 0x1234,
        )
        self.assertEqual(struct.unpack_from("<I", relocated, 12)[0], 0x8001C350)

    @unittest.skipUnless(importlib.util.find_spec("unicorn"), "Unicorn is not installed")
    def test_bounded_mips_machine_executes_a_leaf_and_returns(self) -> None:
        # addiu t0,zero,0x1234; sw t0,0(a0); jr ra; nop
        program = struct.pack("<4I", 0x24081234, 0xAC880000, 0x03E00008, 0)
        retail = RetailImage.synthetic("GAME.EXE", 0x80010000, program)
        linked = LinkedProgram((LinkedFunction("leaf", 0x80010000, 16, "retail"),))
        capture = MemoryRange("result", CURRENT_ASSET_PTR_VA, 4)
        result = ParserMachine(retail, linked).call("leaf", (CURRENT_ASSET_PTR_VA,),
                                                  capture=[capture], allowed_writes=[capture], instruction_limit=8)
        self.assertEqual(result.memory[0].data, struct.pack("<I", 0x1234))
        self.assertEqual(result.pc, RETURN_VA)

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and (BUILD / "objdiff/game/base/8001b7b0_render.o").is_file(),
        "local retail image and current render candidate are required",
    )
    def test_retail_and_candidate_agree_on_synthetic_all_modes(self) -> None:
        retail = RetailImage.load("GAME.EXE")
        candidate = load_candidate_patch(BUILD / "objdiff/game/base/8001b7b0_render.o")
        case = synthetic_cases()[-1]

        retail_result = execute_case(retail, case, None, 100_000)
        candidate_result = execute_case(retail, case, candidate, 100_000)

        expected = reference_prepare(case.payload, case.tmd_offset, case.label)
        self.assertEqual(retail_result.payload, expected)
        self.assertEqual(candidate_result.payload, expected)


if __name__ == "__main__":
    unittest.main()
