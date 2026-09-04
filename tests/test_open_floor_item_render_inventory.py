from __future__ import annotations

import hashlib
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


class OpenFloorItemRenderTests(unittest.TestCase):
    def retail_path(self, relative: str) -> Path:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        path = retail / relative
        if not path.is_file():
            self.skipTest(f"optional retail resource {relative} is unavailable")
        return path

    def test_layout_with_pinned_compiler_and_negative_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        if cpp is None or cc1 is None:
            self.skipTest("pinned GCC probe unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/open_sprite_layout.c"
        with TemporaryDirectory(prefix="kf-sprite-layout-") as directory:
            root = Path(directory)
            for expected_size in (12, 13):
                with self.subTest(expected_size=expected_size):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         f"-DEXPECTED_SPRITE_SIZE={expected_size}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if expected_size == 12:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("sprite_size", result.stderr)

    def test_typed_boundaries_and_contiguous_module(self) -> None:
        evidence = CONFIG / "evidence/open_semantic_floor_item_render.tsv"
        _, rows = read_tsv(evidence)
        functions = load_function_identities(RETAIL_CONFIG, required=True)
        expected = {
            0x800190F4: ("render_floor_item", "KfFloorItem *item"),
            0x800189A0: (
                "render_enqueue_sprite", "KfSpriteQuad *sprite;s16 depth_bias;s32 flag",
            ),
        }
        self.assertTrue(set(expected) <= {parse_int(row["va"]) for row in rows})
        for row in rows:
            va = parse_int(row["va"])
            if va not in expected:
                continue
            identity = functions[("OPEN.EXE", va)]
            name, parameters = expected[va]
            self.assertEqual((identity.name, identity.return_type, identity.parameters),
                             (name, "void", parameters))
            self.assertIn(evidence.name, identity.evidence)
            self.assertEqual(row["final_signature"],
                             f"void {name}({parameters.replace(';', ', ')})")
        unit = load_manifest().by_name()["open.entity_render"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual([function.va for function in unit.functions],
                         [0x80018ECC, 0x800190F4, 0x80019240])
        self.assertEqual([(datum.va, datum.size, datum.symbol) for datum in unit.data],
                         [(0x800358E0, 0x54, "floor_item_sprites"),
                          (0x800359E4, 0x20, "floor_item_light_matrix")])
        self.assertEqual(unit.rodata, (0x80012238, 0x44))

    def test_shared_sprite_layout_and_referents(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfSpriteQuad"].size, 12)
        fields = {field.name: (field.offset, field.size, field.datatype)
                  for field in load_structure_field_identities(RETAIL_CONFIG)
                  if field.structure == "KfSpriteQuad"}
        self.assertEqual(fields, {
            "u": (0, 1, "u8"), "v": (1, 1, "u8"),
            "u_span": (2, 1, "u8"), "v_span": (3, 1, "u8"),
            "x": (4, 2, "u16"), "y": (6, 2, "u16"),
            "w": (8, 2, "u16"), "h": (10, 2, "u16"),
        })
        data = load_data_identities(RETAIL_CONFIG)
        sprites = data[("OPEN.EXE", 0x800358E0)]
        self.assertEqual((sprites.name, sprites.datatype, sprites.size, sprites.storage),
                         ("floor_item_sprites", "KfSpriteQuad[7]", 0x54, "load"))
        self.assertFalse(any(image == "OPEN.EXE" and 0x800358E0 < va < 0x80035934
                             for image, va in data))
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        by_site = {parse_int(row["site_va"]): row for row in rows
                   if row["image"] == "OPEN.EXE"}
        body = {site: row for site, row in by_site.items()
                if 0x800190F4 <= site < 0x80019240}
        self.assertEqual(len(body), 16)
        self.assertEqual({row["status"] for row in body.values()}, {"reviewed"})
        self.assertEqual(body[0x800191F0]["target_name"], "floor_item_sprites")
        self.assertEqual(parse_int(body[0x800191F0]["target_va"]), 0x800358E0)
        self.assertEqual(body[0x800191F8]["target_name"], "render_enqueue_sprite")
        self.assertEqual(by_site[0x80019488]["target_name"], "render_floor_item")
        self.assertEqual(by_site[0x80019488]["status"], "reviewed")

    def test_retail_sprite_records(self) -> None:
        data = self.retail_path("OPEN.EXE").read_bytes()
        base = struct.unpack_from("<I", data, 0x18)[0]
        self.assertEqual(base, 0x80012000)
        start = 0x800358E0 - base + 0x800
        table = data[start:start + 0x54]
        self.assertEqual(hashlib.sha256(table).hexdigest(),
                         "48983ebb2b808edd22313decd08d5c19ab00fcffcb35d4d6003445023345a1f0")
        rows = [struct.unpack_from("<4B4H", table, i * 12) for i in range(7)]
        self.assertEqual([row[0] for row in rows],
                         [0x90, 0xB0, 0xD0, 0xB0, 0x90, 0xB0, 0xD0])
        self.assertEqual([row[1:] for row in rows[:4]],
                         [(0, 0x20, 0x20, 0xFE00, 0xFC40, 0x400, 0x400)] * 4)
        self.assertEqual([row[1:] for row in rows[4:]],
                         [(0x20, 0x20, 0x27, 0xFE00, 0xFB40, 0x400, 0x500)] * 3)

    def test_scene_zero_placements_use_first_four_sprites(self) -> None:
        data = self.retail_path("KF/B0/MIXA0.").read_bytes()
        self.assertEqual(len(data), 564052)
        self.assertEqual(hashlib.sha256(data).hexdigest(),
                         "4221c17d405e2b85d63f925977cc1d4418a5e35ecd0d2c355bfbf8caa7718526")
        offset = 0
        # Two VAB chunks, then the combined five-grid chunk.
        for _ in range(3):
            offset += struct.unpack_from("<I", data, offset)[0] + 4
        self.assertEqual(offset, 0x8996C)
        size = struct.unpack_from("<I", data, offset)[0]
        self.assertEqual(size, 15 * 12)
        rows = [struct.unpack_from("<H4B3h", data, offset + 4 + i * 12)
                for i in range(15)]
        self.assertEqual(rows[-1][0], 0xFFFF)
        self.assertEqual([(row[0], row[1]) for row in rows[:-1]], [(0, 4)] * 14)


if __name__ == "__main__":
    unittest.main()
