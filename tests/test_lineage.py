from __future__ import annotations

import unittest

from scripts.kf.lineage import (
    _jal_target,
    instruction_shape,
    load_rows,
    signature_directory_from_environment,
    verify_archive_anchors,
    verify_inventory,
)


class OverlayLineageTests(unittest.TestCase):
    def test_jal_target_uses_pc_region_and_instruction_index(self) -> None:
        self.assertEqual(_jal_target(0x8004A598, 0x0C01294B), 0x8004A52C)
        self.assertIsNone(_jal_target(0x8004A598, 0x0801294B))

    def test_instruction_shape_preserves_registers_not_linked_immediates(self) -> None:
        self.assertEqual(
            instruction_shape(0x3C03800A),
            instruction_shape(0x3C038007),
        )
        self.assertEqual(
            instruction_shape(0x0C01294B),
            instruction_shape(0x0C00A8C0),
        )
        self.assertNotEqual(
            instruction_shape(0x3C03800A),
            instruction_shape(0x3C02800A),
        )
        self.assertNotEqual(
            instruction_shape(0x00621821),
            instruction_shape(0x00421821),
        )

    def test_evidence_matches_contiguous_function_inventory(self) -> None:
        rows = load_rows()
        verify_inventory(rows)
        self.assertEqual(len(rows), 167)
        self.assertEqual(
            {row.address_delta for row in rows},
            {
                -0xDD8,
                -0xD98,
                -0xD58,
                -0xD54,
                -0xD2C,
                0x4CDC,
                0x51AC,
                0x5280,
                0x5C9C,
                0x6CDC,
                0x6D64,
                0x201E0,
                0x2022C,
                0x2031C,
            },
        )
        self.assertEqual(sum(row.total_words for row in rows), 8915)
        self.assertEqual(sum(row.shape_words for row in rows), 8915)

    def test_pinned_psyq_corpus_contains_archive_anchors(self) -> None:
        signature_dir = signature_directory_from_environment()
        if signature_dir is None:
            self.skipTest("GHIDRA_PSX_LOADER is unavailable outside the Nix environment")
        rows = load_rows()
        expected = {
            (row.archive_library, row.archive_module, row.archive_symbol)
            for row in rows
            if row.archive_library and row.archive_module
        }
        self.assertEqual(verify_archive_anchors(signature_dir, rows), len(expected))


if __name__ == "__main__":
    unittest.main()
