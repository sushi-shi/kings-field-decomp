"""Whole menu asset ownership and bounded retail controls, not menu closure."""

from __future__ import annotations

import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import diff_unit
from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities, load_structure_field_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import (
    CandidateFunction, CandidateProgram, ExternalHook, GameSymbols, HookReturn,
    MemoryInput, MemoryRange, ParserMachine, RetailProgram,
)
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


ASSETS = 0x800580E8
ASSET_SIZE = 912
BANKS = ((ASSETS, 912), (0x80058478, 2376), (0x80058DC0, 1600),
         (0x80059400, 180), (0x800594B8, 320), (0x800595F8, 320))
WINDOW_PAIRS = ((0x8002897C, 0x33C), (0x8002898C, 0x30C),
                (0x800289B8, 0x33C), (0x800289D0, 0x348),
                (0x800289F8, 0x384), (0x80028A08, 0x30C))
PACKETS = 0x800F0000
ORDERING_TABLE = 0x800F2000


class GameMenuAssetsTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def object(self, unit):
        path = BUILD / 'objdiff/game/base' / load_manifest().by_name()[unit].object_name
        if not path.is_file():
            self.skipTest('fresh menu source and delinked objects required')
        return path

    def test_one_complete_owner_no_interior_identities_and_no_field_holes(self):
        identities = load_data_identities(RETAIL_CONFIG)
        owners = [d for (image, va), d in identities.items()
                  if image == 'GAME.EXE' and va < ASSETS + ASSET_SIZE
                  and ASSETS < va + d.size]
        self.assertEqual([(d.va, d.size, d.name, d.datatype, d.storage) for d in owners],
                         [(ASSETS, ASSET_SIZE, 'menu_assets', 'KfMenuAssets', 'bss')])
        claims = [(u.unit, d.symbol, d.size, d.scope) for u in load_manifest().units
                  if u.image == 'GAME.EXE' for d in u.data if d.va == ASSETS]
        self.assertEqual(claims, [('game.item', 'menu_assets', ASSET_SIZE, 'global')])
        fields = [f for f in load_structure_field_identities(RETAIL_CONFIG)
                  if f.structure == 'KfMenuAssets']
        self.assertEqual([f.offset for f in fields],
                         [0, 0x140, 0x190, 0x1E0, 0x300, 0x30C, 0x318,
                          0x324, 0x330, 0x33C, 0x348, 0x354, 0x384])
        self.assertEqual(sum(f.size for f in fields), ASSET_SIZE)
        self.assertEqual(fields[3].datatype, 'POLY_F4[2][6]')
        self.assertEqual(fields[10].name, 'row_confirmed_background')
        opaque = [f for f in load_structure_field_identities(RETAIL_CONFIG)
                  if f.structure == 'MenuTileSprite' and f.meaning_confidence == 'opaque']
        self.assertEqual([(f.offset, f.size) for f in opaque], [(5, 1), (7, 1)])

    def test_complete_source_and_target_bss_allocations_match(self):
        source = self.object('game.item')
        target = BUILD / 'delink/game/modules' / source.name
        if not target.is_file():
            self.skipTest('fresh delinked menu owner required')
        for path in (source, target):
            with path.open('rb') as stream:
                elf = ELFFile(stream)
                symbols = elf.get_section_by_name('.symtab').get_symbol_by_name('menu_assets')
                self.assertEqual(len(symbols or ()), 1)
                symbol = symbols[0]
                self.assertEqual((symbol['st_value'], symbol['st_size'], symbol['st_info']['bind']),
                                 (0, ASSET_SIZE, 'STB_GLOBAL'))
                section = elf.get_section(symbol['st_shndx'])
                self.assertEqual((section.name, section['sh_type'], section['sh_size']),
                                 ('.bss', 'SHT_NOBITS', ASSET_SIZE))
        comparison = diff_unit('GAME.EXE', source.name, BUILD / 'delink', BUILD / 'objdiff')
        self.assertIsNotNone(comparison)
        self.assertEqual([(d.name, d.status) for d in comparison.diffs],
                         [('.rodata', 'match'), ('.bss', 'match')])

    def test_all_380_reviewed_interior_references_round_trip(self):
        image = self.retail()
        ctx, catalog = Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = [r for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
                if r['image'] == 'GAME.EXE' and r['target_name'] == 'menu_assets']
        self.assertEqual(len(rows), 380)
        by_site = {int(r['site_va'], 0): r for r in rows}
        for site, offset in WINDOW_PAIRS:
            self.assertEqual(int(by_site[site]['target_va'], 0), ASSETS + offset)
        incoming = {r.site: r for r in ctx.refs.incoming(ctx.idx.datum(ASSETS),
                                                       confirmed_only=True)}
        for row in rows:
            site, low_site, target = (int(row[key], 0)
                                      for key in ('site_va', 'paired_site_va', 'target_va'))
            with self.subTest(site=hex(site)):
                self.assertEqual((row['kind'], row['status']), ('mips_hi16_lo16', 'reviewed'))
                self.assertTrue(ASSETS <= target < ASSETS + ASSET_SIZE)
                reference = incoming[site]
                self.assertEqual((reference.destination, reference.referent.name),
                                 (ASSETS, 'menu_assets'))
                owner = ctx.idx.function_owner(site)
                body = bytearray(image.require(owner.va, owner.body_size))
                relocations, used = _apply_relocation(
                    body, catalog.function_starts['GAME.EXE'][owner.va], row, catalog, 'safe')
                self.assertEqual([r.symbol for r in relocations], ['menu_assets', 'menu_assets'])
                self.assertEqual(int(used['addend'], 0), target - ASSETS)
                high = struct.unpack_from('<I', body, site - owner.va)[0]
                low = struct.unpack_from('<I', body, low_site - owner.va)[0]
                self.assertEqual(decode_hi_lo_target(high, low), target - ASSETS)
                restored = encode_hi_lo_addend(high, low, target)
                self.assertEqual(restored, (image.u32(site), image.u32(low_site)))

    def test_loader_copies_all_six_banks_and_preserves_the_alignment_gap(self):
        image, symbols = self.retail(), GameSymbols.load()
        function = 'item_load_database'
        path = self.object('game.item')
        payload = bytes((i * 73 + 19) & 255 for i in range(sum(size for _, size in BANKS)))
        for alignment in range(4):
            source = 0x800F4000 + alignment

            def service(ctx):
                if ctx.call.name == 'cd_file_load_allocated':
                    self.assertEqual(ctx.read(ctx.args[1], 13), b'COM\\STAT.DAT\0')
                    ctx.write_u32(ctx.args[0], source)
                elif ctx.call.name == 'memset':
                    ctx.write(ctx.args[0], bytes([ctx.args[1] & 255]) * ctx.args[2])
                elif ctx.call.name == 'exit':
                    self.fail('unexpected loader error path')
                return HookReturn(0)

            hooks = [ExternalHook(name, service) for name in
                     ('cd_file_load_allocated', 'memset', 'exit', 'memory_release_last', 'CdSearchFile')]
            programs = (RetailProgram.link(symbols, [function], hooks=hooks),
                        CandidateProgram.link(symbols, [CandidateFunction(function, path)], hooks=hooks))
            regions = [MemoryRange(f'bank{i}', va, size) for i, (va, size) in enumerate(BANKS)]
            gap = MemoryRange('gap', 0x800594B4, 4)
            for program in programs:
                with self.subTest(alignment=alignment, program=program.label):
                    result = ParserMachine(image, program).call(
                        function, memory=[MemoryInput(source, payload), MemoryInput(gap.address, b'KEEP')],
                        capture=[*regions, gap], allowed_writes=regions, instruction_limit=30000)
                    got, offset = result.memory_by_name(), 0
                    for region in regions:
                        self.assertEqual(got[region.name], payload[offset:offset + region.size])
                        offset += region.size
                    self.assertEqual(got['gap'], b'KEEP')
                    self.assertEqual(sum(call.name == 'CdSearchFile' for call in result.trace), 80)

    def test_retail_and_source_mirrored_packets_and_background_enqueue_order(self):
        image, symbols = self.retail(), GameSymbols.load()
        path = self.object('game.menu_runtime')
        display = symbols.datum('display_state')[0]
        current = symbols.datum('current_poly_ft4')[0]
        descriptor = (0x123, 0x7C80, 231, 197, 71, 104)
        asset_data = bytearray(ASSET_SIZE)
        struct.pack_into('<HHBxBxHH', asset_data, 0x318, *descriptor)
        passive = ('RotMatrix', 'MulMatrix0', 'SetLightMatrix', 'SetRotMatrix',
                   'SetTransMatrix', 'menu_render_item_model')
        for function, origin_x in (('menu_draw_window_backdrop', 166),
                                   ('menu_draw_item_name_frame', 118)):
            for buffer_index in (0, 1):
                state = {'next': PACKETS}
                labels = []

                def service(ctx):
                    name = ctx.call.name
                    if name == 'primitive_buffer_begin_poly_ft4':
                        ctx.write_u32(current, state['next'])
                        ctx.write(state['next'], bytes([0xA5]) * 40)
                        ctx.write(state['next'] + 7, b'\x2c')
                    elif name == 'SetSemiTrans':
                        self.assertEqual(ctx.args[1], 1)
                        ctx.write(ctx.args[0] + 7, b'\x2e')
                    elif name == 'primitive_buffer_commit_poly_ft4':
                        self.assertEqual(ctx.args[0], 2900)
                        state['next'] += 40
                    elif name == 'menu_draw_string':
                        labels.append((ctx.args[0], ctx.read(ctx.args[1], 24)))
                    elif name not in (*passive, 'AddPrim'):
                        self.fail(f'unexpected external service {name}')
                    return HookReturn(0)

                names = ('primitive_buffer_begin_poly_ft4', 'primitive_buffer_commit_poly_ft4',
                         'SetSemiTrans', 'AddPrim')
                if function == 'menu_draw_item_name_frame':
                    names += (*passive, 'menu_draw_string')
                hooks = [ExternalHook(name, service) for name in names]
                programs = (RetailProgram.link(symbols, [function], hooks=hooks),
                            CandidateProgram.link(symbols, [CandidateFunction(function, path)], hooks=hooks))
                packet_range = MemoryRange('packets', PACKETS, 160)
                pointer_range = MemoryRange('current', current, 4)
                spin_range = MemoryRange('spin', 0x80057B72, 2)
                glyphs = struct.pack('<10h', *range(41, 51))
                inputs = [MemoryInput(ASSETS, bytes(asset_data)),
                          MemoryInput(display, bytes([buffer_index])),
                          MemoryInput(display + 0x20024, struct.pack('<I', ORDERING_TABLE)),
                          MemoryInput(display + 0x20, struct.pack('<I', display + 8)),
                          MemoryInput(display + 16, struct.pack('<I', PACKETS)),
                          MemoryInput(0x80058DC0 + 3 * 20, glyphs),
                          MemoryInput(spin_range.address, struct.pack('<H', 4092))]
                for program in programs:
                    state['next'] = PACKETS
                    labels.clear()
                    with self.subTest(function=function, buffer=buffer_index, program=program.label):
                        result = ParserMachine(image, program).call(
                            function, [3], memory=inputs, capture=[packet_range, spin_range],
                            allowed_writes=[packet_range, pointer_range, spin_range], instruction_limit=2000)
                        packets = result.memory_by_name()['packets']
                        for tile in range(4):
                            expected = bytearray([0xA5]) * 40
                            expected[7] = 0x2E
                            page, clut, u, v, width, height = descriptor
                            x, y = origin_x + (tile & 1) * 71, 16 + (tile >> 1) * 104
                            struct.pack_into('<H', expected, 14, clut)
                            struct.pack_into('<H', expected, 22, page)
                            for corner, offset in enumerate((8, 16, 24, 32)):
                                struct.pack_into('<hh', expected, offset,
                                                 x + (corner & 1) * width, y + (corner >> 1) * height)
                                expected[offset + 4] = (u + ((corner ^ tile) & 1) * width) & 255
                                expected[offset + 5] = (v + (((corner ^ tile) >> 1) & 1) * height) & 255
                            self.assertEqual(packets[tile * 40:(tile + 1) * 40], expected)
                        enqueues = [call.args[:2] for call in result.trace if call.name == 'AddPrim']
                        self.assertEqual(enqueues, [(ORDERING_TABLE + 3000 * 4,
                                                     ASSETS + buffer_index * 160 + i * 40)
                                                    for i in (3, 2, 1, 0)])
                        if function == 'menu_draw_item_name_frame':
                            self.assertEqual(labels, [(ASSETS + 0x30C, struct.pack('<2H', 128, 36) + glyphs)])
                            self.assertEqual(result.memory_by_name()['spin'], b'\x04\0')


if __name__ == '__main__':
    unittest.main()
