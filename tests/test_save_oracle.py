from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import LOCAL_CONFIG, REPO
from scripts.kf.rust_codec import RustCodec
from scripts.kf.save_oracle import (
    HEADER_SIZE,
    SAVE_OBJECT,
    TRACE_OPEN,
    TRACE_READ,
    TRACE_SEEK,
    DeterministicIo,
    _header,
    compare_read_header,
    io_scenario,
)
from scripts.kf.sema.image import RetailImage


class FakeHookContext:
    def __init__(self, args: tuple[int, int, int, int], memory: bytearray) -> None:
        self.args = args
        self.memory = memory

    def write(self, address: int, data: bytes) -> None:
        self.memory[address:address + len(data)] = data


class SaveOracleTests(unittest.TestCase):
    def test_io_scenario_is_driver_little_endian_format(self) -> None:
        self.assertEqual(
            io_scenario(True, [1, -1, HEADER_SIZE]),
            b"\x01" + struct.pack("<Iiii", 3, 1, -1, HEADER_SIZE),
        )

    def test_deterministic_io_records_meaningful_ordered_effects(self) -> None:
        service = DeterministicIo(b"abcdefgh", True, [2])
        memory = bytearray(32)
        self.assertEqual(service.open(FakeHookContext((0, 0, 0, 0), memory)), 3)
        service.seek(FakeHookContext((3, 3, 0, 0), memory))
        self.assertEqual(service.read(FakeHookContext((3, 10, 4, 0), memory)), 2)
        self.assertEqual(memory[10:12], b"de")
        self.assertEqual(
            bytes(service.trace or b""),
            bytes((TRACE_OPEN, 1, TRACE_SEEK))
            + struct.pack("<I", 3)
            + bytes((TRACE_READ,))
            + struct.pack("<Ii", 4, 2),
        )

    def test_header_fixture_has_four_directory_entries(self) -> None:
        header = _header()
        self.assertEqual(len(header), HEADER_SIZE)
        self.assertEqual(header[0x200:0x204], bytes((1, 2, 3, 4)))

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and SAVE_OBJECT.is_file()
        and (REPO / "tools/target/debug/kf-codec-oracle").is_file(),
        "local retail image, candidate object, Rust driver, and Unicorn are required",
    )
    def test_three_way_read_header_scenarios(self) -> None:
        count = compare_read_header(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(REPO / "tools/target/debug/kf-codec-oracle"),
        )
        self.assertEqual(count, 4)


if __name__ == "__main__":
    unittest.main()
