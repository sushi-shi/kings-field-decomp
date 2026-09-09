"""Complete map-script constants, retail referents and bounded consumers."""

from __future__ import annotations

import json
import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import Elf, _diff_init_section, diff_unit
from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import (
    ExternalHook, GameSymbols, HookReturn, MemoryInput, MemoryRange,
    ParserMachine, RetailProgram,
)
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


SEGMENTS = (
    (65, 80, 1, 0, 2, 0, 100), (61, 73, 0, 255, 2, 0, 100),
    (75, 56, 1, 0, 3, 0, 100), (37, 27, 0, 255, 3, 0, 100),
    (32, 82, 0, 1, 12, 0, 100),
)
OWNERS = (
    ('game.map_scripts', 0x800561D0, 'map_floor5_camera_path',
     struct.pack('<4i6h', 173000, -11500, 85000, 0, 0, 2048, 0, 0, 100, 0)
     + struct.pack('<4i6h', -1, -1, -1, 0, -1, -1, -1, 0, -1, 0)),
    ('game.map_scripts', 0x80056208, 'map_floor1_sound_position',
     struct.pack('<4i', 65000, -10000, 25000, 0)),
    ('game.map_scripts', 0x80056218, 'map_reveal_light_matrix',
     struct.pack('<9h2x3i', *([0, -4096, 0] * 3), 0, 0, 0)),
    ('game.map_scripts', 0x80056238, 'map_screen_image_path', b'KAN\\B0\\K000.TIM\0'),
    ('game.player_warp', 0x80056248, 'actor_transform_color_matrix',
     struct.pack('<9h2x3i', *([250, 100, 500] * 3), 0, 0, 0)),
    ('game.effect_update', 0x80056268, 'floor_deform_segments',
     bytes(value for segment in SEGMENTS for value in segment)),
)
PAIR_SITES = (
    (0x80033F8C, 0x80056208, 0), (0x800344D8, 0x80056218, 0),
    (0x80034704, 0x800561D0, 0), (0x80034D94, 0x80056238, 8),
    (0x80034DA4, 0x80056238, 5), (0x80034DB8, 0x80056238, 9),
    (0x80034DC4, 0x80056238, 10), (0x80036D74, 0x80056248, 0),
    (0x80036DCC, 0x80056248, 0), (0x80038538, 0x80056268, 0),
)


class GameMapScriptDataTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def built_pair(self, name):
        unit = load_manifest().by_name()[name]
        paths = [BUILD / prefix / unit.object_name
                 for prefix in ('delink/game/modules', 'objdiff/game/base')]
        if not all(path.is_file() for path in paths):
            self.skipTest('freshly built map-script source and target objects required')
        return unit, paths

    def test_six_complete_private_owners_and_no_interior_aliases(self):
        manifest, identities = load_manifest(), load_data_identities(RETAIL_CONFIG)
        headers = '\n'.join(p.read_text() for p in (REPO / 'include/kf').glob('*.h'))
        for owner, va, name, payload in OWNERS:
            claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                      for d in u.data if d.va == va]
            self.assertEqual(len(claims), 1)
            unit, datum = claims[0]
            self.assertEqual((unit, datum.symbol, datum.size, datum.storage, datum.scope),
                             (owner, name, len(payload), 'load', 'static'))
            identity = identities['GAME.EXE', va]
            self.assertEqual((identity.name, identity.size, identity.scope, identity.storage),
                             (name, len(payload), 'static', 'load'))
            self.assertEqual([address for image, address in identities
                              if image == 'GAME.EXE' and va < address < va + len(payload)], [])
            self.assertNotIn(name, headers)
        for old in ('DAT_800561c8', 'DAT_80056238', 'DAT_80056247', 'DAT_80056248'):
            self.assertNotIn(old, headers)
        fields = [r for r in read_tsv(RETAIL_CONFIG / 'structure_fields.tsv')[1]
                  if r['structure'] == 'KfFloorDeformSegment']
        self.assertEqual(len(fields), 7)
        self.assertEqual([int(r['offset'], 0) for r in fields], list(range(7)))
        self.assertTrue(all(r['datatype'] == 'u8' and int(r['size'], 0) == 1 for r in fields))

    def test_census_split_preserves_every_neighbor_byte(self):
        rows = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]
                if r['image'] == 'GAME.EXE' and 0x800561B0 <= int(r['va'], 0) < 0x8005630C]
        expected = [(0x800561B0, 30), (0x800561CE, 2)]
        expected += [(va, len(p)) for _, va, _, p in OWNERS]
        expected += [(0x8005628B, 1), (0x8005628C, 128)]
        self.assertEqual([(int(r['va'], 0), int(r['size'], 0)) for r in rows], expected)
        self.assertEqual(sum(size for _, size in expected), 348)
        self.assertTrue(all(va + size == next_va for (va, size), (next_va, _)
                            in zip(expected, expected[1:])))
        image = self.retail()
        self.assertEqual(image.require(0x800561C8, 6), bytes((0, 0, 36, 4, 7, 1)))
        self.assertEqual(image.require(0x800561CE, 2), b'\x01\x01')
        self.assertEqual(rows[1]['kind'], 'unclassified')
        for site in (0x800343C8, 0x80036438):
            self.assertEqual(struct.unpack('<2I', image.require(site, 8)),
                             (0x0C00C2A6, 0x34040004))  # jal copy; li a0,4
        self.assertEqual(image.require(0x80056247, 1), b'\0')
        self.assertEqual(image.require(0x8005628B, 1), b'\x64')
        self.assertEqual(rows[-2]['kind'], 'unclassified')
        self.assertEqual(rows[-1]['kind'], 'unclassified')

    def test_full_initializers_compiler_sizes_and_local_binding(self):
        image = self.retail()
        for name in dict.fromkeys(owner for owner, _, _, _ in OWNERS):
            unit, paths = self.built_pair(name)
            selected = [(va, symbol, payload) for owner, va, symbol, payload in OWNERS
                        if owner == name]
            payload = b''.join(p for _, _, p in selected)
            self.assertEqual(image.require(selected[0][0], len(payload)), payload)
            for path in paths:
                with path.open('rb') as stream:
                    elf = ELFFile(stream)
                    section = elf.get_section_by_name('.data')
                    self.assertEqual(section['sh_size'], len(payload))
                    self.assertEqual(section.data(), payload)
                    for va, symbol, data in selected:
                        symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(symbol)
                        self.assertEqual(len(symbols or ()), 1)
                        row = symbols[0]
                        self.assertEqual((row['st_size'], row['st_value'], row['st_info']['bind']),
                                         (len(data), va - selected[0][0], 'STB_LOCAL'))
            metadata = json.loads((paths[1].parent / f'{unit.object_name}.json').read_text())
            sizes = metadata['data_symbol_sizes']
            self.assertEqual(sizes['method'], 'pinned-compiler-sizeof-probe')
            self.assertEqual(sizes['sizes'], {s: len(p) for _, s, p in selected})

    def test_equal_sections_satisfy_native_aspsx_placement(self):
        for name in ('game.map_scripts', 'game.player_warp', 'game.effect_update'):
            unit, paths = self.built_pair(name)
            self.assertEqual(_diff_init_section('.data', *(Elf(p) for p in paths)).status, 'match')
            result = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
            datum = next(d for d in result.diffs if d.name == '.data')
            self.assertEqual(datum.status, 'match')
            with paths[1].open('rb') as stream:
                alignment = ELFFile(stream).get_section_by_name('.data')['sh_addralign']
                self.assertEqual(alignment, 4)
                self.assertEqual(unit.data[0].va % alignment, 0)

    def test_twelve_reviewed_pairs_roundtrip_and_sdk_candidate_stays_candidate(self):
        image, ctx, catalog = self.retail(), Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        names = {va: name for _, va, name, _ in OWNERS}
        names[0x80057B88] = 'effect_swing_probe_offsets'
        pairs = PAIR_SITES + ((0x8003A054, 0x80057B88, 0), (0x8003A06C, 0x80057B88, 8))
        for site, va, addend in pairs:
            row, name = rows['GAME.EXE', site], names[va]
            self.assertEqual((row['target_name'], row['status'], int(row['target_va'], 0)),
                             (name, 'reviewed', va + addend))
            owner = ctx.idx.function_owner(site)
            body = bytearray(image.require(owner.va, owner.body_size))
            relocs, used = _apply_relocation(
                body, catalog.function_starts['GAME.EXE'][owner.va], row, catalog, 'safe')
            self.assertEqual([r.symbol for r in relocs], [name, name])
            self.assertEqual(int(used['addend'], 0), addend)
            high, low = struct.unpack_from('<2I', body, site - owner.va)
            self.assertEqual(decode_hi_lo_target(high, low), addend)
            self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(high, low, va + addend)),
                             image.require(site, 8))
        sdk = rows['GAME.EXE', 0x8003ADF8]
        self.assertEqual((sdk['status'], int(sdk['target_va'], 0)), ('candidate', 0x8005628C))

    def test_bounded_retail_path_mutation_includes_nul_and_preserves_neighbors(self):
        image, symbols = self.retail(), GameSymbols.load()
        path = MemoryRange('path', 0x80056238, 16)
        guard = MemoryRange('guard', path.address - 8, 32)
        player = symbols.datum('player_state')[0]
        for floor in range(1, 6):
            for group in (0, 1):
                for index in (0, 1, 9, 10, 19, 49, 90, 99, 255):
                    with self.subTest(floor=floor, group=group, index=index):
                        expected = bytearray(b'KAN\\B0\\K000.TIM\0')
                        expected[5] = floor + 48
                        expected[8:11] = bytes((group + 48, index // 10 + 48, index % 10 + 48))
                        observed = []

                        def screen(ctx):
                            observed.append((ctx.args[0], ctx.read(ctx.args[0], 16)))
                            return HookReturn(0)

                        program = RetailProgram.link(symbols, ['map_show_screen_image'], hooks=[
                            ExternalHook('screen_show_image_until_input', screen)])
                        result = ParserMachine(image, program).call(
                            'map_show_screen_image', args=[group, index],
                            memory=[MemoryInput(player + 10, bytes([floor]))],
                            capture=[guard], allowed_writes=[path], instruction_limit=500)
                        self.assertEqual(observed, [(path.address, expected)])
                        self.assertEqual(result.memory_by_name()['guard'],
                                         image.require(guard.address, 8) + expected
                                         + image.require(path.address + 16, 8))

    def test_bounded_retail_floor_lines_advance_progress_on_every_cell(self):
        image, symbols = self.retail(), GameSymbols.load()
        grid = MemoryRange('grid', *symbols.datum('map_floor_height_grid'))
        guard = MemoryRange('guard', grid.address - 8, grid.size + 16)
        table = MemoryRange('table-neighbors', 0x80056260, 52)
        sound = symbols.datum('gameplay_sound_ref_4')[0]
        for index, segment in enumerate(SEGMENTS):
            for start in (-800, -1, 0, 1, 3899, 3900, 3901, 4096, 4097, 4899):
                for step in (-800, -1, 0, 1, 800):
                    with self.subTest(index=index, start=start, step=step):
                        fill = bytes([0xA5]) * guard.size
                        expected = bytearray(fill)
                        calls, observed = [], []
                        col, row, dc, dr, count, low, high = segment
                        for cell in range(count):
                            progress = start + cell * step
                            if 3900 <= progress <= 4096 and progress < abs(step) + 3900:
                                calls.append((sound, (2000 * (col & 255) + 1000,
                                                      -high * 100,
                                                      2000 * (row & 255) + 1000), 127))
                            height = (((high - low) * min(4096, max(0, progress))) >> 12) + low
                            expected[8 + (row & 255) * 100 + (col & 255)] = height
                            col += dc
                            row += dr

                        def play(ctx):
                            observed.append((ctx.args[0], struct.unpack('<3i', ctx.read(ctx.args[1], 12)),
                                             ctx.args[2]))
                            return HookReturn(0)

                        program = RetailProgram.link(symbols, ['effect_floor_deform_line'], hooks=[
                            ExternalHook('audio_play_spatial_default_range', play)])
                        result = ParserMachine(image, program).call(
                            'effect_floor_deform_line', args=[index, start, step],
                            memory=[MemoryInput(guard.address, fill)], capture=[guard, table],
                            allowed_writes=[grid], instruction_limit=5000)
                        self.assertEqual(result.memory_by_name()['guard'], expected)
                        self.assertEqual(result.memory_by_name()['table-neighbors'],
                                         image.require(table.address, table.size))
                        self.assertEqual(observed, calls)
