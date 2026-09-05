"""Complete player DATA owners; individual payload checks are not TU closure."""

from __future__ import annotations

import ast
import re
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


# Complete allocations, including each path's terminating NUL. Gap bytes are
# deliberately not part of these arrays or a claim of original TU boundaries.
OWNERS = (
    ('game.player_core', 0x8005581C, 'weapon_image_path_template', b'WEPON\\WEP00.MIM\0'),
    ('game.player_core', 0x8005582C, 'floor_entry_cells', bytes((15, 2, 29, 56, 28, 18, 7, 22, 39, 69))),
    ('game.player_use_item', 0x80055838, 'enemy_info_image_path_template', b'ENE0\\EI00.TIM\0'),
    ('game.player_use_item', 0x80055848, 'person_image_path_template', b'PRSN\\PER00.TIM\0'),
)
REFERENCES = (
    (0x80016AAC, 0x8005581C, 9), (0x80016AC8, 0x8005581C, 10),
    (0x80017D40, 0x8005582C, -2),
    (0x80017F0C, 0x80055838, 3), (0x80017F58, 0x80055838, 7), (0x80017F84, 0x80055838, 8),
    (0x80018000, 0x80055848, 8), (0x80018034, 0x80055848, 9),
)


class GamePlayerDataTests(unittest.TestCase):
    def test_complete_allocations_have_one_source_and_one_census_owner(self):
        manifest = load_manifest()
        identities = load_data_identities(RETAIL_CONFIG)
        census = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1] if r['image'] == 'GAME.EXE']
        for unit_name, va, name, payload in OWNERS:
            with self.subTest(name=name):
                claims = [(unit.unit, datum) for unit in manifest.units if unit.image == 'GAME.EXE'
                          for datum in unit.data if datum.va == va]
                self.assertEqual(len(claims), 1)
                owner, datum = claims[0]
                self.assertEqual((owner, datum.symbol, datum.size, datum.storage, datum.scope),
                                 (unit_name, name, len(payload), 'load', 'global'))
                identity = identities['GAME.EXE', va]
                self.assertEqual((identity.name, identity.size, identity.storage),
                                 (name, len(payload), 'load'))
                overlap = [r for r in census if int(r['va'], 0) < va + len(payload)
                           and va < int(r['va'], 0) + int(r['size'], 0)]
                self.assertEqual(len(overlap), 1)
                self.assertEqual((int(overlap[0]['va'], 0), int(overlap[0]['size'], 0)),
                                 (va, len(payload)))
        for va, size in ((0x80055836, 2), (0x80055846, 2), (0x80055857, 1)):
            row = next(r for r in census if int(r['va'], 0) == va)
            self.assertEqual((int(row['size'], 0), row['kind']), (size, 'unclassified'))

    def test_c_initializers_include_terminators_and_x_before_z(self):
        units = load_manifest().by_name()
        for unit, _, name, payload in OWNERS:
            source = units[unit].source_path.read_text()
            if name == 'floor_entry_cells':
                definition = re.search(r'KfFloorEntryCell floor_entry_cells\[5\] = \{(.*?)\};', source, re.S)
                self.assertIsNotNone(definition)
                pairs = re.findall(r'\{(\d+), (\d+)\}', definition[1])
                self.assertEqual(bytes(int(value) for pair in pairs for value in pair), payload)
                continue
            definition = re.search(r'char ' + name + r'\[(\d+)\] = ("(?:\\.|[^"\\])*");', source)
            self.assertIsNotNone(definition)
            self.assertEqual(int(definition[1]), len(payload))
            self.assertEqual(ast.literal_eval(definition[2]).encode('ascii') + b'\0', payload)
            self.assertNotIn(name, (REPO / 'include/kf/game_state.h').read_text())
        header = (REPO / 'include/kf/game_player.h').read_text()
        self.assertRegex(header, r'typedef struct KfFloorEntryCell\s*\{\s*u8 x;\s*u8 z;\s*\}')

    def test_curated_relocation_sites_and_signed_addends_are_unchanged(self):
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        names = {va: name for _, va, name, _ in OWNERS}
        for site, va, addend in REFERENCES:
            row = rows['GAME.EXE', site]
            self.assertEqual((row['kind'], row['status'], row['target_name']),
                             ('mips_hi16_lo16', 'reviewed', names[va]))
            self.assertEqual(int(row['target_va'], 0), va + addend)
            self.assertEqual(int(row['paired_site_va'], 0), site + 4)

    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def test_retail_complete_initializers_and_reference_owner_resolution(self):
        image = self.retail()
        ctx = Context('GAME.EXE')
        for _, va, name, payload in OWNERS:
            self.assertEqual(image.require(va, len(payload)), payload)
            references = ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
            expected = [(site, addend) for site, owner, addend in REFERENCES if owner == va]
            self.assertEqual([(r.site, r.target - va) for r in references], expected)
            self.assertTrue(all(r.destination == va and r.referent.name == name for r in references))
        for site, va, addend in REFERENCES:
            self.assertEqual(decode_hi_lo_target(image.u32(site), image.u32(site + 4)), va + addend)

    def test_full_compiled_and_delinked_allocations_equal_retail(self):
        image = self.retail()
        units = load_manifest().by_name()
        for unit_name, va, name, payload in OWNERS:
            unit = units[unit_name]
            paths = (BUILD / 'objdiff/game/base' / unit.object_name,
                     BUILD / 'delink/game/modules' / unit.object_name)
            if any(not path.is_file() for path in paths):
                self.skipTest('freshly built player source and target objects required')
            for path in paths:
                with self.subTest(name=name, object=str(path)), path.open('rb') as stream:
                    elf = ELFFile(stream)
                    symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(name)
                    self.assertEqual(len(symbols or ()), 1)
                    symbol = symbols[0]
                    self.assertEqual((symbol['st_size'], symbol['st_info']['type'], symbol['st_info']['bind']),
                                     (len(payload), 'STT_OBJECT', 'STB_GLOBAL'))
                    section = elf.get_section(symbol['st_shndx'])
                    self.assertEqual((section.name, section['sh_type'], section['sh_flags']),
                                     ('.data', 'SHT_PROGBITS', 3))
                    start = symbol['st_value']
                    # Exactly the complete declared object; no NUL stripping or
                    # prefix comparison. Whole-section verification is separate.
                    actual = section.data()[start:start + symbol['st_size']]
                    self.assertEqual(actual, payload)
                    self.assertEqual(actual, image.require(va, len(payload)))


if __name__ == '__main__':
    unittest.main()
