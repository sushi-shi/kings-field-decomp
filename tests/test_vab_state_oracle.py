from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import LOCAL_CONFIG
from scripts.kf.rust_codec import DEFAULT_DRIVER, RustCodec
from scripts.kf.sema.image import RetailImage
from scripts.kf.vab_state_oracle import (
    AUDIO_STATE_SIZE,
    CANDIDATE_OBJECT,
    POINTER_TABLE_SIZE,
    SPU_ALLOCATION,
    VB_VA,
    VH_VA,
    VabCase,
    compare_vab_states,
    seeded_request,
    vab_cases,
)


def synthetic_vab() -> VabCase:
    programs = 1
    samples = 2
    tones_at = 32 + 128 * 16
    lengths_at = tones_at + programs * 16 * 32
    vh = bytearray(lengths_at + 512)
    vb = bytes(range(48))
    struct.pack_into(
        "<4sII I HHHH4BI",
        vh,
        0,
        b"pBAV",
        6,
        0,
        len(vh) + len(vb),
        0,
        programs,
        1,
        samples,
        127,
        64,
        0,
        0,
        0,
    )
    vh[32] = 1
    struct.pack_into("<3H", vh, lengths_at, 0, 2, 4)
    return VabCase("synthetic", bytes(vh), vb)


class VabStateOracleTests(unittest.TestCase):
    def test_seed_covers_every_runtime_input_with_nonzero_sentinels(self) -> None:
        request = seeded_request(synthetic_vab())

        self.assertEqual(len(request), 16)
        self.assertEqual(len(request[2]), AUDIO_STATE_SIZE)
        self.assertEqual(struct.unpack_from("<I", request[2], 0x10)[0], 0)
        self.assertEqual(request[5][:4], bytes((1, 2, 0, 1)))
        self.assertEqual([len(block) for block in request[6:12]], [POINTER_TABLE_SIZE] * 6)
        self.assertNotEqual(request[6], bytes(POINTER_TABLE_SIZE))
        self.assertEqual(
            struct.unpack("<IIIi", request[-1]),
            (VH_VA, VB_VA, SPU_ALLOCATION, 0),
        )

    def test_shipped_discovery_returns_one_vab_pair_per_floor(self) -> None:
        if not LOCAL_CONFIG.is_file():
            self.skipTest("configured retail directory is required")

        cases = vab_cases(configured_retail_dir())

        self.assertEqual([case.label for case in cases], [f"B{i}/MIXA.DAT" for i in range(1, 6)])
        self.assertTrue(all(case.vh[:4] == b"pBAV" and case.vb for case in cases))

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and CANDIDATE_OBJECT.is_file()
        and DEFAULT_DRIVER.is_file(),
        "local retail image, candidate object, Rust driver, and Unicorn are required",
    )
    def test_one_shipped_bank_matches_retail_candidate_and_rust_state(self) -> None:
        count, retail_steps, candidate_steps = compare_vab_states(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(),
            vab_cases(configured_retail_dir())[:1],
        )

        self.assertEqual(count, 1)
        self.assertGreater(retail_steps, 4_000)
        self.assertEqual(candidate_steps, retail_steps)


if __name__ == "__main__":
    unittest.main()
