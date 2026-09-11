from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class OpenMapEnqueueTests(unittest.TestCase):
    def test_pinned_layouts_with_wrong_packet_size_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and Psy-Q headers unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/open_map_enqueue_layout.c"
        with TemporaryDirectory(prefix="kf-map-enqueue-layout-") as directory:
            root = Path(directory)
            for expected_size in (52, 48):
                with self.subTest(expected_size=expected_size):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"),
                         "-I", sdk, f"-DEXPECTED_GT4_SIZE={expected_size}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if expected_size == 52:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("gt4_size", result.stderr)

    def test_identity_complete_colour_owner_and_campaign(self) -> None:
        unit = load_manifest().by_name()["open.render_map"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([(f.va, f.body_size) for f in unit.functions],
                         [(0x800185E8, 0x3B8)])
        self.assertEqual([(d.va, d.size) for d in unit.data], [(0x800372F8, 4)])
        self.assertIsNone(unit.rodata)
        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("OPEN.EXE", 0x800185E8)]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("render_enqueue_map", "void", "u16 object_index"))
        self.assertIn("open_semantic_map_enqueue.tsv", identity.evidence)
        data = load_data_identities(RETAIL_CONFIG)
        colour = data[("OPEN.EXE", 0x800372F8)]
        self.assertEqual((colour.name, colour.datatype, colour.size, colour.storage),
                         ("map_textured_primitive_color", "CVECTOR", 4, "load"))
        self.assertFalse(any(image == "OPEN.EXE" and 0x800372F8 < va < 0x800372FC
                             for image, va in data))
        _, rows = read_tsv(CONFIG / "evidence/open_semantic_map_enqueue.tsv")
        self.assertEqual({parse_int(row["va"]) for row in rows}, {0x800185E8, 0x80018BBC})
        _, vendors = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")
        self.assertFalse(any(row["image"] == "OPEN.EXE"
                             and parse_int(row["va"]) == 0x800185E8 for row in vendors))

    def test_ordered_calls_and_interior_colour_relocation(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and 0x800185E8 <= parse_int(row["site_va"]) < 0x800189A0]
        self.assertEqual(len(body), 21)
        self.assertEqual({row["status"] for row in body}, {"reviewed"})
        self.assertEqual([(parse_int(row["site_va"]), row["target_name"])
                          for row in body if row["opcode"] == "jal"], [
            (0x80018614, "tmd_get_object"), (0x8001863C, "tmd_project_vertices"),
            (0x800186B8, "NormalClip"), (0x800186D0, "primitive_buffer_allocate"),
            (0x800186DC, "SetPolyGT4"), (0x8001876C, "NormalColorCol"),
            (0x8001877C, "DpqColor"), (0x8001878C, "DpqColor"),
            (0x8001879C, "DpqColor"), (0x800187AC, "DpqColor"),
            (0x80018814, "NormalClip"), (0x80018824, "primitive_buffer_allocate"),
            (0x80018830, "SetPolyGT3"), (0x800188A8, "NormalColorCol"),
            (0x800188B8, "DpqColor"), (0x800188C8, "DpqColor"),
            (0x800188D8, "DpqColor"), (0x80018948, "AddPrim"),
        ])
        pairs = [row for row in body if row["kind"] == "mips_hi16_lo16"]
        self.assertEqual([(parse_int(row["target_va"]), row["target_name"])
                          for row in pairs], [
            (0x80069B60, "open_graphics_runtime"),
            (0x800372F8 + 3, "map_textured_primitive_color"),
        ])
        jump = next(row for row in body if row["opcode"] == "j")
        self.assertEqual((parse_int(jump["site_va"]), parse_int(jump["target_va"])),
                         (0x800187E8, 0x80018940))
        caller = next(row for row in rows if row["image"] == "OPEN.EXE"
                      and parse_int(row["site_va"]) == 0x80018D6C)
        self.assertEqual(caller["target_name"], "render_enqueue_map")
        for row in body + [caller]:
            self.assertIn("manual:open_semantic_map_enqueue", row["provenance"].split(";"))

    def test_raw_retail_fields_signed_depth_and_owner_bytes(self) -> None:
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
            0x80018618: 0x3084FFFF, 0x80018628: 0x8E040004,
            0x80018644: 0x8E170014, 0x8001864C: 0x8E230000,
            0x80018664: 0x26360020, 0x8001866C: 0x27DE72FB,
            0x80018694: 0x96420012, 0x80018698: 0x96430016,
            0x800186A4: 0x96420014, 0x800186C8: 0x96500018,
            0x800186CC: 0x34040034, 0x80018728: 0xAE22002C,
            0x80018754: 0x27C5FFFD, 0x8001875C: 0xA3C30000,
            0x80018760: 0x96440010, 0x80018778: 0x86A50006,
            0x800187B0: 0x26260028, 0x800187D4: 0x00021103,
            0x800187D8: 0x244400C8, 0x800187DC: 0x28824000,
            0x800187EC: 0x30843FFF, 0x800187F0: 0x9642000E,
            0x800187F4: 0x96430012, 0x80018800: 0x96420010,
            0x80018828: 0x34040028, 0x8001889C: 0x9644000C,
            0x800188F8: 0x0062001A, 0x80018904: 0x0007000D,
            0x8001891C: 0x0006000D, 0x80018928: 0x00031883,
            0x8001892C: 0x246300C8, 0x80018930: 0x28624000,
            0x8001893C: 0x30643FFF, 0x80018940: 0x8EC2FEEC,
            0x8001895C: 0x304203FC, 0x8001899C: 0x27BD0050,
        }.items():
            with self.subTest(va=hex(va)):
                self.assertEqual(word(va), expected)
        self.assertEqual(0x80069B60 + 32, 0x80069B80)
        self.assertEqual(0x80069B80 - 276, 0x80069A6C)
        start = 0x800372F8 - base + 0x800
        self.assertEqual(data[start:start + 4], bytes((128, 128, 128, 0)))


if __name__ == "__main__":
    unittest.main()
