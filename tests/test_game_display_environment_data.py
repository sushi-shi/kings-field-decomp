"""GAME environment BSS ownership, first-declaration order and exact screen CFG."""

from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import Elf, _diff_bss, diff_unit
from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import (
    decode_hi_lo_target, encode_hi_lo_addend, validate_relocation,
)
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from tests import test_game_graphics_owner_probe as graphics
from tests.test_open_runtime_owner_probe import linked_words


OWNERS = (
    (0x80090EC0, 'display_draw_environments', 0xB8),
    (0x80090F78, 'display_disp_environments', 0x28),
)
REFERENCES = {
    0x80090EC0: (
        0x8001455C, 0x8001456C, 0x8001B9C4, 0x8001B9D4, 0x8001B9E4,
        0x8001BA74, 0x8001BA84, 0x8001BBC0, 0x8001BBF8, 0x8001BC30,
        0x8001BC38, 0x8001BC44, 0x8001BC4C, 0x8001BC54, 0x8001BC5C,
        0x8001BC64, 0x8001BC6C, 0x8001BC74, 0x8001BC7C, 0x8001C090,
        0x8002AC74, 0x8002C8A0, 0x8002C8B0, 0x8002C8C0, 0x8002C990, 0x8002C9A0,
    ),
    0x80090F78: (0x8001BBDC, 0x8001BC14, 0x8001C0B4, 0x8002AC98),
}


class GameDisplayEnvironmentDataTests(unittest.TestCase):
    def probe(self):
        return graphics.GameGraphicsOwnerProbeTests()

    def address_maps(self):
        data = {d.name: d.va for (image, _), d in load_data_identities(RETAIL_CONFIG).items()
                if image == 'GAME.EXE'}
        functions = {f.symbol: f.va for f in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        return data, functions

    def test_complete_source_claims_keep_interior_fields_and_following_gap_separate(self):
        manifest, identities = load_manifest(), load_data_identities(RETAIL_CONFIG)
        for va, name, size in OWNERS:
            claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                      for d in u.data if va <= d.va < va + size]
            self.assertEqual(len(claims), 1)
            unit, datum = claims[0]
            self.assertEqual((unit, datum.va, datum.symbol, datum.size, datum.storage, datum.scope),
                             ('game.render', va, name, size, 'bss', 'global'))
            identity = identities['GAME.EXE', va]
            self.assertEqual((identity.name, identity.size, identity.storage), (name, size, 'bss'))
            self.assertEqual([a for i, a in identities if i == 'GAME.EXE' and va < a < va + size], [])
        self.assertEqual(OWNERS[-1][0] + OWNERS[-1][2], 0x80090FA0)
        self.assertEqual(identities['GAME.EXE', 0x80090FA8].name, 'tmd_state')
        source = manifest.by_name()['game.render'].source_path.read_text()
        header = (REPO / 'include/kf/game_render.h').read_text()
        for declaration in ('DRAWENV display_draw_environments[KF_DISPLAY_BUFFER_COUNT];',
                            'DISPENV display_disp_environments[KF_DISPLAY_BUFFER_COUNT];'):
            self.assertIn(declaration, source)
            self.assertIn('extern ' + declaration, header)

    def test_pinned_sdk_layout_and_retail_cross_array_derivation(self):
        probe = self.probe()
        probe.tools()
        image = probe.retail()
        unit = load_manifest().by_name()['game.render']
        fields = ('dtd', 'dfe', 'isbg', 'r0', 'g0', 'b0', 'dr_env')
        query = ('#include <kf/game_render.h>\nunsigned long layout[] = {\n'
                 'sizeof(DRAWENV), sizeof(DISPENV), sizeof(DR_ENV),\n'
                 'sizeof(display_draw_environments), sizeof(display_disp_environments),\n'
                 + ',\n'.join('(unsigned long)&((DRAWENV *)0)->' + f for f in fields)
                 + '};\n')
        with tempfile.TemporaryDirectory() as directory:
            obj = probe.compile(Path(directory), unit, query)
            symbol = obj.named_symbol('layout')
            measured = struct.unpack_from('<12I', obj.sections['.data'], symbol.value)
            self.assertEqual(measured, (92, 20, 64, 184, 40, 0x16, 0x17, 0x18,
                                        0x19, 0x1A, 0x1B, 0x1C))
        # s0 = DRAWENV[0].dtd, then the PutDispEnv delay slot derives DISPENV[0].
        self.assertEqual(image.require(0x8001BC38, 8), struct.pack('<2I', 0x3C108009, 0x26100ED6))
        self.assertEqual(image.require(0x8001BC84, 8), struct.pack('<2I', 0x0C01439A, 0x260400A2))
        self.assertEqual(0x80090EC0 + 0x16 + 0xA2, 0x80090F78)
        # The same base reaches fog in the larger unresolved graphics owner.
        self.assertEqual(image.require(0x8001BCB8, 4), struct.pack('<I', 0xAE02486A))
        self.assertEqual(0x80090EC0 + 0x16 + 0x486A, 0x80095740)

    def test_complete_bss_matches_layout_and_placement_without_hiding_rodata(self):
        image = self.probe().retail()
        unit = load_manifest().by_name()['game.render']
        paths = [BUILD / prefix / unit.object_name
                 for prefix in ('delink/game/modules', 'objdiff/game/base')]
        if not all(path.is_file() for path in paths):
            self.skipTest('freshly built GAME render source and target objects required')
        for path in paths:
            with path.open('rb') as stream:
                elf = ELFFile(stream)
                section = elf.get_section_by_name('.bss')
                self.assertEqual((section['sh_type'], section['sh_size']), ('SHT_NOBITS', 224))
                for (va, name, size), offset in zip(OWNERS, (0, 184)):
                    symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(name)
                    self.assertEqual(len(symbols), 1)
                    symbol = symbols[0]
                    self.assertEqual((symbol['st_value'], symbol['st_size'], symbol['st_info']['bind']),
                                     (offset, size, 'STB_GLOBAL'))
                    self.assertEqual(elf.get_section(symbol['st_shndx']).name, '.bss')
                    self.assertFalse(image.contains(va, size))
        retail, source = [Elf(path) for path in paths]
        self.assertEqual(_diff_bss(retail, source).status, 'match')
        whole = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
        self.assertFalse(whole.matches)
        sections = {d.name: d for d in whole.diffs}
        self.assertEqual(sections['.bss'].status, 'match')
        self.assertEqual(sections['.bss'].detail, '')
        # This campaign does not claim the existing switch-table residue is fixed.
        self.assertEqual(sections['.rodata'].status, 'addend')

    def test_first_header_declaration_decides_common_order_without_changing_text(self):
        probe = self.probe()
        probe.tools()
        unit = load_manifest().by_name()['game.render']
        target = BUILD / 'delink/game/modules' / unit.object_name
        if not target.is_file():
            self.skipTest('delinked GAME render target required')
        canonical = unit.source_path.read_text()
        declarations = ('DRAWENV display_draw_environments[2];',
                        'DISPENV display_disp_environments[2];')
        # Early declarations are a controlled substitute for each header order;
        # production has one shared declaration of each, in retail order.
        text = None
        for wrong in (False, True):
            first = declarations[::-1] if wrong else declarations
            prefix = '#include <kf/psyq.h>\n' + ''.join('extern ' + d + '\n' for d in first)
            with self.subTest(reversed=wrong), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                obj = probe.compile(root, unit, prefix + canonical)
                source = Elf(root / unit.object_name)
                self.assertEqual(source.sections['.bss'].size, 224)
                result = _diff_bss(Elf(target), source)
                self.assertEqual(result.status, 'layout' if wrong else 'match')
                values = [obj.named_symbol(name).value for _, name, _ in OWNERS]
                self.assertEqual(values, [40, 0] if wrong else [0, 184])
                if text is None:
                    text = obj.sections['.text']
                else:
                    self.assertEqual(obj.sections['.text'], text)

    def test_all_thirty_pairs_use_array_owners_and_round_trip_exactly(self):
        image, ctx = self.probe().retail(), Context('GAME.EXE')
        catalog = load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        new = {0x8002C8B0: 0x17, 0x8002C8C0: 0, 0x8002C9A0: 0x17}
        for va, name, size in OWNERS:
            references = ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
            self.assertEqual({r.site for r in references}, set(REFERENCES[va]))
            for reference in references:
                site = reference.site
                row = rows['GAME.EXE', site]
                target = int(row['target_va'], 0)
                self.assertEqual((row['target_name'], row['status']), (name, 'reviewed'))
                self.assertTrue(va <= target < va + size)
                self.assertEqual(reference.referent.name, name)
                owner = ctx.idx.function_owner(site)
                function = catalog.function_starts['GAME.EXE'][owner.va]
                original = image.require(owner.va, owner.body_size)
                body = bytearray(original)
                relocs, used = _apply_relocation(body, function, row, catalog, 'safe')
                self.assertEqual([r.symbol for r in relocs], [name, name])
                self.assertEqual(int(used['addend'], 0), target - va)
                words = struct.unpack_from('<2I', body, site - owner.va)
                self.assertEqual(decode_hi_lo_target(*words), target - va)
                self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(*words, target)),
                                 image.require(site, 8))
                if site in new:
                    self.assertEqual(target - va, new[site])
                    self.assertIn('manual:game_display_environment_data', row['provenance'].split(';'))
                    with self.assertRaisesRegex(ValueError, '^decoded-target-mismatch$'):
                        validate_relocation(original, function, dict(row, target_va=hex(target + 1)),
                                            catalog, 'safe')

    def test_screen_body_is_exact_and_old_conditional_stores_are_rejected(self):
        probe = self.probe()
        probe.tools()
        image = probe.retail()
        unit = load_manifest().by_name()['game.save_system']
        claim = next(c for c in unit.functions if c.symbol == 'screen_show_image_until_input')
        data, functions = self.address_maps()
        expected = list(struct.unpack('<144I', image.require(claim.va, claim.body_size)))
        self.assertEqual(expected[0x15C // 4:0x174 // 4],
                         [0x10400002, 0, 0x26310001, 0xA3B10014, 0xA3B10015, 0xA3B10016])
        correct = '''        if (brightness < IMAGE_WAIT_MAX_BRIGHTNESS) {
            brightness++;
        }
        polygon.r0 = brightness;
        polygon.g0 = brightness;
        polygon.b0 = brightness;'''
        old = '''        if (brightness < IMAGE_WAIT_MAX_BRIGHTNESS) {
            brightness++;
            polygon.r0 = brightness;
            polygon.g0 = brightness;
            polygon.b0 = brightness;
        }'''
        canonical = unit.source_path.read_text()
        self.assertEqual(canonical.count(correct), 1)
        expected_calls = [((claim.va + i * 4 + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
                          for i, w in enumerate(expected) if w >> 26 == 3]
        self.assertEqual(len(expected_calls), 16)
        for wrong in (False, True):
            with self.subTest(old_bug=wrong), tempfile.TemporaryDirectory() as directory:
                obj = probe.compile(Path(directory), unit,
                                    canonical.replace(correct, old) if wrong else canonical)
                actual, calls, targets = linked_words(obj, unit, claim, data, functions)
                self.assertEqual(calls, expected_calls)
                self.assertEqual(targets, [0x80070E9C, 0x80070E98, 0x80090ED8, 0x80090ED7,
                                           0x80090EC0, 0x80090EBC, 0x80090ED8, 0x80090ED7])
                if wrong:
                    self.assertEqual(actual[:0x15C // 4], expected[:0x15C // 4])
                    self.assertEqual(actual[0x15C // 4], 0x10400005)
                    self.assertNotEqual(actual, expected)
                else:
                    self.assertEqual(actual, expected)
                    shifted = {**data, 'display_draw_environments': 0x80090EC1}
                    bad, same_calls, _ = linked_words(obj, unit, claim, shifted, functions)
                    self.assertNotEqual(bad, expected)
                    self.assertEqual(same_calls, calls)

    def test_array_only_aggregate_regresses_exact_presenters(self):
        probe = self.probe()
        probe.tools()
        image, data_maps = probe.retail(), self.address_maps()
        data, functions = data_maps
        data['display_environments_probe'] = 0x80090EC0
        for name, target in (('game.render', 'display_present_frame'),
                             ('game.menu_runtime', 'menu_present_frame')):
            unit = load_manifest().by_name()[name]
            claim = next(c for c in unit.functions if c.symbol == target)
            canonical = unit.source_path.read_text()
            selected = graphics.candidate_source(unit, {target})
            # Reuse the bounded function-only rewrite, but test a smaller owner
            # than the complete-clear pilot. Both authentic array types remain.
            selected = selected.replace('#include "game_graphics_owner_probe.h"',
                                        '#include <kf/game_render.h>\n'
                                        'typedef struct { DRAWENV draw[2]; DISPENV disp[2]; } '
                                        'DisplayEnvironmentsProbe;\n'
                                        'extern DisplayEnvironmentsProbe display_environments_probe;')
            selected = selected.replace('graphics_owner_probe.display_draw_environments',
                                        'display_environments_probe.draw')
            selected = selected.replace('graphics_owner_probe.display_disp_environments',
                                        'display_environments_probe.disp')
            selected = selected.replace('graphics_owner_probe.display_state', 'display_state')
            expected = list(struct.unpack('<38I', image.require(claim.va, claim.body_size)))
            for small_owner in (False, True):
                with self.subTest(unit=name, small_owner=small_owner), tempfile.TemporaryDirectory() as directory:
                    obj = probe.compile(Path(directory), unit, selected if small_owner else canonical)
                    actual, calls, _ = linked_words(obj, unit, claim, data, functions)
                    self.assertEqual(len(calls), 5)
                    if small_owner:
                        self.assertEqual((len(actual) * 4, actual[0]), (148, 0x27BDFFE0))
                        self.assertNotEqual(actual, expected)
                    else:
                        self.assertEqual(actual, expected)


if __name__ == '__main__':
    unittest.main()
