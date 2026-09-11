"""Retail-backed reference and compiled-word controls for GAME's effect pool."""

from __future__ import annotations

import os
import shutil
import struct
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import validate_relocation
from scripts.kf.retail import read_tsv
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.mips import decode_control
from tests.test_open_runtime_owner_probe import linked_words


START, SIZE, TABLE = 0x80036F44, 0x82C, 0x80012C28
JUMPS = {
    0x80037064: 0x800376F4, 0x800370A0: 0x800376F8,
    0x800370B8: 0x800370CC, 0x80037104: 0x8003762C,
    0x8003711C: 0x80037130, 0x80037150: 0x8003762C,
    0x80037168: 0x80037178, 0x80037184: 0x80037748,
    0x800371D4: 0x8003774C, 0x800371F8: 0x80037748,
    0x80037240: 0x8003774C, 0x8003729C: 0x8003774C,
    0x800372C4: 0x800376F4, 0x800372EC: 0x800376F4,
    0x80037330: 0x80037748, 0x8003733C: 0x8003734C,
    0x8003737C: 0x8003774C, 0x800373DC: 0x800376F8,
    0x8003743C: 0x800376F8, 0x8003749C: 0x80037748,
    0x800374EC: 0x80037748, 0x80037540: 0x80037748,
    0x800375B4: 0x8003774C, 0x800375E0: 0x80037748,
    0x800375F8: 0x8003760C, 0x8003763C: 0x8003774C,
    0x80037654: 0x80037668, 0x80037700: 0x8003774C,
    0x80037738: 0x80037748,
}
CALLS = {
    0x80036F74: 0x80036F00, 0x800371CC: 0x800330AC,
    0x80037238: 0x80032FB8, 0x80037294: 0x80032FB8,
    0x800375AC: 0x80032FE8, 0x80037634: 0x80032FE8,
    0x800376F8: 0x80032FB8,
}
# One free-slot call and six sound calls: seven direct call sites in all.
PAIRS = {
    0x80036FFC: 0x8009CE60, 0x8003702C: TABLE,
    0x800370FC: 0x8009CEB2, 0x80037148: 0x8009CEB5,
    0x80037558: 0x8009CFCD, 0x80037624: 0x8009CFCA,
    0x800376EC: 0x8009CFF2, 0x8003772C: 0x800A083A,
}
# Actual table destinations by kind; all unlisted kinds use the default arm.
CASES = {
    4: 0x800370C0, 5: 0x80037048, 6: 0x8003718C, 7: 0x8003706C,
    8: 0x80037384, 9: 0x80037200, 10: 0x80037248, 11: 0x800372A4,
    12: 0x800372CC, 13: 0x800372F4, 14: 0x80037338, 15: 0x80037444,
    16: 0x800374A4, 17: 0x800374F4, 18: 0x80037600, 19: 0x800375BC,
    20: 0x8003765C, 21: 0x80037708, 22: 0x800373E4, 23: 0x800370A8,
    24: 0x80037644, 32: 0x80037124, 33: 0x80037170, 34: 0x800371DC,
    36: 0x80037548, 41: 0x8003710C, 42: 0x80037158, 44: 0x800375E8,
    48: 0x80037344,
}


class EffectConstructorInventoryTests(unittest.TestCase):
    def setUp(self):
        try:
            self.image = RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked GAME retail is required')
        self.catalog = load_catalog(RETAIL_CONFIG)
        self.function = self.catalog.function_starts['GAME.EXE'][START]
        self.blob = self.image.require(START, SIZE)
        _, all_rows = read_tsv(RETAIL_CONFIG / 'relocs.tsv')
        self.rows = {int(row['site_va'], 16): row for row in all_rows
                     if row['image'] == 'GAME.EXE'}

    def test_control_and_data_relocations_have_exact_retail_targets(self):
        for site, target in {**JUMPS, **CALLS, **PAIRS}.items():
            with self.subTest(site=hex(site)):
                row = self.rows[site]
                self.assertEqual(row['status'], 'reviewed')
                decoded = validate_relocation(
                    self.blob, self.function, row, self.catalog, 'safe')
                self.assertEqual(decoded.target, target)
                if site in PAIRS:
                    self.assertEqual(decoded.paired_site, site + 4)
                wrong = dict(row, target_va=f'{target + 4:#x}')
                with self.assertRaisesRegex(ValueError, 'decoded-target-mismatch'):
                    validate_relocation(self.blob, self.function, wrong, self.catalog, 'safe')
        words = struct.unpack(f'<{SIZE // 4}I', self.blob)
        controls = {START + 4 * i: decode_control(START + 4 * i, word)
                    for i, word in enumerate(words)}
        self.assertEqual({site: c.target for site, c in controls.items()
                          if c is not None and c.kind == 'jump'}, JUMPS)
        self.assertEqual({site: c.target for site, c in controls.items()
                          if c is not None and c.call}, CALLS)

    def test_switch_bound_and_all_table_destinations(self):
        self.assertEqual(self.image.require(0x80037014, 16), struct.pack(
            '<4I', 0x2484FFFC, 0x3402002C, 0x0044102B, 0x144001C8))
        self.assertEqual(self.image.require(0x80037028, 32), struct.pack(
            '<8I', 0x00041080, 0x3C018001, 0x24212C28, 0x00220821,
            0x8C220000, 0, 0x00400008, 0))
        words = struct.unpack('<45I', self.image.require(TABLE, 180))
        self.assertEqual(words, tuple(CASES.get(kind, 0x80037740) for kind in range(4, 49)))
        slots = {START + 4 * i + 4 for i, word in enumerate(
            struct.unpack(f'<{SIZE // 4}I', self.blob))
            if (c := decode_control(START + 4 * i, word)) is not None and c.delay_slot}
        for index, target in enumerate(words):
            self.assertTrue(START <= target < START + SIZE)
            self.assertEqual(target & 3, 0)
            self.assertNotIn(target, slots)
            row = self.rows[TABLE + 4 * index]
            self.assertEqual(row['status'], 'reviewed')
            self.assertEqual(int(row['target_va'], 16), target)

    def test_carved_constructor_relinks_every_instruction(self):
        unit = load_manifest().by_name()['game.effect_pool']
        target = BUILD / 'delink/game/modules' / unit.object_name
        if not target.is_file():
            self.skipTest('fresh delinked effect-pool module is required')
        obj = _load_object(target)
        claim = next(c for c in unit.functions if c.va == START)
        data = {item.name: item.va for (key, _), item in
                load_data_identities(RETAIL_CONFIG).items() if key == 'GAME.EXE'}
        functions = {item.symbol: item.va for item in self.catalog.functions['GAME.EXE']}
        actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
        self.assertEqual(actual, list(struct.unpack(f'<{SIZE // 4}I', self.blob)))
        self.assertEqual(calls, list(CALLS.values()))
        self.assertEqual(addresses, list(PAIRS.values()))

    def test_fresh_compiled_unit_and_switch_table_match_retail(self):
        if not all(shutil.which(tool) for tool in ('cpppsx-257', 'cc1psx-257')):
            self.skipTest('pinned compiler is required')
        if 'PSYQ_INCLUDE' not in os.environ:
            self.skipTest('pinned SDK headers are required')
        manifest = load_manifest()
        unit = manifest.by_name()['game.effect_pool']
        profile = manifest.profiles[unit.profile]
        data = {item.name: item.va for (key, _), item in
                load_data_identities(RETAIL_CONFIG).items() if key == 'GAME.EXE'}
        functions = {item.symbol: item.va for item in self.catalog.functions['GAME.EXE']}
        with TemporaryDirectory(prefix='kf-effect-pool-') as directory:
            output = Path(directory) / unit.object_name
            compile_source(
                unit.source_path, unit.image, output, BUILD / 'delink', profile.optimization,
                profile.small_data, profile.aspsx_version,
                (REPO / 'include', REPO / 'vendor/include', REPO / 'tests/fixtures', Path(os.environ['PSYQ_INCLUDE'])),
                profile.cc1_flags, profile.compiler, defines=unit.defines)
            obj = _load_object(output)
            for claim in unit.functions:
                with self.subTest(function=claim.symbol):
                    actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                    expected = struct.unpack(f'<{claim.body_size // 4}I',
                                             self.image.require(claim.va, claim.body_size))
                    self.assertEqual(actual, list(expected))
                    if claim.va == START:
                        self.assertEqual(calls, list(CALLS.values()))
                        self.assertEqual(addresses, list(PAIRS.values()))
            constructor = obj.named_symbol('effect_pool_construct')
            table = list(struct.unpack_from('<45I', obj.sections['.rodata']))
            relocs = [r for r in obj.relocations
                      if r.section == '.rodata' and r.offset < 180]
            self.assertEqual(sorted(r.offset for r in relocs), list(range(0, 180, 4)))
            for reloc in relocs:
                self.assertEqual(reloc.kind, 2)  # R_MIPS_32
                symbol = obj.symbol(reloc.symbol_index)
                self.assertEqual(symbol.section, '.text')
                offset = symbol.value + table[reloc.offset // 4] - constructor.value
                self.assertTrue(0 <= offset < SIZE)
                table[reloc.offset // 4] = START + offset
            self.assertEqual(table, list(struct.unpack('<45I', self.image.require(TABLE, 180))))
