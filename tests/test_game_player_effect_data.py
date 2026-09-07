"""Bounded player-effect retail witnesses; not whole-player-update closure."""

from __future__ import annotations

import hashlib
import re
import struct
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import _apply_relocation, load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import (
    CandidateFunction, CandidateProgram, ExternalHook, GameSymbols, HookReturn,
    MemoryInput, MemoryRange, ParserMachine, RetailProgram,
)
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, encode_hi_lo_addend
from scripts.kf.retail import read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.image import RetailImage


FUNCTION = 'player_update'
# Reviewed direct callees. Non-selected paths must fail instead of quietly
# acquiring a permissive game/SDK provider when the input fixture changes.
CALLEES = (
    'player_death_update', 'player_death_update_reverse_fade',
    'collision_adjust_cell_occupancy', 'PadRead', 'display_show_error_screen',
    'menu_enter_mode', 'player_use_item', 'pool_release_all', 'audio_close_vab',
    'func_800365f8', 'player_sync_position_to_map', 'player_equip_weapon',
    'player_select_magic', 'map_interaction_dispatch', 'SquareRoot0',
    'player_move_horizontal', 'player_update_view_bob', 'player_begin_weapon_attack',
    'magic_cast', 'fixed6_ratio_step', 'matrix_set_rotation_yxz', 'ApplyMatrix',
    'actor_pool_find_target_in_cone', 'rand', 'pitch_yaw_to_forward_vector',
    'vector3s_scale_shift12', 'effect_pool_construct', 'player_update_vertical_motion',
    'player_update_weapon_attack', 'lighting_set_active_color_matrix',
    'lighting_set_color_matrix', 'fog_interpolate_near', 'SetColorMatrix',
    'fog_set_near', 'player_death_begin', 'player_adjust_hp', 'player_adjust_mp',
    'player_apply_damage', 'player_recalculate_combat_stats',
    'lighting_apply_weapon9_environment', 'lighting_apply_timed_player_effect',
)
PASSIVE = frozenset((
    'player_death_update', 'player_death_update_reverse_fade',
    'collision_adjust_cell_occupancy', 'player_update_weapon_attack',
    'lighting_set_active_color_matrix', 'lighting_set_color_matrix',
    'fog_interpolate_near', 'SetColorMatrix', 'fog_set_near', 'player_death_begin',
))
OFFSETS = ((0, 0, 0, 0), (-32, 0, -32, 0), (-64, 0, -64, 0),
           (-48, 0, -32, 0), (-32, 0, 0, 0), (-16, 0, 32, 0),
           (0, 0, 64, 0), (-16, 0, 32, 0))
OWNERS = (
    ('game.player_death', 0x80055810, 'player_sound_refs', 9, 'global',
     '40d624c1dafc7bd232bafeb0eef6328b279c4bb15f62705bf3e1e6358e547f3c'),
    ('game.player_update', 0x80055858, 'player_darkness_color_matrix', 32, 'static',
     '1dd66d0b6b8d902cb46da4a6fca05fb6f58a1c6b822c6a550cd5c8f934c8257f'),
    ('game.player_update', 0x80055878, 'player_damage_camera_offsets', 64, 'static',
     '8cd244854dbbe4485fcebd2a0d517e42c1778c390a679ea0f15f5718765612b3'),
)
REFERENCES = (
    (0x80015188, 0x80055810, 'player_sound_refs', 3),
    (0x80016240, 0x80055810, 'player_sound_refs', 6),
    (0x80016B5C, 0x80055810, 'player_sound_refs', 0),
    (0x800199BC, 0x80055858, 'player_darkness_color_matrix', 0),
    (0x800199F0, 0x80055858, 'player_darkness_color_matrix', 0),
    (0x80019A44, 0x80055878, 'player_damage_camera_offsets', 0),
    (0x80019AA0, 0x80055878, 'player_damage_camera_offsets', 3),
    (0x80019AB0, 0x80055878, 'player_damage_camera_offsets', 0),
    (0x80019AC0, 0x80055878, 'player_damage_camera_offsets', 7),
    (0x80019AD0, 0x80055878, 'player_damage_camera_offsets', 4),
)


def effect_hook(ctx):
    if ctx.call.name == 'PadRead':
        return HookReturn(0x40)
    if ctx.call.name == 'menu_enter_mode':
        return HookReturn(-4)
    if ctx.call.name in PASSIVE:
        return HookReturn(0)
    raise AssertionError(f'unexpected path into {ctx.call.name}')


def effect_programs(symbols, object_path=None):
    hooks = [ExternalHook(name, effect_hook) for name in CALLEES]
    programs = [RetailProgram.link(symbols, [FUNCTION], hooks=hooks)]
    if object_path is not None:
        # Used for the separately built CFG experiment before DATA ownership.
        # No scattered-section or symbol overrides are allowed here.
        programs.append(CandidateProgram.link(
            symbols, [CandidateFunction(FUNCTION, object_path)], hooks=hooks))
    return programs


def effect_call(image, symbols, program, timer, flags, state=0, hp=100):
    player = bytearray(0xE0)
    struct.pack_into('<H', player, 0x12, hp)
    struct.pack_into('<H', player, 0x2A, flags)
    struct.pack_into('<6h', player, 0x48, -1, timer, -1, -1, -1, -1)
    struct.pack_into('<h', player, 0x70, -1)
    for offset in (0x5C, 0x64, *range(0x90, 0x96)):
        player[offset] = 0xFF
    player[0x9A:0xA2] = bytes.fromhex('2211443366558877')
    player[0xA2] = state
    extent = MemoryRange('player', *symbols.datum('player_state'))
    previous = MemoryRange('previous_input', *symbols.datum('player_previous_input'))
    return ParserMachine(image, program).call(
        FUNCTION, memory=[MemoryInput(extent.address, bytes(player))],
        capture=[extent, previous], allowed_writes=[extent, previous],
        instruction_limit=1000)


def expected_fade(timer, flags):
    if timer == -1:
        return -1, flags, [('fog_set_near', (11000,))]
    if not flags & 2 and timer >= 33:
        timer = 32
    timer -= 1
    if timer == -1:
        return timer, flags & ~2, []
    fade = timer - 968 if timer >= 968 else 32 - timer
    if fade >= 0:
        return timer, flags, [
            ('lighting_set_color_matrix', (0x80055858, 0x80055DBC, fade << 7)),
            ('fog_interpolate_near', (5000, 11000, fade << 7)),
        ]
    return timer, flags, [('SetColorMatrix', (0x80055858,)), ('fog_set_near', (5000,))]


def observed_fade(result):
    widths = {'fog_set_near': 1, 'SetColorMatrix': 1,
              'lighting_set_color_matrix': 3, 'fog_interpolate_near': 3}
    return [(call.name, call.args[:widths[call.name]]) for call in result.trace
            if call.name in widths]


class GamePlayerEffectDataTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def test_complete_owners_shared_declaration_and_private_scope(self):
        manifest, identities = load_manifest(), load_data_identities(RETAIL_CONFIG)
        census = [r for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1] if r['image'] == 'GAME.EXE']
        for unit, va, name, size, scope, _digest in OWNERS:
            claims = [(u.unit, d) for u in manifest.units if u.image == 'GAME.EXE'
                      for d in u.data if d.va == va]
            self.assertEqual(len(claims), 1)
            owner, datum = claims[0]
            self.assertEqual((owner, datum.symbol, datum.size, datum.storage, datum.scope),
                             (unit, name, size, 'load', scope))
            identity = identities['GAME.EXE', va]
            self.assertEqual((identity.name, identity.size, identity.scope), (name, size, scope))
            overlaps = [r for r in census if int(r['va'], 0) < va + size
                        and va < int(r['va'], 0) + int(r['size'], 0)]
            self.assertEqual([(int(r['va'], 0), int(r['size'], 0)) for r in overlaps], [(va, size)])
        gap = next(r for r in census if int(r['va'], 0) == 0x80055819)
        self.assertEqual((gap['kind'], int(gap['size'], 0)), ('unclassified', 3))
        self.assertNotIn(('GAME.EXE', 0x80055819), identities)
        self.assertIn('extern SoundRef player_sound_refs[3];',
                      (REPO / 'include/kf/game_player.h').read_text())
        headers = '\n'.join(p.read_text() for p in (REPO / 'include/kf').glob('*.h'))
        for name in ('DAT_80055858', 'DAT_80055878', *[owner[2] for owner in OWNERS[1:]]):
            self.assertNotIn(name, headers)

    def test_literal_initializers_reproduce_all_bytes_including_sdk_lanes(self):
        units = load_manifest().by_name()
        for unit, _va, name, size, _scope, digest in OWNERS:
            source = units[unit].source_path.read_text()
            initializer = re.search(name + r'(?:\[[^\]]+\])? = \{(.*?)\};', source, re.S)
            self.assertIsNotNone(initializer)
            values = [int(v) for v in re.findall(r'-?\d+', initializer[1])]
            fmt = {9: '<9B', 32: '<9h2x3i', 64: '<32h'}[size]
            payload = struct.pack(fmt, *values)
            self.assertEqual(hashlib.sha256(payload).hexdigest(), digest)
            if size == 64:
                self.assertEqual(tuple(struct.iter_unpack('<4h', payload)), OFFSETS)

    def test_complete_compiled_and_delinked_objects_preserve_typed_payloads(self):
        image, units = self.retail(), load_manifest().by_name()
        for unit, va, name, size, scope, digest in OWNERS:
            self.assertEqual(hashlib.sha256(image.require(va, size)).hexdigest(), digest)
            for directory in (BUILD / 'objdiff/game/base', BUILD / 'delink/game/modules'):
                path = directory / units[unit].object_name
                if not path.is_file():
                    self.skipTest('freshly built player source and target objects required')
                with path.open('rb') as stream:
                    elf = ELFFile(stream)
                    symbols = elf.get_section_by_name('.symtab').get_symbol_by_name(name)
                    self.assertEqual(len(symbols or ()), 1)
                    symbol = symbols[0]
                    self.assertEqual((symbol['st_size'], symbol['st_info']['bind']),
                                     (size, 'STB_GLOBAL' if scope == 'global' else 'STB_LOCAL'))
                    section, start = elf.get_section(symbol['st_shndx']), symbol['st_value']
                    self.assertEqual(section.name, '.data')
                    self.assertEqual(section.data()[start:start + size], image.require(va, size))

    def test_every_reviewed_reference_round_trips_without_changed_targets(self):
        image = self.retail()
        ctx, catalog = Context('GAME.EXE'), load_catalog(RETAIL_CONFIG)
        rows = {(r['image'], int(r['site_va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]}
        for site, va, name, addend in REFERENCES:
            row = rows['GAME.EXE', site]
            self.assertEqual((row['target_name'], int(row['target_va'], 0), row['status']),
                             (name, va + addend, 'reviewed'))
            reference = next(r for r in ctx.refs.incoming(ctx.idx.datum(va), confirmed_only=True)
                             if r.site == site)
            self.assertEqual((reference.target, reference.destination, reference.referent.name),
                             (va + addend, va, name))
            owner = ctx.idx.function_owner(site)
            body = bytearray(image.require(owner.va, owner.body_size))
            relocs, used = _apply_relocation(body, catalog.function_starts['GAME.EXE'][owner.va],
                                            row, catalog, 'safe')
            self.assertEqual([r.symbol for r in relocs], [name, name])
            self.assertEqual(int(used['addend'], 0), addend)
            high, low = struct.unpack_from('<2I', body, site - owner.va)
            self.assertEqual(decode_hi_lo_target(high, low), addend)
            self.assertEqual(struct.pack('<2I', *encode_hi_lo_addend(high, low, va + addend)),
                             image.require(site, 8))

    def test_retail_status1_full_nominal_timer_range_and_cancellation(self):
        image, symbols = self.retail(), GameSymbols.load()
        program = effect_programs(symbols)[0]
        for flags in (0, 2):
            for timer in range(-1, 1001):
                with self.subTest(timer=timer, flags=flags):
                    result = effect_call(image, symbols, program, timer, flags)
                    expected_timer, expected_flags, calls = expected_fade(timer, flags)
                    player = result.memory_by_name()['player']
                    self.assertEqual(struct.unpack_from('<h', player, 0x4A)[0], expected_timer)
                    self.assertEqual(struct.unpack_from('<H', player, 0x2A)[0], expected_flags)
                    self.assertEqual(observed_fade(result), calls)
                    self.assertEqual(result.memory_by_name()['previous_input'], b'\x40\0\0\0')

    def test_retail_damage_states_copy_the_complete_eight_byte_record(self):
        image, symbols = self.retail(), GameSymbols.load()
        program = effect_programs(symbols)[0]
        for state in range(256):
            for hp in (0, 100):
                with self.subTest(state=state, hp=hp):
                    result = effect_call(image, symbols, program, -1, 0, state, hp)
                    player = result.memory_by_name()['player']
                    if state in (0, 254, 255):
                        offset = bytes.fromhex('2211443366558877')
                        next_state = state
                    else:
                        offset = struct.pack('<4h', *OFFSETS[state if state < 8 else 0])
                        next_state = state + 1 if state < 8 else 0
                    self.assertEqual(player[0x9A:0xA2], offset)
                    self.assertEqual(player[0xA2], next_state)
                    calls = [call.name for call in result.trace]
                    self.assertEqual('player_death_begin' in calls, 8 <= state < 254 and hp == 0)
                    if state in (254, 255):
                        self.assertEqual(calls, [FUNCTION, 'player_death_update_reverse_fade'
                                                if state == 254 else 'player_death_update'])


if __name__ == '__main__':
    unittest.main()
