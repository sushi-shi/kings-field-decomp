from __future__ import annotations

import unittest

from scripts.kf.fid_census import (
    FidSignature,
    RetailFunction,
    classify_hits,
    find_hits,
    signatures_from_listing,
)
from scripts.kf.seed_vendored_functions import parse_object_symbols
from scripts.kf.seed_vendored_functions import apply_fid_rows


LISTING = """
16 : Section symbol number 10 '.text' in group 0 alignment 8
6 : Switch to section 10
2 : Code 32 bytes
0000: 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00
0010: 10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00
10 : Patch type 74 at offset 4 with [20]
12 : XDEF symbol number 20 'first' at offset 0 in section 10
18 : Local symbol 'second' at offset 10 in section 10
74 : Function start :
  section 0010
  offset $00000000
  file 1
  start line 1
  frame reg 29
  frame size 0
  return pc reg 31
  mask $00000000
  mask offset 0
  name first
76 : Function end :
  section 0010
  offset $00000010
  end line 1
0 : End of file
"""


def signature(name: str, data: bytes, mask: bytes | None = None) -> FidSignature:
    return FidSignature(
        "TEST.LIB", f"{name}.OBJ", 0, data,
        mask if mask is not None else b"\xff" * len(data),
        (name,), "debug-function", "a" * 64, "b" * 64,
    )


class FidCorpusTest(unittest.TestCase):
    def test_debug_and_local_boundaries_split_object(self):
        symbols = parse_object_symbols(LISTING)
        rows = signatures_from_listing(
            LISTING,
            symbols,
            library="TEST.LIB",
            module="PAIR",
            library_sha256="a" * 64,
            object_sha256="b" * 64,
        )
        self.assertEqual(
            [(row.member_offset, row.size, row.names, row.boundary) for row in rows],
            [
                (0, 16, ("first",), "debug-function"),
                (16, 16, ("second",), "symbol-object-end"),
            ],
        )
        self.assertEqual(rows[0].mask[4:8], bytes.fromhex("000000fc"))

    def test_relocation_mask_and_overlay_identity(self):
        reference = bytes.fromhex("01020304 aabbccdd 11121314 21222324")
        linked = bytes.fromhex("01020304 99887766 11121314 21222324")
        mask = b"\xff" * 4 + b"\x00" * 4 + b"\xff" * 8
        item = signature("sdk_fn", reference, mask)
        functions = [
            RetailFunction("GAME.EXE", 0x80010000, 16, 16, "", ()),
            RetailFunction("OPEN.EXE", 0x80020000, 16, 16, "", ()),
        ]
        images = {
            "GAME.EXE": (0x80010000, linked),
            "OPEN.EXE": (0x80020000, linked),
        }
        rows = classify_hits(find_hits([item], functions, images))
        self.assertEqual([row["confidence"] for row in rows], ["HIGH", "HIGH"])
        self.assertTrue(all(row["identity_image_matches"] == "1" for row in rows))

    def test_collision_is_not_high(self):
        body = bytes(range(16))
        functions = [RetailFunction("GAME.EXE", 0x80010000, 16, 16, "", ())]
        images = {"GAME.EXE": (0x80010000, body)}
        rows = classify_hits(find_hits(
            [signature("first", body), signature("second", body)],
            functions,
            images,
        ))
        self.assertEqual({row["confidence"] for row in rows}, {"AMBIG"})
        self.assertTrue(all(row["va_identities"] == "2" for row in rows))

    def test_short_ambiguous_fid_stays_report_only(self):
        base = {
            "image": "GAME.EXE",
            "va": "0x80010000",
            "size": "0x10",
            "name": "sdk_fn",
            "aliases": "",
            "library": "TEST.LIB",
            "module": "TEST",
            "member_offset": "0x0",
            "boundary": "debug-function",
            "fixed_bits": "128",
            "identity_image_matches": "1",
            "va_identities": "1",
            "library_sha256": "a" * 64,
            "object_sha256": "b" * 64,
            "fid_sha256": "c" * 64,
            "notes": "-",
        }
        providers = {}
        apply_fid_rows([
            {**base, "confidence": "AMBIG"},
            {**base, "va": "0x80010010", "confidence": "HIGH"},
        ], providers)
        self.assertEqual(list(providers), [("GAME.EXE", 0x80010010)])
        self.assertEqual(
            providers[("GAME.EXE", 0x80010010)]["confidence"],
            "fid-release25",
        )

    def test_full_byte_same_name_ambiguity_preserves_all_candidates(self):
        base = {
            "image": "GAME.EXE",
            "va": "0x80010000",
            "size": "0x30",
            "aliases": "",
            "member_offset": "0x0",
            "boundary": "symbol-next",
            "fixed_bits": "384",
            "identity_image_matches": "4",
            "va_identities": "2",
            "library_sha256": "a" * 64,
            "fid_sha256": "c" * 64,
            "notes": "va_multiidentity=2",
            "confidence": "AMBIG",
        }
        providers = {}
        apply_fid_rows([
            {
                **base,
                "name": "memcpy",
                "library": "FIRST.LIB",
                "module": "ONE",
                "object_sha256": "1" * 64,
            },
            {
                **base,
                "name": "_memcpy",
                "library": "SECOND.LIB",
                "module": "TWO",
                "object_sha256": "2" * 64,
            },
        ], providers)
        row = providers[("GAME.EXE", 0x80010000)]
        self.assertEqual(row["name"], "memcpy")
        self.assertEqual(row["aliases"], "_memcpy")
        self.assertEqual(row["confidence"], "fid-release25-ambiguous")
        self.assertEqual(row["library"], "FIRST.LIB|SECOND.LIB")

    def test_fid_only_corroborates_the_same_archive_identity(self):
        existing = {
            ("GAME.EXE", 0x80010000): {
                "library": "OTHER.LIB",
                "module": "OTHER",
                "provenance": "exact-object",
                "note": "exact object",
            },
        }
        apply_fid_rows([{
            "image": "GAME.EXE",
            "va": "0x80010000",
            "size": "0x10",
            "name": "sdk_fn",
            "aliases": "",
            "library": "TEST.LIB",
            "module": "TEST",
            "member_offset": "0x0",
            "boundary": "debug-function",
            "fixed_bits": "128",
            "object_sha256": "b" * 64,
            "fid_sha256": "c" * 64,
            "confidence": "HIGH",
        }], existing)
        self.assertEqual(
            existing[("GAME.EXE", 0x80010000)]["provenance"],
            "exact-object",
        )


if __name__ == "__main__":
    unittest.main()
