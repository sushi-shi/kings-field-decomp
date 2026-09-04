from __future__ import annotations

import unittest

from scripts.kf.inventory import load_data_identities, load_function_identities
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class PsxBootstrapInventoryTests(unittest.TestCase):
    def test_bootstrap_owns_padded_paths_and_pointer_table(self) -> None:
        identities = load_data_identities(RETAIL_CONFIG)
        expected = {
            0x80010000: (
                "overlay_path_GAME",
                0x14,
                "global",
                "const char[20]",
            ),
            0x80010014: (
                "overlay_path_OPEN",
                0x14,
                "global",
                "const char[20]",
            ),
            0x80010224: (
                "overlay_path_table",
                0x08,
                "global",
                "const char *[2]",
            ),
        }
        for va, wanted in expected.items():
            row = identities[("PSX.EXE", va)]
            self.assertEqual(
                (row.name, row.size, row.scope, row.datatype),
                wanted,
            )
            self.assertEqual(row.owner, "bootstrap")

    def test_bootstrap_relocations_are_reviewed(self) -> None:
        _fields, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        campaign = [
            row
            for row in rows
            if row["provenance"] == "manual:psx_semantic_bootstrap_paths"
        ]
        self.assertEqual(len(campaign), 3)
        self.assertEqual({row["status"] for row in campaign}, {"reviewed"})
        self.assertEqual(
            [(parse_int(row["site_va"]), row["target_name"]) for row in campaign],
            [
                (0x8001004C, "overlay_path_table"),
                (0x80010224, "overlay_path_OPEN"),
                (0x80010228, "overlay_path_GAME"),
            ],
        )

    def test_main_identity_and_evidence_are_supported(self) -> None:
        identities = load_function_identities(RETAIL_CONFIG)
        row = identities[("PSX.EXE", 0x80010028)]
        self.assertEqual((row.name, row.owner, row.action), ("main", "", ""))
        self.assertEqual(row.signature_confidence, "supported")

        _fields, evidence = read_tsv(
            CONFIG / "evidence/psx_semantic_bootstrap_paths.tsv"
        )
        self.assertEqual(len(evidence), 1)
        self.assertEqual(evidence[0]["current_match"], "100.000000% exact")


if __name__ == "__main__":
    unittest.main()
