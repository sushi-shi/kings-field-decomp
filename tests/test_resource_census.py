"""Byte census controls keep layout, bounds and absence evidence explicit."""

from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
import hashlib
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.resource_census import census, main


class ResourceCensusTest(unittest.TestCase):
    def test_counts_offsets_bytes_and_requested_absence(self):
        data = bytes.fromhex("aabbccdd 01fe1122 02fd3344 02fc5566 deadbeef")
        report = census(data, offset=4, length=12, stride=4, field_offset=0,
                        encoding="u8", values=(3,))
        self.assertEqual([(r["value"], r["count"]) for r in report["values"]],
                         [(1, 1), (2, 2), (3, 0)])
        references = report["values"][1]["references"]
        self.assertEqual(references, [
            {"row_index": 1, "record_offset": 8, "field_offset": 8,
             "field_bytes": "02", "record_bytes": "02fd3344"},
            {"row_index": 2, "record_offset": 12, "field_offset": 12,
             "field_bytes": "02", "record_bytes": "02fc5566"},
        ])
        self.assertEqual(report["values"][2]["references"], [])
        self.assertEqual(report["resource_sha256"], hashlib.sha256(data).hexdigest())
        self.assertEqual(report["region"]["sha256"], hashlib.sha256(data[4:16]).hexdigest())
        self.assertEqual(report["region"]["count"], 3)

    def test_signedness_endianness_and_field_offset(self):
        data = bytes.fromhex("aa ff fe bb 00 01")
        signed = census(data, offset=0, count=2, stride=3, field_offset=1, encoding="s16be")
        unsigned = census(data, offset=0, count=2, stride=3, field_offset=1, encoding="u16le")
        self.assertEqual([r["value"] for r in signed["values"]], [-2, 1])
        self.assertEqual([r["value"] for r in unsigned["values"]], [256, 65279])
        self.assertEqual(signed["values"][0]["references"][0]["field_offset"], 1)
        self.assertEqual(signed["values"][0]["references"][0]["field_bytes"], "fffe")

    def test_bad_regions_and_encodings_fail_before_reporting(self):
        base = {"offset": 0, "length": 8, "stride": 4, "field_offset": 0, "encoding": "u8"}
        cases = [
            ({"offset": -1}, "nonnegative"),
            ({"field_offset": -1}, "nonnegative"),
            ({"length": -4}, "nonnegative"),
            ({"stride": 0}, "positive"),
            ({"field_offset": 4}, "beyond"),
            ({"field_offset": 3, "encoding": "u16le"}, "beyond"),
            ({"length": 7}, "trailing"),
            ({"offset": 1}, "exceeds"),
            ({"length": 0, "offset": 9}, "exceeds"),
            ({"encoding": "guess"}, "unknown encoding"),
            ({"count": 2}, "exactly one"),
            ({"length": None}, "exactly one"),
            ({"length": None, "count": -1}, "nonnegative"),
            ({"values": (256,)}, "cannot be encoded"),
        ]
        for update, message in cases:
            with self.subTest(update=update), self.assertRaisesRegex(ValueError, message):
                census(bytes(8), **{**base, **update})

    def test_empty_selected_region_remains_explicit(self):
        report = census(bytes(8), offset=8, count=0, stride=4, field_offset=0,
                        encoding="u32le", values=(3,))
        self.assertEqual(report["region"]["count"], 0)
        self.assertEqual(report["values"], [{"value": 3, "count": 0, "references": []}])

    def test_cli_json_tsv_and_no_partial_result_on_failure(self):
        with TemporaryDirectory() as directory:
            path = Path(directory) / "resource.bin"
            path.write_bytes(bytes([0, 2, 2, 9]))
            args = [str(path), "--offset", "0x1", "--count", "2", "--stride", "1",
                    "--encoding", "u8", "--value", "3"]
            with redirect_stdout(StringIO()) as output, redirect_stderr(StringIO()):
                self.assertEqual(main([*args, "--json"]), 0)
                report = json.loads(output.getvalue())
                self.assertEqual(report["resource"], str(path))
                self.assertEqual([(r["value"], r["count"]) for r in report["values"]],
                                 [(2, 2), (3, 0)])
                output.truncate(0)
                output.seek(0)
                self.assertEqual(main(args), 0)
                self.assertIn("2\t2\t0;1\t0x1;0x2", output.getvalue())
                output.truncate(0)
                output.seek(0)
                self.assertEqual(main([*args, "--offset", "4"]), 2)
                self.assertEqual(output.getvalue(), "")


if __name__ == "__main__":
    unittest.main()
