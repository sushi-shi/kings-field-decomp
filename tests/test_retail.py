from __future__ import annotations

import hashlib
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.retail import ImageLayout, _merged_coverage, parse_psx_exe
from scripts.kf.seed_retail import escape_trailing_spaces
from scripts.kf.propose_function_admission import subtract_interval


class RetailTests(unittest.TestCase):
    def test_subtract_interval(self) -> None:
        self.assertEqual(
            subtract_interval(0x100, 0x200, [(0x80, 0x110), (0x140, 0x160)]),
            [(0x110, 0x140), (0x160, 0x200)],
        )

    def test_escape_trailing_spaces_preserves_byte_count(self) -> None:
        self.assertEqual(escape_trailing_spaces("KF   "), r"KF\x20\x20\x20")
        self.assertEqual(escape_trailing_spaces("KF"), "KF")

    def test_merged_coverage(self) -> None:
        self.assertEqual(
            _merged_coverage([(8, 12), (0, 4), (3, 9), (20, 24)]),
            [(0, 12), (20, 24)],
        )

    def test_parse_psx_exe(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "TEST.EXE"
            data = bytearray(0x820)
            data[:8] = b"PS-X EXE"
            struct.pack_into("<4I", data, 0x10, 0x80010000, 0, 0x80010000, 0x20)
            path.write_bytes(data)
            parsed = parse_psx_exe(path)
        self.assertEqual(
            parsed,
            ImageLayout(
                "TEST.EXE",
                hashlib.sha256(data).hexdigest(),
                0x820,
                0x80010000,
                0x80010000,
                0x20,
            ),
        )


if __name__ == "__main__":
    unittest.main()
