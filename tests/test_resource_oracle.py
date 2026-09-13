from __future__ import annotations

import importlib.util
import unittest

from scripts.kf.local_config import configured_retail_dir
from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import BUILD, LOCAL_CONFIG
from scripts.kf.resource_oracle import (
    compare_records,
    compare_stat,
    pattern,
    require_equal,
    synthetic_actor_placements,
    synthetic_event_definitions,
    synthetic_object_placements,
)
from scripts.kf.rust_codec import DEFAULT_DRIVER, RustCodec
from scripts.kf.sema.image import RetailImage


class ResourceAgreementTests(unittest.TestCase):
    def test_accepts_only_three_equal_outputs(self) -> None:
        require_equal("equal", b"\x01\0", b"\x01\0", b"\x01\0")
        with self.assertRaisesRegex(AssertionError, "retail/C mismatch at byte 0x1"):
            require_equal("candidate", b"\x01\0", b"\x01\xff", b"\x01\0")
        with self.assertRaisesRegex(AssertionError, "retail/Rust mismatch at byte 0x1"):
            require_equal("rust", b"\x01\0", b"\x01\0", b"\x01\xff")

    def test_detects_output_length_and_untouched_tail_changes(self) -> None:
        with self.assertRaisesRegex(AssertionError, "lengths 2/1"):
            require_equal("truncated", b"ab", b"ab", b"a")
        with self.assertRaisesRegex(AssertionError, "byte 0x2"):
            require_equal("tail", b"abc", b"abd", b"abc")

    def test_initial_memory_is_nonuniform_and_seeded(self) -> None:
        first = pattern(1024, 1)
        self.assertEqual(len(first), 1024)
        self.assertGreater(len(set(first)), 200)
        self.assertNotEqual(first, pattern(1024, 2))

    def test_full_capacity_controls_have_no_early_sentinel(self) -> None:
        actors = synthetic_actor_placements()
        objects = synthetic_object_placements(1, 190)
        events = synthetic_event_definitions()
        self.assertEqual(len(actors), 128 * 16)
        self.assertEqual(len(objects), 190 * 20)
        self.assertEqual(len(events), 8 * 24)
        self.assertTrue(all(actors[index * 16] != 0xFF for index in range(128)))
        self.assertTrue(all(objects[index * 20] != 0xFF for index in range(190)))
        self.assertTrue(all(events[index * 24] != 0xFF for index in range(8)))

    def test_object_123_control_names_the_unshipped_branch(self) -> None:
        source = synthetic_object_placements(123, 1) + b"\xff"
        self.assertEqual(source[0], 123)
        self.assertEqual(source[20], 0xFF)


@unittest.skipUnless(
    importlib.util.find_spec("unicorn")
    and LOCAL_CONFIG.is_file()
    and DEFAULT_DRIVER.is_file()
    and all((BUILD / "objdiff/game/base" / name).is_file() for name in (
        "8001b100_resources.o", "800150a8_equipment.o", "8003a244_magic.o",
        "80030a98_map_object_pool.o", "80030818_actor_pool.o", "80020b4c_item.o",
    )),
    "local retail, native candidates, Rust driver and Unicorn are required",
)
class ResourceFixtureIntegrationTests(unittest.TestCase):
    def test_record_loaders_bind_native_objects_and_match_all_bytes(self) -> None:
        count = compare_records(
            RetailImage.load("GAME.EXE"), GameSymbols.load(), RustCodec(),
            configured_retail_dir(),
        )
        self.assertEqual(count, 17)

    def test_stat_loader_uses_the_current_menu_owner(self) -> None:
        count = compare_stat(
            RetailImage.load("GAME.EXE"), GameSymbols.load(), RustCodec(),
            configured_retail_dir(), source_alignment=1,
        )
        self.assertEqual(count, 1)


if __name__ == "__main__":
    unittest.main()
