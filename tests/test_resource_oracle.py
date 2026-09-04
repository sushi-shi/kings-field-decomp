from __future__ import annotations

import unittest

from scripts.kf.resource_oracle import (
    pattern,
    require_equal,
    synthetic_actor_placements,
    synthetic_event_definitions,
    synthetic_object_placements,
)


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


if __name__ == "__main__":
    unittest.main()
