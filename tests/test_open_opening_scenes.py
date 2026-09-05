"""Opening-scene ownership without assuming a gap-free retail data section."""

from __future__ import annotations

import unittest

from scripts.kf.data_match import Elf, diff_unit
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import parse_int, read_tsv
from scripts.kf.sema.image import RetailImage


FUNCTIONS = (
    (0x80014268, 0x174), (0x800143DC, 0x180), (0x8001455C, 0xAC),
    (0x80014608, 0x1FC), (0x80014804, 0x330), (0x80014B34, 0x2F4),
    (0x80014E28, 0x798),
)
DATA = (
    (0x800354F4, 0x1DC), (0x800356D0, 0x54), (0x80035724, 0xFC),
    (0x80035820, 0x54), (0x80035874, 3), (0x80035878, 16),
    (0x80035888, 72), (0x80037284, 8), (0x8003728C, 4),
    (0x80037290, 16), (0x800372A0, 4), (0x800372A4, 4),
    (0x800372A8, 4), (0x800372AC, 4), (0x800372B0, 4),
    (0x800372B4, 4), (0x800372B8, 8),
)


class OpenOpeningScenesTests(unittest.TestCase):
    def image(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        return RetailImage.load("OPEN.EXE")

    def test_contiguous_game_functions_keep_each_boundary(self):
        manifest = load()
        unit = manifest.by_name()["open.opening_scenes"]
        self.assertEqual(unit.image, "OPEN.EXE")
        self.assertEqual(unit.profile, "probe-gcc257-o2-g0")
        self.assertEqual(tuple((fn.va, fn.body_size) for fn in unit.functions), FUNCTIONS)
        self.assertEqual(unit.va + sum(size for _, size in FUNCTIONS), 0x800155C0)
        for (va, size), (following, _) in zip(FUNCTIONS, FUNCTIONS[1:]):
            self.assertEqual(va + size, following)
        for va in (0x80014100, 0x800155C0, 0x800156BC):
            self.assertNotEqual(manifest.by_identity()["OPEN.EXE", va].unit, unit.unit)
        vendored = read_tsv(RETAIL_CONFIG / "functions_vendored.tsv")[1]
        self.assertFalse({parse_int(row["va"]) for row in vendored
                          if row["image"] == "OPEN.EXE"} & {va for va, _ in FUNCTIONS})

    def test_data_claims_retain_two_runs_and_original_linkages(self):
        unit = load().by_name()["open.opening_scenes"]
        self.assertEqual(tuple((d.va, d.size) for d in unit.data), DATA)
        self.assertEqual(unit.rodata, (0x80012000, 32))
        self.assertEqual({d.storage for d in unit.data}, {"load"})
        self.assertEqual([d.scope for d in unit.data],
                         ["global"] * 3 + ["static"] + ["global"] * 2
                         + ["static"] + ["global"] * 2 + ["static"] * 8)
        for run, start, end in ((DATA[:7], 0x800354F4, 0x800358D0),
                                (DATA[7:], 0x80037284, 0x800372C0)):
            self.assertEqual(run[0][0], start)
            gaps = 1 if start == 0x800354F4 else 0
            self.assertEqual(start + sum(size for _, size in run) + gaps, end)
            for (va, size), (following, _) in zip(run, run[1:]):
                self.assertEqual(following - (va + size), 1 if va == 0x80035874 else 0)
        self.assertEqual(sum(size for _, size in DATA), 1047)
        # These bytes belong to other objects; never manufacture a padding claim.
        self.assertEqual(DATA[7][0] - (DATA[6][0] + DATA[6][1]), 0x19B4)

    def test_initializer_payloads_preserve_the_independently_measured_sound_size(self):
        unit = load().by_name()["open.opening_scenes"]
        image = self.image()
        for directory in ("objdiff/open/base", "delink/open/modules"):
            path = BUILD / directory / unit.object_name
            if not path.is_file():
                self.skipTest("compiled and delinked opening-scene objects are required")
            obj = Elf(path)
            allocations = {symbol.name: symbol for symbol in obj.allocations[".data"]}
            self.assertEqual(set(allocations), {d.symbol for d in unit.data})
            bases = set()
            for datum in unit.data:
                with self.subTest(object=directory, datum=datum.symbol):
                    symbol = allocations[datum.symbol]
                    self.assertEqual(symbol.size, datum.size)
                    self.assertEqual(symbol.binding, 0 if datum.scope == "static" else 1)
                    self.assertEqual(symbol.visibility, 0)
                    payload = obj.sections[".data"].data
                    self.assertEqual(payload[symbol.offset:symbol.offset + datum.size],
                                     image.require(datum.va, datum.size))
                    bases.add(datum.va - symbol.offset)
            # Per-object bytes can agree while whole-section placement is impossible.
            self.assertEqual(bases, {0x800354F4, 0x80036EA8})
            self.assertFalse(obj.relocations(".data"))
            self.assertEqual(obj.sections[".rodata"].data, image.require(0x80012000, 32))
            self.assertEqual(obj.sections['.data'].data,
                             image.require(0x800354F4, 988) + image.require(0x80037284, 60))
            self.assertEqual(obj.sections['.data'].data[899:900], image.require(0x80035877, 1))
        result = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
        self.assertFalse(result.matches)
        self.assertTrue(any('conflicting-section-bases' in diff.detail for diff in result.diffs))
        self.assertFalse(any('owned-symbol-layout' in diff.detail for diff in result.diffs))

    def test_sound_owner_and_gap_partition_without_an_interior_identity(self):
        from scripts.kf.inventory import load_data_identities

        identities = load_data_identities(RETAIL_CONFIG)
        self.assertEqual(identities['OPEN.EXE', 0x80035874].size, 3)
        self.assertNotIn(('OPEN.EXE', 0x80035877), identities)
        census = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]
                  if r['image'] == 'OPEN.EXE' and 0x80035874 <= int(r['va'], 0) < 0x80035878]
        self.assertEqual([(int(r['va'], 0), int(r['size'], 0), r['kind']) for r in census],
                         [(0x80035874, 3, 'data'), (0x80035877, 1, 'unclassified')])

    def test_sound_consumer_uses_three_bytes_and_preserves_the_original_address_pair(self):
        from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target

        image = self.image()
        self.assertEqual(image.require(0x80035874, 4), b'\x09\0\x43\0')
        self.assertEqual([image.u32(site) for site in (0x8001A234, 0x8001A238, 0x8001A23C)],
                         [0x90830000, 0x90860001, 0x90870002])
        self.assertEqual(decode_hi_lo_target(image.u32(0x800142FC), image.u32(0x80014300)),
                         0x80035874)
        self.assertEqual(decode_mips26_target(0x80014304, image.u32(0x80014304)), 0x8001A220)
        self.assertEqual(image.u32(0x80014308), 0x34050064)


if __name__ == "__main__":
    unittest.main()
