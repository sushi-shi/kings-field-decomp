from __future__ import annotations

import hashlib
import struct
import unittest

from scripts.kf.inventory import (
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class OpenMapRenderTests(unittest.TestCase):
    def test_campaign_contracts_and_reviewed_referents(self) -> None:
        evidence = CONFIG / "evidence/open_semantic_map_render.tsv"
        _, rows = read_tsv(evidence)
        expected = {
            0x80018BBC: ("render_map_cell", "s32 col;s32 row;u8 cell"),
            0x80018D8C: ("opening_render_map_cells", ""),
        }
        self.assertEqual({parse_int(row["va"]) for row in rows}, set(expected))
        functions = load_function_identities(RETAIL_CONFIG, required=True)
        for row in rows:
            va = parse_int(row["va"])
            identity = functions[("OPEN.EXE", va)]
            name, parameters = expected[va]
            self.assertEqual(
                (identity.name, identity.return_type, identity.parameters),
                (name, "void", parameters),
            )
            self.assertIn(evidence.name, identity.evidence)
            self.assertEqual(
                row["final_signature"],
                f"void {name}({parameters.replace(';', ', ') or 'void'})",
            )

        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body_rows = [
            row for row in rows
            if row["image"] == "OPEN.EXE"
            and 0x80018BBC <= parse_int(row["site_va"]) < 0x80018ECC
        ]
        self.assertEqual(len(body_rows), 31)
        self.assertEqual({row["status"] for row in body_rows}, {"reviewed"})
        by_site = {parse_int(row["site_va"]): row for row in body_rows}
        expected_targets = {
            0x80018BE4: (0x800730A0, "map_cell_attribute_grid"),
            0x80018C20: (0x80046DF8, "map_cell_orientation_grid"),
            0x80018C88: (0x8006E260, "map_floor_height_grid"),
            0x80018D2C: (0x8006E0C8, "open_graphics_runtime"),
            0x80018DD4: (0x800439D8, "render_cell_windows"),
            0x80018DE0: (0x8006E1C8, "open_graphics_runtime"),
            0x80018E60: (0x80018BBC, "render_map_cell"),
        }
        for site, expected_target in expected_targets.items():
            row = by_site[site]
            self.assertEqual(
                (parse_int(row["target_va"]), row["target_name"]),
                expected_target,
            )

    def test_window_table_extent_and_shared_fields(self) -> None:
        structures = load_structure_identities(RETAIL_CONFIG)
        self.assertEqual(structures["KfCellWindow"].size, 0xCC)
        fields = {
            row.name: (row.offset, row.size, row.datatype)
            for row in load_structure_field_identities(RETAIL_CONFIG)
            if row.structure == "KfCellWindow"
        }
        self.assertEqual(fields, {
            "width": (0, 2, "u16"),
            "height": (2, 2, "u16"),
            "origin_x": (4, 2, "u16"),
            "origin_z": (6, 2, "u16"),
            "cells": (8, 196, "u8[196]"),
        })
        data = load_data_identities(RETAIL_CONFIG)
        table = data[("OPEN.EXE", 0x800439D8)]
        self.assertEqual(
            (table.name, table.datatype, table.size, table.storage),
            ("render_cell_windows", "KfCellWindow[16]", 16 * 0xCC, "bss"),
        )
        self.assertNotIn(("OPEN.EXE", 0x8006E1C8), data)
        pointer = next(row for row in load_structure_field_identities(RETAIL_CONFIG)
                       if row.structure == "KfGraphicsRuntimeOpen"
                       and row.offset == 0x24780)
        self.assertEqual(
            (pointer.name, pointer.datatype, pointer.size),
            ("active_cell_window", "KfCellWindow *", 4),
        )
        self.assertEqual(
            [va for image, va in data if image == "OPEN.EXE"
             and 0x800439D8 < va < 0x800439D8 + table.size],
            [],
        )

    def test_retail_rtbl_contains_sixteen_complete_windows(self) -> None:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        path = retail / "KF/B0/RTBL."
        if not path.is_file():
            self.skipTest("optional retail RTBL resource is unavailable")
        data = path.read_bytes()
        self.assertEqual(len(data), 16 * 204)
        self.assertEqual(
            hashlib.sha256(data).hexdigest(),
            "b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72",
        )
        for start in range(0, len(data), 204):
            width, height, origin_x, origin_z = struct.unpack_from("<4H", data, start)
            self.assertEqual((width, height), (14, 14))
            self.assertLess(origin_x, width)
            self.assertLess(origin_z, height)
            self.assertLessEqual(set(data[start + 8:start + 204]), {0, 1, 2})


if __name__ == "__main__":
    unittest.main()
