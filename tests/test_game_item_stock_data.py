"""Complete GAME item-stock owner, banks and referents; not placement closure."""

from __future__ import annotations

import json
import re
import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.data_match import Elf, _diff_bss, diff_unit
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


STOCK = 0x800652A8
SEEDS = (
    0x00, 0x2B, 0x50, 0x51, 0x52, 0x5D, 0x5E, 0x64, 0x6A, 0x6B, 0x70,
    0x73, 0x7B, 0x7C, 0x7D, 0x7F, 0xA2, 0xA3, 0xA6, 0xAE, 0xAF, 0xB3,
    0xB5, 0xB6, 0xBC, 0xC5, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD4,
)
PAIR_SITES = (
    0x80015374, *range(0x80015398, 0x800154A0, 8), 0x800154D8,
    0x800182C0, 0x800183B8, 0x8002159C, 0x800215C8, 0x80021678,
    0x80021AAC, 0x80021AC4, 0x80021B64, 0x80022030, 0x8002220C,
    0x80022224, 0x80022650, 0x80022658, 0x8002391C, 0x80024A08,
    0x80027B3C, 0x80027D9C, 0x8002B974, 0x8002C16C, 0x800343E0,
    0x80034610, 0x800346E0, 0x80034AD8, 0x80034B20, 0x80034B80,
    0x80034C30, 0x80034C78, 0x80036440, 0x80036454,
)


class GameItemStockDataTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def built_pair(self):
        unit = load_manifest().by_name()['game.player_death']
        paths = [BUILD / prefix / unit.object_name
                 for prefix in ('delink/game/modules', 'objdiff/game/base')]
        if not all(path.is_file() for path in paths):
            self.skipTest('freshly built item-stock source and target objects required')
        return unit, paths

    def test_one_complete_owner_and_shared_save_shape(self):
        manifest, identities = load_manifest(), load_data_identities(RETAIL_CONFIG)
        claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                  for d in u.data if d.va == STOCK]
        self.assertEqual(len(claims), 1)
        owner, datum = claims[0]
        self.assertEqual((owner, datum.symbol, datum.size, datum.storage, datum.scope),
                         ('game.player_death', 'item_stock', 240, 'bss', 'global'))
        identity = identities['GAME.EXE', STOCK]
        self.assertEqual((identity.name, identity.size, identity.datatype,
                          identity.storage, identity.scope),
                         ('item_stock', 240, 'u8[3][80]', 'bss', 'global'))
        self.assertEqual([address for image, address in identities
                          if image == 'GAME.EXE' and STOCK < address < STOCK + 240], [])
        self.assertIn('extern u8 item_stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];',
                      (REPO / 'include/kf/item.h').read_text())
        self.assertNotIn('item_stock', (REPO / 'include/kf/game_state.h').read_text())
        self.assertIn('u8 item_stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];',
                      (REPO / 'include/kf/game_save.h').read_text())
        self.assertIn('KfSavePayload\t0x2440\t0xf0\titem_stock\tu8[3][80]\t',
                      (RETAIL_CONFIG / 'structure_fields.tsv').read_text())
        source = manifest.by_name()[owner].source_path.read_text()
        self.assertNotRegex(source, r'\bu8\s+item_stock\[[^\]]+\]\[[^\]]+\]\s*=')
        item_constants = {
            name: int(value, 0) for name, value in re.findall(
                r'\b(KF_ITEM_\w+)\s*=\s*(0x[0-9a-f]+|\d+)\b',
                (REPO / 'include/kf/item.h').read_text())
        }
        seeds = [(item_constants[bank], item_constants[slot])
                 for bank, slot in re.findall(
                     r'item_stock\[(KF_ITEM_STOCK_\w+)\]'
                     r'\[KF_ENUM_ENCODE\(u8, (KF_ITEM_\w+)\)\] = 1;', source)]
        self.assertEqual([bank * 80 + slot for bank, slot in seeds], list(SEEDS))
        self.assertTrue(all(0 <= bank < 3 and 0 <= slot < 80 for bank, slot in seeds))

    def test_independent_compiler_size_and_complete_nobits_extent(self):
        image, (unit, paths) = self.retail(), self.built_pair()
        self.assertFalse(image.contains(STOCK, 240))
        for path in paths:
            with path.open('rb') as stream:
                elf = ELFFile(stream)
                symbols = elf.get_section_by_name('.symtab').get_symbol_by_name('item_stock')
                self.assertEqual(len(symbols or ()), 1)
                symbol = symbols[0]
                self.assertEqual((symbol['st_size'], symbol['st_info']['bind']),
                                 (240, 'STB_GLOBAL'))
                section = elf.get_section(symbol['st_shndx'])
                self.assertEqual((section.name, section['sh_type'], section['sh_size']),
                                 ('.bss', 'SHT_NOBITS', 280))
                self.assertLessEqual(symbol['st_value'] + symbol['st_size'], section['sh_size'])
        metadata = json.loads((BUILD / 'objdiff/game/base' / f'{unit.object_name}.json').read_text())
        self.assertEqual(metadata['data_symbol_sizes']['method'], 'pinned-compiler-sizeof-probe')
        self.assertEqual(metadata['data_symbol_sizes']['sizes']['item_stock'], 240)

    def test_native_alignment_does_not_waive_conflicting_allocation_order(self):
        unit, paths = self.built_pair()
        target, source = [Elf(path) for path in paths]
        # The two newly owned death snapshots expose a different tentative
        # allocation order; equal section lengths must not hide that failure.
        self.assertEqual(_diff_bss(target, source).status, 'layout')
        with paths[1].open('rb') as stream:
            section = ELFFile(stream).get_section_by_name('.bss')
            self.assertEqual(section['sh_addralign'], 4)
            self.assertEqual(STOCK % section['sh_addralign'], 0)
        result = diff_unit(unit, BUILD / 'delink', BUILD / 'objdiff')
        self.assertFalse(result.matches)
        bss = next(diff for diff in result.diffs if diff.name == '.bss')
        self.assertEqual(bss.status, 'layout')
        self.assertIn('conflicting-section-bases', bss.detail)

    def test_all_64_reviewed_pairs_keep_owner_and_interior_addends(self):
        image, ctx, catalog = self.retail(), Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        references = ctx.refs.incoming(ctx.idx.datum(STOCK), confirmed_only=True)
        self.assertEqual(len(PAIR_SITES), 64)
        self.assertEqual({r.site for r in references}, set(PAIR_SITES))
        for reference in references:
            site, row = reference.site, rows['GAME.EXE', reference.site]
            target = int(row['target_va'], 0)
            self.assertTrue(STOCK <= target < STOCK + 240)
            self.assertEqual((row['target_name'], row['status']), ('item_stock', 'reviewed'))
            self.assertEqual((reference.target, reference.destination, reference.referent.name),
                             (target, STOCK, 'item_stock'))
            owner = ctx.idx.function_owner(site)
            body = bytearray(image.require(owner.va, owner.body_size))
            relocs, used = _apply_relocation(
                body, catalog.function_starts['GAME.EXE'][owner.va], row, catalog, 'safe')
            self.assertEqual([r.symbol for r in relocs], ['item_stock', 'item_stock'])
            self.assertEqual(int(used['addend'], 0), target - STOCK)
            high, low = struct.unpack_from('<2I', body, site - owner.va)
            self.assertEqual(decode_hi_lo_target(high, low), target - STOCK)
            self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(high, low, target)),
                             image.require(site, 8))

    def test_bounded_retail_initializer_clears_and_seeds_all_three_banks(self):
        image, symbols = self.retail(), GameSymbols.load()
        hooks = [ExternalHook(name, lambda ctx: HookReturn(0)) for name in
                 ('player_equip_weapon', 'player_set_equipment_slot', 'player_select_magic')]
        # Only this retail body executes. Do not relocate a source BSS section
        # to an address that violates its actual compiler alignment.
        program = RetailProgram.link(symbols, ['game_state_initialize'], hooks=hooks)
        stock = MemoryRange('stock', STOCK, 240)
        guard = MemoryRange('guarded-stock', STOCK - 8, 256)
        player = MemoryRange('player', *symbols.datum('player_state'))
        world = MemoryRange('world', symbols.datum('map_runtime_state')[0] + 0x22C, 0x2134)
        expected = bytearray(240)
        for offset in SEEDS:
            expected[offset] = 1
        for fill in (0x55, 0xAA):
            with self.subTest(fill=fill):
                result = ParserMachine(image, program).call(
                    'game_state_initialize',
                    memory=[MemoryInput(guard.address, bytes([fill]) * guard.size),
                            MemoryInput(player.address, bytes([fill]) * player.size),
                            MemoryInput(world.address, bytes([fill]) * world.size),
                            MemoryInput(symbols.datum('player_level_growth_table')[0], bytes(480))],
                    capture=[guard, world], allowed_writes=[stock, world, player],
                    instruction_limit=50000)
                self.assertEqual(result.memory_by_name()['guarded-stock'],
                                 bytes([fill]) * 8 + expected + bytes([fill]) * 8)
                self.assertEqual(result.memory_by_name()['world'], bytes(0x2134))
                calls = [(call.name, call.args[:2] if call.name == 'player_set_equipment_slot'
                          else call.args[:1]) for call in result.trace if call.kind == 'hook']
                self.assertEqual(calls, [('player_equip_weapon', (0,)),
                                         ('player_set_equipment_slot', (0, 0xFF)),
                                         ('player_select_magic', (8,))])

    def test_bounded_retail_buy_panel_selects_shop_bank_and_caps_player_stack(self):
        image, symbols = self.retail(), GameSymbols.load()
        stock = MemoryRange('stock', STOCK, 240)
        for bank in (1, 2):
            for owned in (0, 98, 99):
                expected = [52, 70, 2] if owned < 99 else [70, 2]
                for selection, item in enumerate(expected):
                    with self.subTest(bank=bank, owned=owned, selection=selection):
                        payload = bytearray(240)
                        payload[52] = owned
                        for slot, count in ((52, 3), (70, 5), (2, 4)):
                            payload[bank * 80 + slot] = count
                        payload[(3 - bank) * 80 + 42] = 7
                        context = []

                        def initialize(ctx):
                            data = bytearray(40)
                            data[29] = selection
                            ctx.write(ctx.args[0], bytes(data))
                            context.append(ctx.args[0])
                            return HookReturn(0)

                        def inspect_model(ctx):
                            self.assertEqual(ctx.args[0], item)
                            self.assertEqual(ctx.read(context[0] + 26, 1), bytes([len(expected)]))
                            # A model-load failure is a real early-return path;
                            # no UI loop or game execution is needed to inspect the list.
                            return HookReturn(1)

                        hooks = [ExternalHook('PadRead', lambda ctx: HookReturn(0)),
                                 ExternalHook('menu_list_init', initialize),
                                 ExternalHook('menu_frame_begin', lambda ctx: HookReturn(0)),
                                 ExternalHook('menu_load_item_model', inspect_model)]
                        program = RetailProgram.link(symbols, ['item_menu_buy'], hooks=hooks)
                        result = ParserMachine(image, program).call(
                            'item_menu_buy', args=[bank], memory=[MemoryInput(STOCK, bytes(payload))],
                            capture=[stock], allowed_writes=[], instruction_limit=10000)
                        self.assertEqual(result.memory_by_name()['stock'], bytes(payload))
                        self.assertEqual(sum(call.name == 'menu_load_item_model'
                                             for call in result.trace), 1)
