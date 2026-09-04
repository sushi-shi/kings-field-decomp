from __future__ import annotations

import struct
import unittest

from scripts.kf.inventory import (
    load_data_identities,
    load_function_identities,
    load_structure_field_identities,
    load_structure_identities,
)
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv


class OpenEntityTraversalTests(unittest.TestCase):
    def test_module_and_shared_owner(self) -> None:
        units = load_manifest().by_name()
        entity = units["open.entity_render"]
        self.assertEqual([function.va for function in entity.functions],
                         [0x80018ECC, 0x800190F4, 0x80019240])
        self.assertEqual(entity.functions[-1].body_size, 0x298)
        item = units["open.item"]
        self.assertEqual([(datum.va, datum.size, datum.symbol) for datum in item.data],
                         [(0x8006DA28, 0x618, "floor_item_state")])
        data = load_data_identities(RETAIL_CONFIG)
        state = data[("OPEN.EXE", 0x8006DA28)]
        self.assertEqual((state.name, state.datatype, state.size, state.storage),
                         ("floor_item_state", "KfFloorItemStateOpen", 0x618, "bss"))
        self.assertFalse(any(image == "OPEN.EXE" and 0x8006DA28 < va < 0x8006E040
                             for image, va in data))
        self.assertIn(("OPEN.EXE", 0x8006E040), data)
        self.assertEqual(load_structure_identities(RETAIL_CONFIG)["KfFloorItemStateOpen"].size,
                         0x618)
        fields = {field.name: (field.offset, field.size, field.datatype)
                  for field in load_structure_field_identities(RETAIL_CONFIG)
                  if field.structure == "KfFloorItemStateOpen"}
        self.assertEqual(fields, {
            "material": (0, 8, "KfSpriteMaterial"), "unknown_08": (8, 6, "u8[6]"),
            "texture_clut": (14, 2, "u16"), "texture_tpage": (16, 2, "u16"),
            "count": (18, 2, "u16"), "unknown_14": (20, 4, "u8[4]"),
            "items": (24, 0x600, "KfFloorItem[64]"),
        })

    def test_reviewed_references_and_evidence(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        body = [row for row in rows if row["image"] == "OPEN.EXE"
                and 0x80019240 <= parse_int(row["site_va"]) < 0x800194D8]
        self.assertEqual(len(body), 19)
        self.assertEqual({row["status"] for row in body}, {"reviewed"})
        self.assertEqual([(parse_int(row["site_va"]), row["target_name"])
                          for row in body if row["opcode"] == "jal"], [
            (0x800192A4, "tmd_select"), (0x80019324, "opening_entity_render"),
            (0x80019354, "SetLightMatrix"), (0x80019488, "render_floor_item"),
        ])
        state_rows = [row for row in rows if row["image"] == "OPEN.EXE"
                      and 0x8006DA28 <= parse_int(row["target_va"]) < 0x8006E040]
        self.assertEqual(len(state_rows), 22)
        self.assertEqual({row["target_name"] for row in state_rows}, {"floor_item_state"})
        self.assertEqual({row["status"] for row in state_rows}, {"reviewed"})
        for row in body + state_rows:
            self.assertIn("manual:open_semantic_entity_traversal", row["provenance"].split(";"))
        _, evidence = read_tsv(CONFIG / "evidence/open_semantic_entity_traversal.tsv")
        self.assertEqual({parse_int(row["va"]) for row in evidence},
                         {0x80019240, 0x80016908, 0x800189A0, 0x800197E4})
        identity = load_function_identities(RETAIL_CONFIG, required=True)[
            ("OPEN.EXE", 0x80019240)]
        self.assertEqual((identity.name, identity.return_type, identity.parameters),
                         ("opening_render_entities_and_items", "void", ""))
        self.assertIn("open_semantic_entity_traversal.tsv", identity.evidence)

    def test_retail_base_chain_divisions_and_light_matrix(self) -> None:
        try:
            retail = configured_retail_dir(validate=False)
        except ValueError:
            self.skipTest("retail files are not configured")
        data = (retail / "OPEN.EXE").read_bytes()
        base = struct.unpack_from("<I", data, 0x18)[0]
        self.assertEqual(base, 0x80012000)

        def word(va: int) -> int:
            return struct.unpack_from("<I", data, va - base + 0x800)[0]

        for va, expected in {
            0x80019378: 0x3C048007, 0x8001937C: 0x2484DA2A,
            0x800193B8: 0x24940016, 0x800193C0: 0x2490001A,
            0x800194AC: 0x26940018, 0x8001932C: 0x26100028,
            0x80019348: 0x26310028, 0x800193BC: 0x341107D0,
            0x800193CC: 0x0051001A, 0x80019420: 0x0051001A,
            0x800194D4: 0x27BD0030,
        }.items():
            with self.subTest(va=hex(va)):
                self.assertEqual(word(va), expected)
        offset = 0x800359E4 - base + 0x800
        self.assertEqual(struct.unpack_from("<9h2x3i", data, offset),
                         (0, 0, 4096, 0, 0, 4096, 0, 0, 0, 0, 0, 0))
        _, coverage = read_tsv(RETAIL_CONFIG / "data.tsv")
        by_va = {parse_int(row["va"]): row for row in coverage if row["image"] == "OPEN.EXE"}
        self.assertEqual(parse_int(by_va[0x800359E4]["size"]), 0x20)
        self.assertEqual(parse_int(by_va[0x80035A04]["size"]), 0x80)


if __name__ == "__main__":
    unittest.main()
