from __future__ import annotations

import struct
import unittest

from scripts.kf.save_write_oracle import WriteSink, words


class Context:
    def __init__(self, args: tuple[int, int, int, int], data: bytes = b"") -> None:
        self.args = args
        self.data = data

    def read(self, address: int, size: int) -> bytes:
        return self.data[address:address + size]


class SaveWriteSinkTests(unittest.TestCase):
    def test_partial_write_records_full_request_but_changes_only_returned_bytes(self) -> None:
        sink = WriteSink(bytearray(b"........"), [], [2], [])
        sink.seek(Context((7, 1, 0, 0)))
        self.assertEqual(sink.write(Context((7, 0, 4, 0), b"abcd")), 2)
        self.assertEqual(sink.file, b".ab.....")
        self.assertEqual(sink.offset, 3)
        self.assertEqual(sink.trace, struct.pack("<BII", 3, 7, 1)
                         + struct.pack("<BIIi", 4, 7, 4, 2) + b"abcd")

    def test_negative_write_is_observable_without_mutating_the_file(self) -> None:
        sink = WriteSink(bytearray(b"...."), [], [-1], [])
        self.assertEqual(sink.write(Context((3, 0, 4, 0), b"abcd")), -1)
        self.assertEqual(sink.file, b"....")
        self.assertEqual(sink.offset, 0)

    def test_out_of_file_write_is_rejected(self) -> None:
        sink = WriteSink(bytearray(2), [], [], [])
        with self.assertRaisesRegex(AssertionError, "escapes simulated card file"):
            sink.write(Context((3, 0, 4, 0), b"abcd"))

    def test_result_words_preserve_signed_bios_results(self) -> None:
        self.assertEqual(words([-1, 0, 3]), struct.pack("<iii", -1, 0, 3))


if __name__ == "__main__":
    unittest.main()
