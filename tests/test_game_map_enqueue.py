from __future__ import annotations

import struct
import unittest

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.sema.mips import branch_target


START = 0x8001DE18
SIZE = 0x418


class GameMapEnqueueTests(unittest.TestCase):
    def retail_words(self) -> list[int]:
        try:
            raw = (configured_retail_dir() / "GAME.EXE").read_bytes()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        offset = IMAGE_LAYOUTS["GAME.EXE"].file_offset(START)
        return list(struct.unpack_from(f"<{SIZE // 4}I", raw, offset))

    def assert_overflow_returns(self, words: list[int], calls: list[int]) -> None:
        # Narrow control for the two decoded allocation guards, not a general
        # CFG-equivalence proof. Their destinations must be the restore tail,
        # never the packet advance or another primitive's body.
        self.assertEqual(len(calls), 2)
        epilogues = [i for i, word in enumerate(words) if word == 0x8FBF004C]
        self.assertEqual(len(epilogues), 1)
        epilogue = epilogues[0]
        self.assertEqual(words[epilogue:], [
            0x8FBF004C, 0x8FBE0048, 0x8FB70044, 0x8FB60040,
            0x8FB5003C, 0x8FB40038, 0x8FB30034, 0x8FB20030,
            0x8FB1002C, 0x8FB00028, 0x03E00008, 0x27BD0050,
        ])
        for call in calls:
            index = call // 4
            self.assertEqual(words[index - 6:index - 2], [
                0x8C430008,  # cursor = buffer->cursor
                0x8C420004,  # end = buffer->end
                0, 0x0043102B,  # end < cursor, unsigned
            ])
            guard = words[index - 2]
            self.assertEqual(guard >> 16, 0x1440)  # bnez v0
            self.assertEqual(branch_target(call - 8, guard), epilogue * 4, hex(call))
            # Both normal and overflow paths execute this owned delay slot.
            self.assertIn(words[index - 1], (0, 0x0096A821, 0x0097A821))

    def test_retail_and_compiled_allocation_overflow_exit_the_function(self) -> None:
        words = self.retail_words()
        self.assert_overflow_returns(words, [0x8001DF38 - START, 0x8001E0C0 - START])
        path = BUILD / "objdiff/game/base/8001c7f8_render_enqueue.o"
        if not path.is_file():
            self.skipTest("compiled GAME map renderer is required")
        obj = _load_object(path)
        function = obj.named_symbol("render_enqueue_map")
        text = obj.sections[".text"][function.value:function.value + function.size]
        calls = [reloc.offset - function.value for reloc in obj.relocations
                 if reloc.section == ".text" and reloc.kind == 4
                 and function.value <= reloc.offset < function.value + function.size
                 and obj.symbol(reloc.symbol_index).name in ("SetPolyGT3", "SetPolyGT4")]
        self.assert_overflow_returns(list(struct.unpack(f"<{len(text) // 4}I", text)), calls)

    def test_packet_advance_is_not_an_allocation_failure_exit(self) -> None:
        words = self.retail_words()
        calls = [0x8001DF38 - START, 0x8001E0C0 - START]
        for call in calls:
            with self.subTest(packet_setup_offset=hex(call)):
                changed = words.copy()
                guard = call - 8
                displacement = ((0x8001E1E0 - START) - (guard + 4)) // 4
                changed[guard // 4] = (changed[guard // 4] & 0xFFFF0000) | displacement
                with self.assertRaises(AssertionError):
                    self.assert_overflow_returns(changed, calls)
