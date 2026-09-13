"""Physical transfer extents and logical views must agree on the target ABI."""

import os
import re
import shutil
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.paths import REPO


class VoidBoundaryLayoutTests(unittest.TestCase):
    def test_map_allocator_restores_only_complete_packet_types(self):
        source = (REPO / "src/open/render_map.c").read_text()
        restorations = re.findall(
            r"\((Kf\w+)\s*\*\)primitive_buffer_allocate\(sizeof\((POLY_\w+)\)\)",
            source,
        )
        self.assertEqual(restorations, [("KfGpuGT4", "POLY_GT4"), ("KfGpuGT3", "POLY_GT3")])

    def test_pinned_storage_layout_and_wrong_offset_control(self):
        sdk = os.environ.get("PSYQ_INCLUDE")
        cpp, cc1 = shutil.which("cpppsx-257"), shutil.which("cc1psx-257")
        if not sdk or not cpp or not cc1:
            self.skipTest("pinned compiler and SDK required")
        fixture = REPO / "tests/fixtures/void_boundary_layout.c"
        includes = ["-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"), "-I", sdk]
        with TemporaryDirectory(prefix="kf-void-boundary-layout-") as directory:
            source = Path(directory) / "layout.i"
            for offset in (0xcf0, 0x1000):
                with self.subTest(offset=offset):
                    result = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", *includes,
                         f"-DEXPECTED_COLLISION_GRID_OFFSET={offset}", str(fixture)],
                        check=True, capture_output=True,
                    )
                    source.write_bytes(result.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(Path(directory) / "layout.s")],
                        text=True, capture_output=True,
                    )
                    self.assertEqual(result.returncode == 0, offset == 0xcf0, result.stderr)
                    if offset != 0xcf0:
                        self.assertIn("grid_offset", result.stderr)

    def test_modern_and_retail_clang_agree_on_the_physical_owner(self):
        sdk = os.environ.get("PSYQ_INCLUDE")
        clang = shutil.which("clang")
        if not sdk or not clang:
            self.skipTest("pinned Clang and SDK required")
        for mode, flags in MODES.items():
            with self.subTest(mode=mode):
                result = subprocess.run(
                    [clang, *FLAGS, *flags, "-fsyntax-only", "-I", str(REPO / "include"),
                     "-I", str(REPO / "vendor/include"), "-I", sdk,
                     str(REPO / "tests/fixtures/void_boundary_layout.c")],
                    capture_output=True, text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
