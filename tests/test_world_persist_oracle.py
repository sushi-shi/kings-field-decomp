from __future__ import annotations

import importlib.util
import unittest

from scripts.kf.parser_machine import GameSymbols
from scripts.kf.paths import LOCAL_CONFIG, REPO
from scripts.kf.rust_codec import RustCodec
from scripts.kf.sema.image import RetailImage
from scripts.kf.world_persist_oracle import (
    PERSIST_OBJECT,
    cases,
    compare_world_persist,
)


class WorldPersistOracleTests(unittest.TestCase):
    def test_cases_cover_all_floor_offsets_and_floor_five_capacity(self) -> None:
        selected = cases()
        self.assertEqual([case.floor for case in selected], [1, 2, 3, 4, 5])
        floor_five = selected[-1]
        active_links = sum(
            floor_five.objects[index * 44] != 0xFF for index in range(160)
        )
        self.assertEqual(active_links, 149)
        self.assertEqual(349 + active_links * 9, 1_690)

    def test_mixed_case_exercises_sparse_filters_and_event_tag_indices(self) -> None:
        mixed = cases()[0]
        self.assertEqual(
            [mixed.actors[index * 72] for index in (0, 7, 127)],
            [1, 3, 1],
        )
        self.assertEqual(
            [mixed.events[index * 68 + 9] for index in range(8)],
            [0, 1, 2, 3, 4, 5, 1, 5],
        )
        self.assertEqual(
            [mixed.objects[index * 44 + 40] for index in range(7)],
            [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 9, 0xFF],
        )

    @unittest.skipUnless(
        importlib.util.find_spec("unicorn")
        and LOCAL_CONFIG.is_file()
        and PERSIST_OBJECT.is_file()
        and (REPO / "tools/target/debug/kf-codec-oracle").is_file(),
        "local retail image, candidate object, Rust driver, and Unicorn are required",
    )
    def test_three_way_world_persistence(self) -> None:
        count = compare_world_persist(
            RetailImage.load("GAME.EXE"),
            GameSymbols.load(),
            RustCodec(REPO / "tools/target/debug/kf-codec-oracle"),
        )
        self.assertEqual(count, 5)


if __name__ == "__main__":
    unittest.main()
