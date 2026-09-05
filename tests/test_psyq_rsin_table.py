"""SDK and retail controls for complete sine-table ownership and biased bases."""

from __future__ import annotations

from functools import lru_cache
import hashlib
import os
from pathlib import Path
import re
import shutil
import struct
import tempfile
import unittest

from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend, encode_mips26_addend
from scripts.kf.retail import read_tsv, validate_config
from scripts.kf.seed_vendored_functions import parse_object_symbols, run_psyk
from scripts.kf.sema import Context
from scripts.kf.sema.xref import _row as xref_row


CASES = (("GAME.EXE", 0x80057070, 0x8004F27C),
         ("OPEN.EXE", 0x800367E8, 0x8002F050))
TABLE_HASH = "74743e361fc4d78cbd41bd99b2acf5c75c9b5b0268ccd753ed282ec4394fb25a"
FUNCTIONS = (("rsin", 0x930, 0x50), ("sin_1", 0x980, 0xB8), ("rcos", 0xA38, 0xD4))
PATCHES = ((0x98C, 0), (0x9BC, 0), (0x9E4, -0x1000), (0xA18, 0),
           (0xA70, 0), (0xA98, -0x800), (0xAC8, 0), (0xAF4, -0x1800))


@lru_cache(maxsize=1)
def sdk_geo():
    directory, tool = os.environ.get("PSYQ_LIB"), shutil.which("psyk")
    if directory is None or tool is None:
        raise unittest.SkipTest("pinned Psy-Q archive tools are required")
    archive = Path(directory) / "LIBGTE.LIB"
    with tempfile.TemporaryDirectory(prefix="kf-rsin-table-") as temporary:
        root = Path(temporary)
        run_psyk(Path(tool), "extract", str(archive), cwd=root)
        obj = root / "GEO.OBJ"
        digest = hashlib.sha256(obj.read_bytes()).hexdigest()
        listing = run_psyk(Path(tool), "list", "--code", str(obj))
    sections = {name: number for number, name in re.findall(
        r"Section symbol number ([0-9a-f]+) '([^']+)'", listing)}
    match = re.search(rf"6 : Switch to section {sections['.data']}\n"
                      r"2 : Code ([0-9]+) bytes\n(.*?)\n6 : Switch", listing, re.S)
    if match is None:
        raise AssertionError("missing GEO data contribution")
    payload = bytes.fromhex(" ".join(re.findall(
        r"^[0-9a-f]+: ([0-9a-f ]+)$", match.group(2), re.M)))
    if len(payload) != int(match.group(1)):
        raise AssertionError("truncated SDK data listing")
    return listing, sections, payload, digest


class PsyqRsinTableTests(unittest.TestCase):
    def contexts(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        return {name: Context(name) for name, _, _ in CASES}

    def test_inventory_owns_complete_tables_and_retains_sdk_classification(self):
        identities = load_data_identities(RETAIL_CONFIG)
        census = read_tsv(RETAIL_CONFIG / "data.tsv")[1]
        vendored = {(r["image"], int(r["va"], 0)): r for r in
                    read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")[1]}
        for image, va, rsin in CASES:
            identity = identities[image, va]
            self.assertEqual((identity.name, identity.size, identity.datatype,
                              identity.scope, identity.storage),
                             ("rsin_tbl", 2048, "s16[1024]", "global", "load"))
            overlapping = [row for row in census if row["image"] == image
                           and int(row["va"], 0) < va + 2048
                           and va < int(row["va"], 0) + int(row["size"], 0)]
            self.assertEqual(len(overlapping), 1)
            self.assertEqual((overlapping[0]["kind"], overlapping[0]["confidence"]),
                             ("array", "reviewed"))
            for name, offset, size in FUNCTIONS:
                row = vendored[image, rsin - 0x930 + offset]
                self.assertEqual((row["name"], int(row["size"], 0), row["module"]),
                                 (name, size, "GEO"))
        validate_config(RETAIL_CONFIG)

    def test_sdk_exports_full_table_and_explicit_negative_linker_addends(self):
        listing, sections, payload, digest = sdk_geo()
        self.assertEqual(digest, "7e62d2e92b7a30001549db9b60abaed0811db7db2ed213e3eca2e62d8a7db36e")
        self.assertEqual((len(payload), hashlib.sha256(payload).hexdigest()),
                         (2048, TABLE_HASH))
        self.assertRegex(listing, rf"XDEF symbol number [0-9a-f]+ 'rsin_tbl' "
                         rf"at offset 0 in section {sections['.data']}")
        self.assertIn(f"Section symbol number {sections['.data']} '.data' in group 0 alignment 8",
                      listing)
        for site, addend in PATCHES:
            expression = f"(sectbase({sections['.data']})+${addend & 0xFFFFFFFF:x})"
            self.assertIn(f"Patch type 82 at offset {site:x} with {expression}", listing)
            self.assertIn(f"Patch type 84 at offset {site + 4:x} with {expression}", listing)

    def test_sdk_payload_and_unmasked_relocated_consumers_equal_both_retail_images(self):
        contexts = self.contexts()
        listing, sections, payload, _digest = sdk_geo()
        text = parse_object_symbols(listing).text_data
        patches = re.findall(r"Patch type (\d+) at offset ([0-9a-f]+) with "
                             r"\(sectbase\(([0-9a-f]+)\)\+\$([0-9a-f]+)\)", listing)
        for image, va, rsin in CASES:
            img = contexts[image].img
            self.assertEqual(img.require(va, 2048), payload)
            text_base = rsin - 0x930
            bases = {sections[".text"]: text_base, sections[".data"]: va}
            for name, offset, size in FUNCTIONS:
                body = bytearray(text[offset:offset + size])
                for kind, site, section, addend in patches:
                    site, kind = int(site, 16), int(kind)
                    if not offset <= site < offset + size:
                        continue
                    value = (bases[section] + int(addend, 16)) & 0xFFFFFFFF
                    word = struct.unpack_from("<I", body, site - offset)[0]
                    if kind == 74:
                        word = encode_mips26_addend(word, value)
                    elif kind in (82, 84):
                        immediate = (value + 0x8000) >> 16 if kind == 82 else value
                        word = (word & 0xFFFF0000) | (immediate & 0xFFFF)
                    else:
                        self.fail(f"unsupported in-scope SDK patch {kind}")
                    struct.pack_into("<I", body, site - offset, word)
                self.assertEqual(body, img.require(text_base + offset, size), (image, name))

    def test_reviewed_referents_delink_and_restore_every_address_pair(self):
        contexts = self.contexts()
        catalog = load_catalog(RETAIL_CONFIG)
        rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")[1]
        for image, va, rsin in CASES:
            ctx = contexts[image]
            references = ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
            self.assertEqual([r.site for r in references],
                             [rsin - 0x930 + site for site, _ in PATCHES])
            for reference, (offset, addend) in zip(references, PATCHES, strict=True):
                self.assertEqual(reference.target, va + addend)
                self.assertEqual(reference.destination, va)
                self.assertEqual(reference.referent.name, "rsin_tbl")
                rendered = xref_row(ctx, reference, outgoing=True, raw=True)
                self.assertEqual(rendered["addend"], addend)
                self.assertEqual(rendered["endpoint"]["va"], va)
                self.assertTrue(rendered["target_label"].startswith("rsin_tbl"))
                row = next(r for r in rows if r["image"] == image
                           and int(r["site_va"], 0) == reference.site)
                self.assertEqual(row["status"], "reviewed")
                owner = ctx.idx.function_owner(reference.site)
                function = catalog.function_starts[image][owner.va]
                body = bytearray(ctx.img.require(owner.va, owner.body_size))
                relocs, used = _apply_relocation(body, function, row, catalog, "safe")
                self.assertEqual([r.symbol for r in relocs], ["rsin_tbl"] * 2)
                self.assertEqual(int(used["addend"], 0), addend)
                position = reference.site - owner.va
                high, low = struct.unpack_from("<2I", body, position)
                self.assertEqual(decode_hi_lo_target(high, low), addend & 0xFFFFFFFF)
                restored = encode_hi_lo_addend(high, low, va + addend)
                self.assertEqual(struct.pack("<2I", *restored), ctx.img.require(reference.site, 8))
            # A biased base equal to unrelated code/data does not make it an owner.
            negative = next(r for r in references if r.target == va - 0x1000)
            wrong = ctx.idx.covering(negative.target)
            self.assertNotIn(negative, ctx.refs.incoming(wrong, confirmed_only=True))


if __name__ == "__main__":
    unittest.main()
