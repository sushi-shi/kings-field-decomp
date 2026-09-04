from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.animation_oracle import (
    CANDIDATE_OBJECT,
    CANDIDATE_UNIT,
    FUNCTION,
    INITIAL_KF_INDEX,
    AnimationCase,
    apply_gte_mime,
    compare_cases,
    compare_lifecycle_cases,
    lifecycle_cases,
    parse_asset,
    synthetic_cases,
)
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import BUILD, LOCAL_CONFIG
from scripts.kf.rust_codec import DEFAULT_DRIVER, RustCodec
from scripts.kf.sema.image import RetailImage


def _put_vertex(data: bytearray, offset: int, values: tuple[int, int, int, int]) -> None:
    struct.pack_into("<4h", data, offset, *values)


def synthetic_asset() -> bytes:
    data = bytearray(176)
    struct.pack_into("<IiIII", data, 0, len(data), 1, 120, 56, 20)
    struct.pack_into("<I", data, 20, 24)
    struct.pack_into("<2H2I", data, 24, 2, 20, 36, 46)
    struct.pack_into("<5H", data, 36, 0, 2048, 0, 1, 1)
    struct.pack_into("<4H", data, 46, 0, 2048, 0, 0)
    struct.pack_into("<2I", data, 56, 64, 92)
    struct.pack_into("<3I", data, 64, 0x11111111, 0, 2)
    _put_vertex(data, 76, (2, 4, 6, 99))
    _put_vertex(data, 84, (8, 10, 12, 99))
    struct.pack_into("<3I", data, 92, 0x22222222, 0, 2)
    _put_vertex(data, 104, (10, 20, 30, 99))
    _put_vertex(data, 112, (20, 40, 60, 99))
    struct.pack_into("<3I", data, 120, 0x41, 0, 1)
    struct.pack_into("<2I", data, 132, 28, 2)
    _put_vertex(data, 160, (100, 200, 300, 7))
    _put_vertex(data, 168, (-100, -200, -300, 8))
    return bytes(data)


class AnimationOracleTests(unittest.TestCase):
    def test_candidate_object_follows_the_binder_unit_owner(self) -> None:
        manifest = load_manifest()
        unit = manifest.by_identity()[("GAME.EXE", 0x800205D4)]

        self.assertEqual(unit.unit, CANDIDATE_UNIT)
        self.assertEqual(unit.function.symbol, FUNCTION)
        self.assertEqual(CANDIDATE_OBJECT,
                         BUILD / "objdiff" / unit.image_key / "base" / unit.object_name)

    def test_bounded_asset_scan_recovers_clip_keyframe_and_morph_tables(self) -> None:
        asset = parse_asset(synthetic_asset(), "fixture")

        self.assertEqual(asset.vertex_count, 2)
        self.assertEqual(len(asset.clips), 1)
        self.assertEqual(asset.clips[0].unknown_02, 20)
        self.assertEqual(len(asset.clips[0].keyframes), 2)
        self.assertEqual(asset.clips[0].keyframes[0].morph_indices, (1,))
        self.assertEqual(
            [(item.offset, item.base_vertex, item.vertex_count) for item in asset.objects],
            [(64, 0, 2), (92, 0, 2)],
        )

    def test_gte_model_saturates_ir_wraps_halfwords_and_preserves_padding(self) -> None:
        destination = struct.pack("<4h", 32767, -32768, 1, 0x1234)
        source = struct.pack("<4h", -32768, 32767, -3, 0x5678)

        output = apply_gte_mime(destination, source, 1, 0xFFFF)

        self.assertEqual(struct.unpack("<4h", output), (-1, -1, -47, 0x1234))

    def test_scan_rejects_a_morph_range_beyond_tmd_vertices(self) -> None:
        data = bytearray(synthetic_asset())
        struct.pack_into("<I", data, 96, 2)

        with self.assertRaisesRegex(ValueError, "morph object 1 range 2..4 exceeds 2"):
            parse_asset(bytes(data), "bad")

    def test_lifecycle_controls_cover_pool_retry_reinit_and_static_release(self) -> None:
        selected = lifecycle_cases()

        self.assertEqual(
            [case.label for case in selected],
            [
                "lifecycle/null-record-success",
                "lifecycle/pool-unavailable",
                "lifecycle/malloc-retry",
                "lifecycle/different-asset-reinit",
                "lifecycle/static-record-release",
            ],
        )
        self.assertEqual(selected[2].allocation_results, (0, 0x800A0000))
        self.assertNotEqual(selected[3].record_asset_id, 7)
        self.assertFalse(selected[4].asset.clips)

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and CANDIDATE_OBJECT.is_file()
        and DEFAULT_DRIVER.is_file(),
        "local retail image, candidate object, and Rust driver are required",
    )
    def test_retail_candidate_and_rust_agree_on_a_synthetic_morph(self) -> None:
        asset = parse_asset(synthetic_asset(), "fixture")
        clip = asset.clips[0]
        keyframe = clip.keyframes[0]
        case = AnimationCase(
            "fixture/miss",
            asset,
            clip,
            keyframe,
            1024,
            INITIAL_KF_INDEX,
            False,
        )

        count, _retail_steps, _candidate_steps = compare_cases(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(),
            [case, *synthetic_cases()],
        )

        self.assertEqual(count, 4)

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and CANDIDATE_OBJECT.is_file()
        and DEFAULT_DRIVER.is_file(),
        "local retail image, candidate object, and Rust driver are required",
    )
    def test_retail_candidate_and_rust_agree_on_lifecycle_controls(self) -> None:
        count, _retail_steps, _candidate_steps = compare_lifecycle_cases(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(),
            lifecycle_cases(),
        )

        self.assertEqual(count, 5)


if __name__ == "__main__":
    unittest.main()
