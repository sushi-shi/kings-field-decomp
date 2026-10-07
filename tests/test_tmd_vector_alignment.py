"""GTE vector access preconditions the C types do not express."""

import os
import shutil
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.paths import REPO

FIXTURE = REPO / "tests/fixtures/tmd_vector_alignment.c"


class TmdVectorAlignmentTests(unittest.TestCase):
    def test_prepared_offsets_stay_element_sized_on_the_pinned_compiler(self):
        sdk = os.environ.get("PSYQ_INCLUDE")
        cpp, cc1 = shutil.which("cpppsx-257"), shutil.which("cc1psx-257")
        if not sdk or not cpp or not cc1:
            self.skipTest("pinned compiler and SDK required")
        includes = ["-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"), "-I", sdk]
        with TemporaryDirectory(prefix="kf-tmd-vector-alignment-") as directory:
            source = Path(directory) / "alignment.i"
            for shift in (3, 2):
                with self.subTest(shift=shift):
                    result = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", *includes,
                         f"-DEXPECTED_VECTOR_OFFSET_SHIFT={shift}", str(FIXTURE)],
                        check=True, capture_output=True,
                    )
                    source.write_bytes(result.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(Path(directory) / "alignment.s")],
                        text=True, capture_output=True,
                    )
                    self.assertEqual(result.returncode == 0, shift == 3, result.stderr)
                    if shift != 3:
                        self.assertIn("vector_offset_shift", result.stderr)

    def test_modern_and_retail_clang_agree_on_the_vector_preconditions(self):
        sdk = os.environ.get("PSYQ_INCLUDE")
        clang = shutil.which("clang")
        if not sdk or not clang:
            self.skipTest("pinned Clang and SDK required")
        for mode, flags in MODES.items():
            with self.subTest(mode=mode):
                result = subprocess.run(
                    [clang, *FLAGS, *flags, "-fsyntax-only", "-I", str(REPO / "include"),
                     "-I", str(REPO / "vendor/include"), "-I", sdk, str(FIXTURE)],
                    capture_output=True, text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
