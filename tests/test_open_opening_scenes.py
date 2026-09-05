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
    (0x80035820, 0x54), (0x80035874, 4), (0x80035878, 16),
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
            self.assertEqual(start + sum(size for _, size in run), end)
            for (va, size), (following, _) in zip(run, run[1:]):
                self.assertEqual(va + size, following)
        self.assertEqual(sum(size for _, size in DATA), 1048)
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
                    # The old wrapper copied the four-byte retail claim into
                    # the source symbol. SoundRef is actually three bytes;
                    # the following zero still exists, but is not its field.
                    expected_size = (3 if directory == 'objdiff/open/base'
                                     and datum.symbol == 'opening_scene0_sound' else datum.size)
                    self.assertEqual(symbol.size, expected_size)
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
        result = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
        self.assertFalse(result.matches)
        self.assertTrue(any('owned-symbol-layout' in diff.detail
                            and 'opening_scene0_sound' in diff.detail
                            and '"actual_size": 3' in diff.detail for diff in result.diffs))


if __name__ == "__main__":
    unittest.main()
