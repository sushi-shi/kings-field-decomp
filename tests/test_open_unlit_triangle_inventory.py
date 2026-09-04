from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.inventory import load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class OpenUnlitTriangleTests(unittest.TestCase):
    def test_pinned_packet_layouts_with_negative_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and Psy-Q headers unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/open_unlit_triangle_layout.c"
        with TemporaryDirectory(prefix="kf-unlit-layout-") as directory:
            root = Path(directory)
            for expected_size in (32, 36):
                with self.subTest(expected_size=expected_size):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", sdk, f"-DEXPECTED_FT3_SIZE={expected_size}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if expected_size == 32:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("gpu_textured_size", result.stderr)

    def test_identity_and_unit_ownership(self) -> None:
        unit = load_manifest().by_name()["open.render_unlit"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([(function.va, function.body_size) for function in unit.functions],
                         [(0x80018344, 0x2A4)])
        self.assertFalse(unit.data)
        self.assertIsNone(unit.rodata)
        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("OPEN.EXE", 0x80018344)]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("render_enqueue_unlit_triangles", "void",
                          "u16 object_index;s16 depth_bias"))
        self.assertIn("open_semantic_unlit_triangles.tsv", identity.evidence)
        _, rows = read_tsv(CONFIG / "evidence/open_semantic_unlit_triangles.tsv")
        self.assertEqual({parse_int(row["va"]) for row in rows}, {0x80018344, 0x80018ECC})
        _, vendors = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        self.assertFalse(any(row["image"] == "OPEN.EXE"
                             and parse_int(row["va"]) == 0x80018344 for row in vendors))

    def test_ordered_reviewed_references(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and 0x80018344 <= parse_int(row["site_va"]) < 0x800185E8]
        self.assertEqual(len(body), 10)
        self.assertEqual({row["status"] for row in body}, {"reviewed"})
        self.assertEqual([(parse_int(row["site_va"]), row["target_name"])
                          for row in body if row["opcode"] == "jal"], [
            (0x80018374, "tmd_get_object"), (0x800183F4, "NormalClip"),
            (0x80018404, "primitive_buffer_allocate"), (0x80018410, "SetPolyFT3"),
            (0x800184C0, "NormalClip"), (0x800184D0, "primitive_buffer_allocate"),
            (0x800184DC, "SetPolyF3"), (0x80018598, "AddPrim"),
        ])
        caller = next(row for row in rows if row["image"] == "OPEN.EXE"
                      and parse_int(row["site_va"]) == 0x80019094)
        self.assertEqual(caller["target_name"], "render_enqueue_unlit_triangles")
        for row in body + [caller]:
            self.assertIn("manual:open_semantic_unlit_triangles", row["provenance"].split(";"))

    def test_retail_base_chain_packet_widths_and_depth(self) -> None:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        data = (retail / "OPEN.EXE").read_bytes()
        base = struct.unpack_from("<I", data, 0x18)[0]
        self.assertEqual(base, 0x80012000)

        def word(va: int) -> int:
            return struct.unpack_from("<I", data, va - base + 0x800)[0]

        for va, expected in {
            0x8001837C: 0x8C560014, 0x80018380: 0x3C048007,
            0x80018384: 0x24849B60, 0x800183A4: 0x24950020,
            0x800183D0: 0x9622000E, 0x800183D4: 0x96230012,
            0x800183E0: 0x96220010, 0x80018408: 0x34040020,
            0x80018438: 0xAE020008, 0x8001845C: 0xA602000C,
            0x80018478: 0x92A23EAC, 0x80018484: 0x92A23EAD,
            0x80018490: 0x92A23EAE, 0x800184D4: 0x34040014,
            0x80018544: 0x0062001A, 0x80018574: 0x00031883,
            0x8001857C: 0x28620005, 0x8001858C: 0x30643FFF,
            0x80018590: 0x8EA2FEEC, 0x800185A4: 0x304203FC,
            0x800185E4: 0x27BD0040,
        }.items():
            with self.subTest(va=hex(va)):
                self.assertEqual(word(va), expected)
        self.assertEqual(0x80069B60 + 32, 0x80069B80)
        self.assertEqual(0x80069B80 + 16044, 0x8006DA2C)
        self.assertEqual(0x80069B80 - 276, 0x80069A6C)


if __name__ == "__main__":
    unittest.main()
