import importlib.util
from pathlib import Path
import struct
import unittest


spec = importlib.util.spec_from_file_location(
    "kf3_assets", Path(__file__).resolve().parents[1] / "scripts/kf3_assets.py"
)
assets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(assets)


class ArchiveTests(unittest.TestCase):
    def test_sector_aliases_and_terminal_aliases(self):
        data = bytearray(3 * 2048)
        struct.pack_into("<6H", data, 0, 4, 1, 1, 2, 3, 3)
        data[2048] = 17
        data[4096] = 29
        entries = assets.archive_entries(data)
        self.assertEqual([slots for slots, _ in entries], [[0, 1], [2]])
        self.assertEqual([entry[0] for _, entry in entries], [17, 29])
        for offsets in ((0, 1, 2, 3, 3), (1, 2, 1, 3, 3), (1, 1, 2, 3, 4)):
            bad = data.copy()
            struct.pack_into("<5H", bad, 2, *offsets)
            with self.assertRaises(ValueError):
                assets.archive_entries(bad)
        with self.assertRaises(ValueError):
            assets.archive_entries(data[:-1])

    def test_model_extents_and_face_indices(self):
        # A self-contained MO with one standard flat-shaded triangle.
        tmd = struct.pack("<III", 0x41, 0, 1)
        tmd += struct.pack("<IIIIIIi", 28, 3, 52, 1, 60, 1, 0)
        tmd += struct.pack("<12h", 0, 0, 0, 0, 100, 0, 0, 0, 0, 100, 0, 0)
        tmd += struct.pack("<4h", 0, 0, 4096, 0)
        tmd += struct.pack("<8B4H", 4, 3, 0, 0x20, 128, 64, 32, 0, 0, 0, 1, 2)
        data = struct.pack("<III", len(tmd) + 12, 0, 12) + tmd
        model = assets.read_model(data)
        self.assertEqual(model["objects"][0]["faces"][0]["vertices"], [0, 1, 2])
        self.assertIn("f 1 2 3", assets.obj_text(model))
        for end in range(len(data)):
            with self.assertRaises(ValueError):
                assets.read_model(data[:end])
        invalid_index = bytearray(data)
        struct.pack_into("<H", invalid_index, len(data) - 2, 3)
        with self.assertRaises(ValueError):
            assets.read_model(invalid_index)


class TextureTests(unittest.TestCase):
    def test_avatar_mesh_origin_scale_and_atlas(self):
        model = {"objects": [{"scale": 0, "vertices": [[0, -100, 0], [100, 0, 0], [0, 0, 100]],
            "faces": [{"vertices": [0, 1, 2], "colors": [[128, 64, 32]] * 3, "flags": 1}]}]}
        mesh = assets.avatar_mesh(model, ([], bytearray()), 41)
        self.assertEqual(struct.unpack_from("<HHI", mesh), (41, 1, 1))
        self.assertEqual(struct.unpack_from("<6hHH4B", mesh, 8), (0, -2000, 0, 0, 0, 0, 0, 0, 128, 64, 32, 1))
        self.assertEqual(len(mesh), 8 + 60 + 1024)
        self.assertEqual(mesh[-1024:], bytes([255]) * 1024)
        model["objects"][0]["scale"] = 1
        with self.assertRaises(ValueError):
            assets.avatar_mesh(model, ([], bytearray()), 41)

    @staticmethod
    def block(x, y, width, height, words):
        return struct.pack("<4H", x, y, width, height) * 2 + struct.pack("<" + "H" * len(words), *words)

    def test_palette_and_nibble_order(self):
        palette = [0, 31, 31 << 5, 31 << 10] + [0] * 12
        data = self.block(16, 480, 16, 1, palette) + self.block(64, 0, 1, 1, [0x3210])
        blocks, consumed = assets.texture_blocks(data + bytes(16))
        self.assertEqual(consumed, len(data))
        memory = assets.texture_memory(blocks)
        rgba = assets.texture_rgba(memory, 1, 480 * 64 + 1, 0, 0, 4, 1)
        self.assertEqual(rgba, bytes([0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]))
        self.assertTrue(assets.png_bytes(4, 1, rgba).startswith(b"\x89PNG\r\n\x1a\n"))
        with self.assertRaises(ValueError):
            assets.texture_rgba(memory, 2, 480 * 64 + 1, 0, 0, 4, 1)
        with self.assertRaises(ValueError):
            assets.texture_rgba(memory, 1, 480 * 64 + 2, 0, 0, 4, 1)
        bad = bytearray(data)
        bad[8] ^= 1
        with self.assertRaises(ValueError):
            assets.texture_blocks(bad)
        for end in range(1, len(data)):
            with self.assertRaises(ValueError):
                assets.texture_blocks(data[:end])


if __name__ == "__main__":
    unittest.main()
