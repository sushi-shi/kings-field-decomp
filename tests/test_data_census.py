"""Reviewed instruction-byte rejections must not return as phantom data owners."""

from __future__ import annotations

from collections import Counter
import unittest

from scripts.kf.data_reachability import data_extents
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.paths import CONFIG, RETAIL_CONFIG
from scripts.kf.retail import read_tsv, validate_config
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import index
from scripts.kf.sema.mips import decode_control


REJECTIONS = CONFIG / "evidence" / "data_census_instruction_overlaps.tsv"


class DataCensusTests(unittest.TestCase):
    def setUp(self):
        _, self.rows = read_tsv(REJECTIONS)

    def test_reviewed_scan_rows_have_code_owners_not_data_claims(self):
        self.assertEqual(Counter(r["image"] for r in self.rows),
                         {"GAME.EXE": 27, "OPEN.EXE": 4})
        self.assertEqual(len({(r["image"], r["va"]) for r in self.rows}), 31)
        extents = data_extents(load())
        for row in self.rows:
            with self.subTest(image=row["image"], va=row["va"]):
                va, size = int(row["va"], 0), int(row["size"], 0)
                self.assertEqual((row["kind"], row["confidence"], row["provenance"]),
                                 ("string", "byte-pattern", "investigation:string-census"))
                idx = index(row["image"])
                owners = tuple(idx.function(int(v, 0)) for v in row["owner_vas"].split(";"))
                for owner in owners:
                    self.assertIsNotNone(owner)
                    self.assertEqual(owner.fragments, 1)
                for byte in range(va, va + size):
                    self.assertEqual(sum(f.contains_body(byte) for f in owners), 1)
                self.assertFalse(any(d.image == row["image"] and d.va < va + size
                                     and va < d.end for d in extents))
                # Rejected rows are evidence only. The SDK bodies retain their
                # vendor ownership and never become game reconstruction work.
                if len(owners) == 2:
                    self.assertEqual([f.name for f in owners],
                                     ["note2pitch", "SpuVmSelectToneAndVag"])
                    self.assertTrue(all(f.vendored for f in owners))

    def test_removing_scan_duplicates_preserves_complete_payload_accounting(self):
        # validate_config checks the full code/data union for all three retail
        # payloads, not just these tiny rejected ranges.
        validate_config(RETAIL_CONFIG)

    def test_reviewed_instruction_windows_match_verified_retail(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        images = {name: RetailImage.load(name) for name in {r["image"] for r in self.rows}}
        for row in self.rows:
            with self.subTest(image=row["image"], va=row["va"]):
                words = b"".join(
                    int(word, 16).to_bytes(4, "little")
                    for word in row["instruction_words"].split()
                )
                start = int(row["instruction_va"], 0)
                va, size = int(row["va"], 0), int(row["size"], 0)
                self.assertLessEqual(start, va)
                self.assertGreaterEqual(start + len(words), va + size)
                self.assertEqual(images[row["image"]].require(start, len(words)), words)

    def test_control_and_switch_references_survive_the_census_correction(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        contexts = {image: Context(image) for image in ("GAME.EXE", "OPEN.EXE")}
        for image, site, target, tier in (
            ("GAME.EXE", 0x80014960, 0x8001482C, "validated"),
            ("GAME.EXE", 0x800181B8, 0x80018128, "validated"),
            ("GAME.EXE", 0x800182B8, 0x80018250, "validated"),
            ("GAME.EXE", 0x8001E678, 0x8001E6AC, "validated"),
            ("GAME.EXE", 0x80012824, 0x80031244, "candidate"),
            ("GAME.EXE", 0x80043AAC, 0x80042E5C, "proven"),
            ("OPEN.EXE", 0x800238CC, 0x80022C7C, "proven"),
        ):
            with self.subTest(image=image, site=hex(site)):
                ctx = contexts[image]
                refs = [r for r in ctx.refs.at_site(site) if r.target == target]
                self.assertEqual([r.tier for r in refs], [tier])
                if refs[0].kind == "pointer":
                    self.assertEqual(ctx.img.u32(site), target)
                else:
                    self.assertEqual(decode_control(site, ctx.img.u32(site)).target, target)
                self.assertIsNotNone(ctx.idx.function_owner(target))
        for image, site in (("GAME.EXE", 0x80042E54), ("OPEN.EXE", 0x80022C74)):
            ctx = contexts[image]
            self.assertEqual(ctx.img.u32(site), 0x03E00008)  # jr ra
            self.assertEqual(ctx.img.u32(site + 4), 0x3062FFFF)  # andi return delay slot


if __name__ == "__main__":
    unittest.main()
