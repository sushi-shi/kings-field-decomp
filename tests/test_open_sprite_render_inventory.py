from __future__ import annotations

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.inventory import (
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, REPO, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class OpenSpriteRenderTests(unittest.TestCase):
    def test_pinned_material_and_sdk_layouts_with_negative_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and Psy-Q headers unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/open_sprite_material_layout.c"
        with TemporaryDirectory(prefix="kf-sprite-material-layout-") as directory:
            root = Path(directory)
            for expected_size, item_size in ((8, 0x618), (9, 0x618), (8, 0x61C)):
                with self.subTest(expected_size=expected_size, item_size=item_size):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", sdk, f"-DEXPECTED_MATERIAL_SIZE={expected_size}",
                         f"-DEXPECTED_ITEM_STATE_SIZE={item_size}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if expected_size == 8 and item_size == 0x618:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        expected_error = ("sprite_material_size" if expected_size != 8
                                          else "floor_item_state_size")
                        self.assertIn(expected_error, result.stderr)

    def test_function_and_data_ownership(self) -> None:
        unit = load_manifest().by_name()["open.render_sprite"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([(function.va, function.body_size) for function in unit.functions],
                         [(0x800189A0, 0x21C)])
        self.assertEqual([(datum.va, datum.size, datum.symbol) for datum in unit.data], [
            (0x800372FC, 8, "render_sprite_light_normal"),
        ])
        functions = load_function_identities(RETAIL_CONFIG, required=True)
        identity = functions[("OPEN.EXE", 0x800189A0)]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("render_enqueue_sprite", "void",
                          "KfSpriteQuad *sprite;s16 depth_bias;s32 flag"))
        self.assertIn("open_semantic_sprite_render.tsv", identity.evidence)
        data = load_data_identities(RETAIL_CONFIG)
        for va, name, datatype, size, storage in (
            (0x800372FC, "render_sprite_light_normal", "SVECTOR", 8, "load"),
            (0x80049A48, "open_graphics_runtime", "KfGraphicsRuntimeOpen", 0x24788, "bss"),
        ):
            datum = data[("OPEN.EXE", va)]
            self.assertEqual((datum.name, datum.datatype, datum.size, datum.storage),
                             (name, datatype, size, storage))
        self.assertFalse(any(image == "OPEN.EXE" and 0x8006DA28 < va < 0x8006DA30
                             for image, va in data))
        structures = load_structure_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfSpriteMaterial"].size, 8)
        fields = {field.name: (field.offset, field.size, field.datatype)
                  for field in load_structure_field_identities(RETAIL_CONFIG)
                  if field.structure == "KfSpriteMaterial"}
        self.assertEqual(fields, {
            "clut": (0, 2, "u16"), "tpage": (2, 2, "u16"),
            "color": (4, 4, "CVECTOR"),
        })

    def test_complete_ordered_body_referents_and_material_writers(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and 0x800189A0 <= parse_int(row["site_va"]) < 0x80018BBC]
        self.assertEqual([(parse_int(row["site_va"]), parse_int(row["target_va"]),
                           row["target_name"]) for row in body], [
            (0x80018A38, 0x8002D820, "RotTransPers"),
            (0x80018A78, 0x8002DE88, "RotTransPers4"),
            (0x80018A80, 0x80016CB4, "primitive_buffer_allocate"),
            (0x80018A8C, 0x800340E0, "SetPolyFT4"),
            (0x80018A94, 0x8006DA28, "open_graphics_runtime"),
            (0x80018AAC, 0x8006DA2A, "open_graphics_runtime"),
            (0x80018B34, 0x8006DA2F, "open_graphics_runtime"),
            (0x80018B58, 0x800372FC, "render_sprite_light_normal"),
            (0x80018B64, 0x8002DAE8, "NormalColorDpq"),
            (0x80018B88, 0x80069A6C, "open_graphics_runtime"),
            (0x80018B94, 0x80033F74, "AddPrim"),
        ])
        material = [row for row in rows if row["image"] == "OPEN.EXE"
                    and 0x8006DA28 <= parse_int(row["target_va"]) < 0x8006DA30]
        self.assertEqual(len(material), 14)
        self.assertEqual({row["target_name"] for row in material}, {"open_graphics_runtime"})
        for row in body + material:
            self.assertEqual(row["status"], "reviewed")
            self.assertIn("manual:open_semantic_sprite_render", row["provenance"].split(";"))
        _, evidence = read_tsv(CONFIG / "evidence/open_semantic_sprite_render.tsv")
        self.assertEqual({parse_int(row["va"]) for row in evidence},
                         {0x800189A0, 0x800156BC, 0x800165C4, 0x800166B4})

    def test_retail_light_normal_and_material_pointer_chain(self) -> None:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        data = (retail / "OPEN.EXE").read_bytes()
        base = struct.unpack_from("<I", data, 0x18)[0]
        self.assertEqual(base, 0x80012000)

        def word(va: int) -> int:
            return struct.unpack_from("<I", data, va - base + 0x800)[0]

        self.assertEqual(struct.unpack_from("<4h", data, 0x800372FC - base + 0x800),
                         (0, 0, 0x1000, 0))
        # a1 is the CLUT base, then advances by four in the flag branch delay slot.
        self.assertEqual(word(0x80018A94), 0x3C058007)
        self.assertEqual(word(0x80018A98), 0x24A5DA28)
        self.assertEqual(word(0x80018A9C), 0x94A20000)
        self.assertEqual(word(0x80018B40), 0x24A50004)
        self.assertEqual(word(0x80018B64), 0x0C00B6BA)  # NormalColorDpq
        self.assertEqual(word(0x80018BB8), 0x27BD0088)  # return delay slot


if __name__ == "__main__":
    unittest.main()
