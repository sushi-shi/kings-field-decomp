"""Run with `nix develop --command python3 -m unittest discover -s tests`."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class NativeRegressions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="kf-native-regressions-")
        cls.addClassCleanup(cls.directory.cleanup)
        cls.binary = Path(cls.directory.name) / "native-regressions"
        subprocess.run(
            [
                "clang++", "-std=c++20", "-O1", "-g", "-fno-exceptions", "-fno-rtti",
                "-ffunction-sections", "-fdata-sections",
                "-Wl,--gc-sections", "-fsanitize=address,undefined",
                "-I", str(ROOT / "include"), str(ROOT / "tests/native_regressions.cpp"),
                "-o", str(cls.binary),
            ],
            check=True, text=True,
        )

    def run_case(self, mode, code, message=""):
        result = subprocess.run([self.binary, mode], capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, code, result.stderr)
        self.assertEqual(result.stderr, message)

    def test_original_common_table_spans(self):
        self.run_case("valid", 0)

    def test_truncated_armor_file(self):
        self.run_case("armor-truncated", 1,
                      "Truncated COM/COM.DAT armor table: need 1176 bytes, have 1175.\n")

    def test_truncated_object_file(self):
        self.run_case("objects-truncated", 1,
                      "Truncated COM/COM.DAT map-object table: need 1280 bytes, have 1279.\n")

    def test_ordinary_records_remain_chunk_bounded(self):
        self.run_case("chunk-only", 1, "Truncated armor chunk: need 1176 bytes, have 756.\n")

    def test_alignment_check(self):
        self.run_case("unaligned", 1, "Unaligned test record: requires 4-byte alignment.\n")

    def test_stream_tail_still_validates_chunk_header(self):
        self.run_case("invalid-header", 1, "Truncated resource chunk payload\n")

    def test_reset_scalars_arrays_nested_arrays_and_pointers(self):
        self.run_case("reset", 0)


if __name__ == "__main__":
    unittest.main()
