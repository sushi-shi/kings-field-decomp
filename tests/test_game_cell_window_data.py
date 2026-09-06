"""Complete GAME visibility-window owners; payload is not placement closure."""

from __future__ import annotations

import hashlib
import re
import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import Elf, _diff_bss, _diff_init_section, diff_unit
from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.local_config import configured_retail_dir
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


FIXED_DIGEST = 'd1eb30733225e518538de1525701956aa5912e263432fd3a0fea3eb98d724e95'
WINDOWS_DIGEST = 'b085bf1fbe30831d084f21d0ba52af1609ee6f721a6f9a1937a305f97cb61e72'
OWNERS = (
    ('game.render_map_cells', 0x80055E9C, 'render_fixed_cell_window', 204, 'load', 'static'),
    ('game.resources', 0x80065BE8, 'render_cell_windows', 3264, 'bss', 'global'),
    ('game.render_map_cells', 0x80095860, 'active_cell_window', 4, 'bss', 'global'),
)
REFERENCES = {
    0x80055E9C: (0x8001E874,),
    0x80065BE8: (0x8001B1DC, 0x8001E8A8),
    0x80095860: (
        0x8001E8B4, 0x8001E8C0, 0x8001E8F0, 0x8001E90C, 0x8001E958,
        0x8001F24C, 0x8001F29C, 0x8001F360, 0x8001F4D0, 0x8001F600, 0x8001F6E8,
    ),
}


class GameCellWindowDataTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def runtime_payload(self):
        try:
            directory = configured_retail_dir()
        except ValueError:
            self.skipTest('local retail files are not configured')
        paths = [directory / 'KF/COM/COM.DAT', directory / 'KF/B0/RTBL.']
        if not all(path.is_file() for path in paths):
            self.skipTest('local COM.DAT and RTBL resources required')
        container, rtbl = [path.read_bytes() for path in paths]
        second = struct.unpack_from('<I', container)[0] + 4
        self.assertEqual(second, 0x1A8)
        size = struct.unpack_from('<I', container, second)[0]
        self.assertEqual(size, 16 * 204)
        payload = container[second + 4:second + 4 + size]
        self.assertEqual(payload, rtbl)
        self.assertEqual(hashlib.sha256(payload).hexdigest(), WINDOWS_DIGEST)
        return payload

    def built_pair(self, name):
        unit = load_manifest().by_name()[name]
        paths = [BUILD / prefix / unit.object_name
                 for prefix in ('delink/game/modules', 'objdiff/game/base')]
        if not all(path.is_file() for path in paths):
            self.skipTest('freshly built visibility-window source and target objects required')
        return unit, paths

    def test_single_complete_owners_without_interior_aliases(self):
        manifest, identities = load_manifest(), load_data_identities(RETAIL_CONFIG)
        for unit, va, name, size, storage, scope in OWNERS:
            claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                      for d in u.data if d.va == va]
            self.assertEqual(len(claims), 1)
            owner, datum = claims[0]
            self.assertEqual((owner, datum.symbol, datum.size, datum.storage, datum.scope),
                             (unit, name, size, storage, scope))
            identity = identities['GAME.EXE', va]
            self.assertEqual((identity.name, identity.size, identity.storage, identity.scope),
                             (name, size, storage, scope))
            self.assertEqual([address for image, address in identities
                              if image == 'GAME.EXE' and va < address < va + size], [])
        census = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]
                  if r['image'] == 'GAME.EXE'
                  and int(r['va'], 0) < 0x80055F68
                  and int(r['va'], 0) + int(r['size'], 0) > 0x80055E9C]
        self.assertEqual([(int(r['va'], 0), int(r['size'], 0)) for r in census],
                         [(0x80055E9C, 204)])
        header = (REPO / 'include/kf/game_render.h').read_text()
        self.assertIn(
            'extern KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];', header)
        self.assertIn('extern const KfCellWindow *active_cell_window;', header)
        headers = '\n'.join(p.read_text() for p in (REPO / 'include/kf').glob('*.h'))
        self.assertNotIn('render_fixed_cell_window', headers)
        self.assertNotIn('DAT_80055e9c', headers)

    def test_fixed_initializer_accounts_for_active_grid_and_remaining_capacity(self):
        source = load_manifest().by_name()['game.render_map_cells'].source_path.read_text()
        initializer = re.search(r'render_fixed_cell_window = \{(.*?)\};', source, re.S)
        self.assertIsNotNone(initializer)
        values = [int(v) for v in re.findall(r'\d+', initializer[1])]
        self.assertEqual(values[:4], [13, 13, 6, 6])
        self.assertEqual(len(values), 4 + 13 * 13)
        payload = struct.pack('<4H', *values[:4]) + bytes(values[4:]) + bytes(196 - 169)
        self.assertEqual(len(payload), 204)
        self.assertEqual(hashlib.sha256(payload).hexdigest(), FIXED_DIGEST)
        self.assertIn('active_cell_window = &render_fixed_cell_window;', source)

    def test_runtime_chunk_is_the_same_sixteen_complete_records_as_open(self):
        payload = self.runtime_payload()
        origins = ((6, 1), (4, 3), (3, 4), (1, 6), (1, 7), (3, 9), (4, 10), (6, 12),
                   (7, 12), (9, 10), (10, 9), (12, 7), (12, 6), (10, 4), (9, 3), (7, 1))
        for index, origin in enumerate(origins):
            self.assertEqual(struct.unpack_from('<4H', payload, index * 204),
                             (14, 14, *origin))
            self.assertLessEqual(set(payload[index * 204 + 8:(index + 1) * 204]), {0, 1, 2})
        source = load_manifest().by_name()['game.resources'].source_path.read_text()
        self.assertIn('KfCellWindow render_cell_windows[KF_CELL_WINDOW_YAW_COUNT];', source)
        self.assertNotRegex(source, r'render_cell_windows\[[^\]]+\]\s*=')

    def test_complete_compiled_and_delinked_allocations(self):
        image = self.retail()
        for unit, va, name, size, storage, scope in OWNERS:
            _unit, paths = self.built_pair(unit)
            for path in paths:
                with path.open('rb') as stream:
                    elf = ELFFile(stream)
                    symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(name)
                    self.assertEqual(len(symbols or ()), 1)
                    symbol = symbols[0]
                    self.assertEqual((symbol['st_size'], symbol['st_info']['bind']),
                                     (size, 'STB_LOCAL' if scope == 'static' else 'STB_GLOBAL'))
                    section, start = elf.get_section(symbol['st_shndx']), symbol['st_value']
                    self.assertEqual(section.name, '.data' if storage == 'load' else '.bss')
                    if storage == 'load':
                        self.assertEqual(section['sh_size'], size)
                        self.assertEqual(section.data(), image.require(va, size))
                        self.assertEqual(hashlib.sha256(section.data()).hexdigest(), FIXED_DIGEST)
                    else:
                        self.assertEqual(section['sh_type'], 'SHT_NOBITS')
                        self.assertFalse(image.contains(va, size))
                        self.assertLessEqual(start + size, section['sh_size'])

    def test_payload_equality_does_not_waive_whole_section_placement_or_extent(self):
        for name in ('game.render_map_cells', 'game.resources'):
            unit, paths = self.built_pair(name)
            retail, source = [Elf(path) for path in paths]
            self.assertEqual(_diff_init_section('.data', retail, source).status, 'match')
            diffs = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
            self.assertFalse(diffs.matches)
            sections = {diff.name: diff for diff in diffs.diffs}
            if name == 'game.render_map_cells':
                self.assertEqual(sections['.data'].status, 'placement')
                self.assertIn('invalid-section-placement', sections['.data'].detail)
                self.assertEqual((sections['.bss'].status, sections['.bss'].retail_size,
                                  sections['.bss'].recon_size), ('size', 4, 8))
            else:
                self.assertEqual(_diff_bss(retail, source).status, 'match')
                self.assertEqual(sections['.bss'].status, 'placement')
                self.assertIn('invalid-section-placement', sections['.bss'].detail)

    def test_all_fourteen_reviewed_pairs_round_trip_to_original_words(self):
        image, ctx, catalog = self.retail(), Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        for _unit, va, name, _size, _storage, _scope in OWNERS:
            references = ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
            self.assertEqual({r.site for r in references}, set(REFERENCES[va]))
            for reference in references:
                site = reference.site
                row = rows['GAME.EXE', site]
                self.assertEqual((row['target_name'], int(row['target_va'], 0), row['status']),
                                 (name, va, 'reviewed'))
                self.assertEqual((reference.target, reference.destination, reference.referent.name),
                                 (va, va, name))
                owner = ctx.idx.function_owner(site)
                body = bytearray(image.require(owner.va, owner.body_size))
                relocs, used = _apply_relocation(
                    body, catalog.function_starts['GAME.EXE'][owner.va], row, catalog, 'safe')
                self.assertEqual([r.symbol for r in relocs], [name, name])
                self.assertEqual(int(used['addend'], 0), 0)
                high, low = struct.unpack_from('<2I', body, site - owner.va)
                self.assertEqual(decode_hi_lo_target(high, low), 0)
                self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(high, low, va)),
                                 image.require(site, 8))

    def test_bounded_retail_selector_publishes_and_walks_only_the_active_grid(self):
        image, payload, symbols = self.retail(), self.runtime_payload(), GameSymbols.load()
        # Only this retail function executes. The source object cannot be
        # faithfully placed yet; no section/symbol override is used to hide it.
        hooks = [ExternalHook(name, lambda ctx: HookReturn(0))
                 for name in ('tmd_select', 'render_map_cell')]
        program = RetailProgram.link(symbols, ['render_map_cells'], hooks=hooks)
        pointer = MemoryRange('pointer', *symbols.datum('active_cell_window'))
        state_address, state_size = symbols.datum('render_state')
        fixed = image.require(0x80055E9C, 204)
        cases = [(0, yaw, 50, 50) for yaw in range(16)]
        cases += [(pitch, yaw, x, z) for pitch in (-512, -511, 511, 512)
                  for yaw in (0, 15) for x, z in ((0, 0), (99, 99), (0, 99), (99, 0))]
        for pitch, yaw, x, z in cases:
            with self.subTest(pitch=pitch, yaw=yaw, x=x, z=z):
                state = bytearray(state_size)
                struct.pack_into('<hh', state, 0xB4, pitch, yaw << 8)
                struct.pack_into('<HH', state, 0xBC, x, z)
                fixed_selected = ((pitch + 511) & 0xFFFF) >= 1023
                index = 15 - yaw
                selected = 0x80055E9C if fixed_selected else 0x80065BE8 + index * 204
                window = fixed if fixed_selected else payload[index * 204:(index + 1) * 204]
                width, height, ox, oz = struct.unpack_from('<4H', window)
                expected = []
                for row in range(height):
                    for col in range(width):
                        cell = window[8 + row * width + col]
                        px, pz = x - ox + col, z - oz + row
                        if cell and 0 <= px < 100 and 0 <= pz < 100:
                            expected.append((px, pz, cell))
                result = ParserMachine(image, program).call(
                    'render_map_cells',
                    memory=[MemoryInput(state_address, bytes(state)),
                            MemoryInput(0x80065BE8, payload)],
                    capture=[pointer], allowed_writes=[pointer], instruction_limit=15000)
                self.assertEqual(result.memory_by_name()['pointer'], struct.pack('<I', selected))
                self.assertEqual([call.args[:1] for call in result.trace
                                  if call.name == 'tmd_select'], [(0,)])
                self.assertEqual([call.args[:3] for call in result.trace
                                  if call.name == 'render_map_cell'], expected)
