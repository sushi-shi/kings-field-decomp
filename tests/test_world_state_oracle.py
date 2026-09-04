from __future__ import annotations

import importlib.util
import struct
import unittest

from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import LOCAL_CONFIG, REPO
from scripts.kf.rust_codec import RustCodec
from scripts.kf.sema.image import RetailImage
from scripts.kf.world_state_oracle import (
    RANDOM_SEED,
    RESTORE_OBJECT,
    Conditions,
    RestoreServices,
    _cases,
    compare_world_state,
    synthetic_record,
)


class FakeHookContext:
    def __init__(self, *args: int) -> None:
        self.args = args


class WorldStateOracleTests(unittest.TestCase):
    def test_synthetic_record_has_exact_consumed_layout(self) -> None:
        record = synthetic_record()
        self.assertEqual(len(record), 362)
        self.assertEqual(record[:8], bytes((1, 1, 9, 1, 2, 0x30, 3, 4)))
        self.assertEqual(record[57:62], bytes((2, 3, 7, 127, 8)))
        object_ids = 62
        self.assertEqual(record[object_ids + 170:object_ids + 173], bytes((42, 45, 50)))
        self.assertEqual(synthetic_record(False), b"\0")
        empty = synthetic_record(sparse=False)
        self.assertEqual(len(empty), 349)
        self.assertEqual(empty[57], 0)
        self.assertEqual(empty[58 + 190], 0)

    def test_conditions_have_stable_driver_order(self) -> None:
        conditions = Conditions(*range(11), found_actor=17)
        self.assertEqual(conditions.codec_bytes(), bytes(range(11)))
        self.assertEqual(conditions.found_actor, 17)

    def test_deterministic_services_trace_callbacks_and_rng(self) -> None:
        services = RestoreServices(found_actor=9)
        first = services.rand(FakeHookContext())
        expected_seed = (RANDOM_SEED * 1_103_515_245 + 12_345) & 0xFFFFFFFF
        self.assertEqual(first, (expected_seed >> 16) & 0x7FFF)
        services.copy_region(FakeHookContext(4))
        self.assertEqual(services.find_actor(FakeHookContext(7, 40)), 9)
        services.clear_link(FakeHookContext(0x34))
        services.trigger_link(FakeHookContext(0x0D))
        services.actor_death(FakeHookContext(3))
        self.assertEqual(
            bytes(services.trace),
            b"\x01" + struct.pack("<i", first)
            + bytes((2, 4, 3, 7, 40)) + struct.pack("<i", 9)
            + bytes((4, 0x34, 5, 0x0D, 6)) + struct.pack("<H", 3),
        )

    def test_cases_cover_every_floor_and_both_floor_five_paths(self) -> None:
        cases = _cases()
        self.assertEqual({case[2].current_floor for case in cases}, {1, 2, 3, 4, 5})
        floor_five = [case for case in cases if case[2].current_floor == 5]
        self.assertEqual({case[2].flag_8009f846 for case in floor_five}, {0, 1})

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and RESTORE_OBJECT.is_file()
        and (REPO / "tools/target/debug/kf-codec-oracle").is_file(),
        "local retail image, candidate object, Rust driver, and Unicorn are required",
    )
    def test_three_way_floor_restore(self) -> None:
        count = compare_world_state(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(REPO / "tools/target/debug/kf-codec-oracle"),
        )
        self.assertEqual(count, 15)


if __name__ == "__main__":
    unittest.main()
