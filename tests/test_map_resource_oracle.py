import struct
import unittest

from scripts.kf.map_resource_oracle import (
    ALL_GRIDS_SIZE,
    FILE_VA,
    SparseResourceFile,
    normalize_trace,
    verify_sparse_layout,
)
from scripts.kf.parser_machine import (
    CallRecord,
    LinkedFunction,
    LinkedProgram,
    MemoryPatch,
    MemoryRange,
)


def chunks(payloads: list[bytes]) -> bytes:
    return b"".join(struct.pack("<I", len(payload)) + payload for payload in payloads)


def call(index: int, name: str, *args: int) -> CallRecord:
    padded = (*args, 0, 0, 0, 0)[:4]
    return CallRecord(index, name, "test", 0x80010000 + index * 4, padded, ())


class SparseResourceFileTests(unittest.TestCase):
    def test_maps_only_walked_headers_and_grid_body(self) -> None:
        payloads = [b"vh", b"vb", bytes(ALL_GRIDS_SIZE + 19), b"i", b"o", b"a", b"d", b"e"]
        resource = SparseResourceFile.parse(
            "MIXA", chunks(payloads), chunks=8, header_count=7, grid_chunk=2
        )
        inputs = resource.sparse_inputs()
        self.assertEqual(len(inputs), 8)
        self.assertEqual(sum(len(item.data) for item in inputs), 7 * 4 + ALL_GRIDS_SIZE)
        self.assertEqual(inputs[-1].address, FILE_VA + resource.payloads[2].payload_offset)
        self.assertEqual(len(inputs[-1].data), ALL_GRIDS_SIZE)
        self.assertNotIn(
            FILE_VA + resource.payloads[7].header_offset,
            {item.address for item in inputs},
        )

    def test_rejects_truncation_and_trailing_data(self) -> None:
        with self.assertRaisesRegex(ValueError, "declares"):
            SparseResourceFile.parse(
                "bad", struct.pack("<I", 9) + b"x", chunks=1, header_count=1
            )
        with self.assertRaisesRegex(ValueError, "remain"):
            SparseResourceFile.parse(
                "bad", chunks([b"x"]) + b"tail", chunks=1, header_count=1
            )

    def test_sparse_layout_guard_checks_functions_and_patches(self) -> None:
        program = LinkedProgram(
            (LinkedFunction("parser", 0x80100000, 0x40, "candidate"),),
            (MemoryPatch("rodata", 0x80140000, bytes(0x40)),),
            (),
            "test",
        )
        verify_sparse_layout(program, [MemoryRange("safe", FILE_VA, 4)])
        for address in (0x80100010, 0x80140020):
            with self.assertRaisesRegex(AssertionError, "overlaps"):
                verify_sparse_layout(program, [MemoryRange("bad", address, 4)])


class TraceTests(unittest.TestCase):
    def test_normalizes_pointer_arguments_and_implicit_retained_cursor(self) -> None:
        mixa = SparseResourceFile.parse(
            "MIXA",
            chunks([b"vh", b"vb", bytes(ALL_GRIDS_SIZE), b"i", b"o", b"a", b"d", b"e"]),
            chunks=8,
            header_count=7,
            grid_chunk=2,
        )
        mixb = SparseResourceFile.parse(
            "MIXB",
            chunks([b"t1", b"t0", bytes(4), bytes(4), bytes(4)]),
            chunks=5,
            header_count=4,
        )
        calls = [
            call(0, "map_resource_path_set_floor", 3),
            call(1, "cd_file_load_allocated"),
            call(
                2,
                "audio_load_vab",
                FILE_VA + mixa.payloads[0].payload_offset,
                FILE_VA + mixa.payloads[1].payload_offset,
            ),
            call(3, "memory_release_last"),
            call(4, "memory_release_last"),
            call(
                5,
                "tmd_register",
                1,
                FILE_VA + mixb.payloads[0].payload_offset,
            ),
            call(
                6,
                "asset_registry_load_tmd_archive",
                30,
                FILE_VA + mixb.payloads[3].payload_offset,
            ),
            call(7, "memory_set_allocation_mode", 2),
        ]
        trace = normalize_trace(calls, mixa, mixb, [1])
        expected = bytearray(struct.pack("<Bi", 4, 3))
        expected.extend((5, 1, 8))
        for payload in mixa.payloads[:2]:
            expected.extend(
                struct.pack("<BII", payload.index, payload.payload_offset, payload.size)
            )
        expected.extend((7, 7))
        expected.extend(struct.pack("<BI", 15, mixa.payloads[1].header_offset + 16))
        expected.extend(struct.pack("<BH", 16, 1))
        payload = mixb.payloads[0]
        expected.extend(struct.pack("<BII", payload.index, payload.payload_offset, payload.size))
        expected.extend(struct.pack("<BH", 17, 30))
        payload = mixb.payloads[3]
        expected.extend(struct.pack("<BII", payload.index, payload.payload_offset, payload.size))
        expected.extend(struct.pack("<Bi", 21, 2))
        self.assertEqual(trace, expected)


if __name__ == "__main__":
    unittest.main()
