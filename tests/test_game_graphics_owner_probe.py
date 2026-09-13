"""GAME graphics owner, complete retail bodies, and rejected ownership controls."""

from __future__ import annotations

import os
import json
import re
import shutil
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.sema.image import RetailImage
from tests.test_open_runtime_owner_probe import linked_words


HEADER = REPO / 'tests/fixtures/game_graphics_owner_probe.h'
ORIGIN, EXTENT = 0x80070E98, 0x249CC
# Independently reviewed physical locations; not calculated by the candidate C.
FIELDS = {
    'display_state': 0x0, 'display_draw_environments': 0x20028,
    'display_disp_environments': 0x200E0, 'unknown_20108': 0x20108,
    'tmd_state': 0x20110, 'unknown_registry_20134': 0x20134,
    'current_tmd_vertices': 0x20224, 'pool_records': 0x20228,
    'tmd_projected_vertices': 0x20318, 'morph_scratch': 0x22258,
    'effect5_texture_pages': 0x241A0, 'unknown_241a6': 0x241A6,
    'effect5_texture_cluts': 0x241B0, 'unknown_241b6': 0x241B6,
    'active_render_clut': 0x241C0, 'active_render_tpage': 0x241C2,
    'active_render_red': 0x241C4, 'active_render_green': 0x241C5,
    'active_render_blue': 0x241C6, 'active_render_code': 0x241C7,
    'hud_clut': 0x241C8, 'hud_tpage': 0x241CA,
    'hud_brightness': 0x241CC, 'unknown_241cd': 0x241CD,
    'notification_text_clut': 0x241CE, 'notification_text_tpage': 0x241D0,
    'notification_digit_clut': 0x241D2, 'notification_digit_tpage': 0x241D4,
    'notification_message_ids': 0x241D6, 'notification_state': 0x241DE,
    'floor_item_clut': 0x241F4, 'floor_item_tpage': 0x241F6,
    'floor_item_count': 0x241F8, 'unknown_241fa': 0x241FA,
    'floor_items': 0x24200, 'DAT_80095698': 0x24800, 'DAT_8009569c': 0x24804,
    'render_state': 0x24808, 'light_quadrant_matrices': 0x24948,
    'active_cell_window': 0x249C8,
}
PILOT = {
    'game.game': {'game_main_loop'},
    'game.render': {'render_initialize', 'render_set_view_transform'},
    'game.render_map_cells': {'render_map_cell', 'render_map_cells'},
}


def data_addresses():
    """Current owner plus independent physical starts for separated-owner controls."""
    data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
            if key == 'GAME.EXE'}
    data.update({name: ORIGIN + offset for name, offset in FIELDS.items()
                 if not name.startswith('unknown_')})
    data.update(asset_registry_entries=0x80090FCC, tmd_projected_vertices=0x800911B0,
                morph_scratch=0x800930F0)
    return data


def standalone_source(unit):
    """Recreate the rejected separate declarations from the current whole owner.

    These extern views exist only in temporary negative controls. Production
    defines one owner and retains no overlapping globals.
    """
    source = unit.source_path.read_text()
    # Historical separate-owner controls retain their original byte fields.
    for member, old in (('r', 'red'), ('g', 'green'), ('b', 'blue'), ('cd', 'code')):
        source = source.replace('active_render_color.' + member, 'active_render_' + old)
    source = source.replace('&game_graphics_runtime.active_render_color',
                            '(CVECTOR *)(&game_graphics_runtime.active_render_clut + 2)')
    source = source.replace(
        '((KfAssetHeader **)game_graphics_runtime.unknown_registry_20134)', 'asset_registry_entries')
    source = source.replace('game_graphics_runtime.', '')
    source = source.replace('memset(&game_graphics_runtime, 0, sizeof game_graphics_runtime);',
                            'memset(&display_state.buffer_index, 0, INITIAL_GRAPHICS_CLEAR_BYTES);')
    if unit.unit == 'game.item':
        source = source.replace('    KfFloorItem *item;\n',
                                '    KfFloorItem *item;\n    u16 *count = &floor_item_count;\n', 1)
        source = source.replace('    floor_item_count = 0;', '    *count = 0;', 1)
        source = source.replace('        floor_item_count++;', '        (*count)++;', 1)
    return '#include "game_graphics_standalone.h"\n' + source


def candidate_source(unit, selected):
    """Replace only reviewed function references, using one shared declaration."""
    names = '|'.join(re.escape(name) for name in FIELDS if not name.startswith('unknown_'))
    pattern = re.compile(r'\b(' + names + r')\b')
    addresses = {claim.va for claim in unit.functions if claim.symbol in selected}

    def rewrite(match):
        if int(match.group(1), 16) not in addresses:
            return match.group(0)
        return pattern.sub(lambda item: 'graphics_owner_probe.' + item[0], match.group(0))

    source = re.sub(r'^ADDRESS\((0x[0-9a-f]+),[^\n]+\n.*?(?=^ADDRESS\(|\Z)',
                    rewrite, standalone_source(unit), flags=re.M | re.S)
    if 'game_main_loop' in selected:
        old = ('memset(&graphics_owner_probe.display_state.buffer_index, 0, '
               'INITIAL_GRAPHICS_CLEAR_BYTES);')
        assert source.count(old) == 1
        source = source.replace(old, 'memset(&graphics_owner_probe, 0, sizeof graphics_owner_probe);')
    return '#include "game_graphics_owner_probe.h"\n' + source


class GameGraphicsOwnerProbeTests(unittest.TestCase):
    def tools(self):
        if not all(shutil.which(name) for name in ('cpppsx-257', 'cc1psx-257', 'dosbox-x')):
            self.skipTest('pinned native compiler tools required')
        if not os.environ.get('PSYQ_INCLUDE'):
            self.skipTest('pinned SDK headers required')

    def retail(self):
        try:
            return RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

    def compile(self, root, unit, source):
        profile = load_manifest().profiles[unit.profile]
        path = root / unit.source_path.name
        path.write_text(source)
        output = root / unit.object_name
        # Layout-only probes need no retail target; register their scratch output
        # locally so the same compiler controls run in a clean flake build.
        delink = root / 'delink'
        index = delink / unit.image.removesuffix('.EXE').lower() / 'objects.tsv'
        index.parent.mkdir(parents=True, exist_ok=True)
        index.write_text(f'object\tscope\nmodules/{unit.object_name}\tmodule\n')
        compile_source(path, unit.image, output, delink, profile.optimization,
                       profile.small_data, profile.aspsx_version,
                       (REPO / 'include', REPO / 'vendor/include', root, HEADER.parent, Path(os.environ['PSYQ_INCLUDE'])),
                       profile.cc1_flags, profile.compiler, defines=unit.defines)
        return _load_object(output)

    def test_clear_extent_and_existing_fields_are_independent_of_candidate(self):
        image = self.retail()
        self.assertEqual(image.require(0x800146CC, 24), struct.pack(
            '<6I', 0x3C048007, 0x24840E98, 0x00002821, 0x3C060002, 0x0C0140C3, 0x34C649CC))
        self.assertEqual(ORIGIN + EXTENT, 0x80095864)
        identities = load_data_identities(RETAIL_CONFIG)
        owner = identities['GAME.EXE', ORIGIN]
        self.assertEqual((owner.name, owner.size, owner.storage),
                         ('game_graphics_runtime', EXTENT, 'bss'))
        self.assertEqual([va for image, va in identities
                          if image == 'GAME.EXE' and ORIGIN < va < ORIGIN + EXTENT], [])
        self.assertEqual(identities['GAME.EXE', ORIGIN + EXTENT + 4].name, 'audio_state')
        # Type only the observed registry prefix; keep the remaining spans opaque.
        header = (REPO / 'include/kf/game_graphics.h').read_text()
        self.assertIn('KfAssetHeader *asset_registry_entries[KF_ASSET_REGISTRY_KNOWN_ENTRIES];', header)
        self.assertIn('u8 unknown_201f4[0x30];', header)
        self.assertIn('KfScreenVertex tmd_projected_vertices[KF_PROJECTED_VERTEX_CAPACITY];', header)
        self.assertIn('SVECTOR morph_scratch[KF_MORPH_SCRATCH_CAPACITY];', header)

    def test_production_registry_prefix_preserves_the_surrounding_layout(self):
        self.tools()
        unit = load_manifest().by_name()['game.game']
        source = '''#include <kf/game_graphics.h>
            unsigned long registry_layout[] = {
                (unsigned long)&((KfGraphicsRuntimeGame *)0)->asset_registry_entries,
                sizeof(((KfGraphicsRuntimeGame *)0)->asset_registry_entries),
                (unsigned long)&((KfGraphicsRuntimeGame *)0)->unknown_201f4,
                sizeof(((KfGraphicsRuntimeGame *)0)->unknown_201f4),
                (unsigned long)&((KfGraphicsRuntimeGame *)0)->current_tmd_vertices,
                sizeof(KfGraphicsRuntimeGame)
            };
        '''
        with tempfile.TemporaryDirectory(prefix='kf-registry-layout-') as directory:
            obj = self.compile(Path(directory), unit, source)
        self.assertEqual(struct.unpack('<6I', obj.sections['.data'][:24]),
                         (0x20134, 0xc0, 0x201f4, 0x30, 0x20224, EXTENT))

    def test_registry_prefix_covers_shipped_registrations_and_event_models(self):
        self.retail()
        from scripts.kf.local_config import configured_retail_dir
        from scripts.kf.tmd_oracle import asset_archive_cases, length_prefixed_chunks

        retail = configured_retail_dir()
        registrations = []
        event_counts = []
        for floor in range(1, 6):
            path = retail / f'KF/B{floor}/MIXB.DAT'
            chunks = length_prefixed_chunks(path.read_bytes(), str(path))
            for chunk, first in ((2, 10), (3, 30), (4, 0)):
                if chunk < len(chunks):
                    count = len(asset_archive_cases(chunks[chunk], f'{path}:{chunk}'))
                    registrations.append((first, count))
            model_count = len(asset_archive_cases(chunks[2], f'{path}:2'))
            event_path = retail / f'KF/B{floor}/MIXA.DAT'
            events = length_prefixed_chunks(event_path.read_bytes(), str(event_path))[7]
            event_count = 0
            # Retail reads up to eight 24-byte definitions, stopping at state 255.
            for offset in range(0, 8 * 24, 24):
                self.assertGreaterEqual(len(events), offset + 24)
                if events[offset] == 255:
                    break
                self.assertLess(events[offset + 2], model_count)
                event_count += 1
            event_counts.append(event_count)
        for path in sorted((retail / 'KF/B5').glob('CHR*.MIM')):
            registrations.append((0, len(asset_archive_cases(path.read_bytes(), str(path)))))
        registrations.extend(((20, 1), (21, 1)))
        self.assertEqual(len(registrations), 19)
        self.assertEqual(max(first + count for first, count in registrations), 48)
        self.assertTrue(all(0 <= first < first + count <= 48
                            for first, count in registrations))
        self.assertEqual(event_counts, [6, 4, 2, 3, 2])

    def test_pinned_compiler_measures_whole_layout_and_detects_wrong_gap(self):
        self.tools()
        unit = load_manifest().by_name()['game.game']
        query = ('#include "game_graphics_owner_probe.h"\nunsigned long layout[] = {\n'
                 + ',\n'.join('(unsigned long)&((KfGraphicsOwnerProbe *)0)->' + name for name in FIELDS)
                 + ',\nsizeof(KfGraphicsOwnerProbe)};\n')
        for wrong in (False, True):
            with self.subTest(wrong=wrong), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                header = HEADER.read_text()
                if wrong:
                    header = header.replace('unknown_20108[8]', 'unknown_20108[4]')
                (root / HEADER.name).write_text(header)
                obj = self.compile(root, unit, query)
                symbol = obj.named_symbol('layout')
                measured = struct.unpack_from(f'<{len(FIELDS) + 1}I', obj.sections['.data'], symbol.value)
                expected = (*FIELDS.values(), EXTENT)
                if wrong:
                    self.assertNotEqual(measured, expected)
                    self.assertEqual(measured[-1], EXTENT - 4)
                else:
                    self.assertEqual(measured, expected)

    def test_shared_owner_pilot_and_exact_controls(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        for unit_name, selected in PILOT.items():
            unit = manifest.by_name()[unit_name]
            # Local static data in the map unit uses its genuine section base.
            addresses = {**data, '.data': unit.data[0].va}
            with self.subTest(unit=unit_name), tempfile.TemporaryDirectory() as directory:
                obj = self.compile(Path(directory), unit, candidate_source(unit, selected))
                for claim in unit.functions:
                    if claim.symbol not in selected:
                        continue
                    actual, calls, targets = linked_words(obj, unit, claim, addresses, functions)
                    expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                 image.require(claim.va, claim.body_size)))
                    if claim.symbol != 'render_map_cell':
                        self.assertEqual(actual, expected, claim.symbol)
                        shifted = dict(addresses, graphics_owner_probe=ORIGIN + 4)
                        wrong, _, _ = linked_words(obj, unit, claim, shifted, functions)
                        self.assertNotEqual(wrong, expected)
                        if claim.symbol == 'game_main_loop':
                            callback = 'frame_pacer_vsync_callback'
                            shifted_functions = {**functions, callback: functions[callback] + 4}
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, addresses, shifted_functions)
                            self.assertNotEqual(wrong, expected)
                            self.assertEqual(same_calls, calls)  # Function-pointer data, not a jal.
                        elif claim.symbol == 'render_map_cells':
                            shifted_section = {**addresses, '.data': addresses['.data'] + 4}
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, shifted_section, functions)
                            self.assertNotEqual(wrong, expected)
                            self.assertEqual(same_calls, calls)
                    else:
                        self.assertNotEqual(actual, expected)
                        self.assertEqual(len(actual) * 4, 592)
                        self.assertEqual(actual[0], 0x27BDFFA8)  # 88-byte frame, retail 120.
                        # All remaining differences are explicit stack operands,
                        # not normalized away or admitted as an exact match.
                        stack_offsets = {
                            0x0, 0x4, 0x8, 0xc, 0x10, 0x148, 0x164, 0x190,
                            0x1a4, 0x1b0, 0x1bc, 0x1e0, 0x1ec, 0x238, 0x23c,
                            0x240, 0x244, 0x24c,
                        }
                        self.assertEqual(
                            {4 * i for i, (a, b) in enumerate(zip(actual, expected)) if a != b},
                            stack_offsets,
                        )
                        for offset in stack_offsets:
                            self.assertEqual(actual[offset // 4],
                                             expected[offset // 4] + (32 if offset == 0 else -32))
                        self.assertEqual(targets, [
                            0x8009A748, 0x800A07D2, 0x80069018, 0x80095744, 0x8009574C,
                            0x80095900, 0x80095748, 0x800956A0, 0x800956A0, 0x80095760,
                        ])
                        self.assertEqual(calls, [0x8004D784, 0x8004D814, 0x8004DADC, 0x8004CDD4,
                                                0x8004D784, 0x8004D814, 0x8004D7B4, 0x8001C148, 0x8001DE18])

    def test_view_transform_matrix_rebuild_is_not_conditional(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.render']
        claim = next(c for c in unit.functions if c.symbol == 'render_set_view_transform')
        data = data_addresses()
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        correct = '''    if (rotation != NULL) {
        render_state.view_rotation = *rotation;
    }
    RotMatrix(&render_state.view_rotation, &render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;'''
        old_bug = '''    if (rotation != NULL) {
        render_state.view_rotation = *rotation;
        RotMatrix(&render_state.view_rotation, &render_state.view_matrix);
        angles.vz = 0;
        angles.vy = 0;
    }'''
        canonical = standalone_source(unit)
        self.assertEqual(canonical.count(correct), 1)
        for wrong in (False, True):
            with self.subTest(old_bug=wrong), tempfile.TemporaryDirectory() as directory:
                source = canonical.replace(correct, old_bug) if wrong else canonical
                obj = self.compile(Path(directory), unit, source)
                actual, calls, _ = linked_words(obj, unit, claim, data, functions)
                self.assertEqual(calls, [0x8004E9B8, 0x8004E9B8])
                if wrong:
                    # Merely having both SDK call sites does not prove the CFG.
                    self.assertNotEqual(actual, expected)
                    self.assertNotEqual(actual[0xB8 // 4], expected[0xB8 // 4])
                else:
                    self.assertEqual(actual, expected)

    def test_extended_display_tmd_pilot_preserves_complete_functions(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        selected = {
            'game.render': {
                'display_show_error_screen', 'lighting_set_active_color_matrix',
                'effect5_texture_cache_prepare', 'display_initialize', 'display_begin_frame',
                'display_present_frame', 'tmd_select', 'tmd_get_object', 'tmd_set_current_vertices',
                'tmd_select_object_vertices', 'tmd_prepare_primitive_indices', 'tmd_register',
                'tmd_release_last_allocation',
            },
            'game.display_play_transition': {'display_play_transition'},
            'game.menu_runtime': {'menu_present_frame'},
            'game.save_system': {'screen_show_image_until_input'},
            'game.render_frame': {'render_frame'},
            'game.geometry_render': {
                'render_weapon', 'render_effect_sprites', 'render_hud_gauges',
            },
            'game.pool': {
                'pool_reset', 'pool_mark_allocated', 'pool_release_all',
                'pool_release_stale', 'pool_allocate',
            },
        }
        controls = {'game.pool': {'render_bind_animated_instance', 'pool_record_release'}}
        pool_base_lows = {
            'pool_reset': 0x4, 'pool_mark_allocated': 0x4, 'pool_release_all': 0x18,
            'pool_release_stale': 0x1C, 'pool_allocate': 0x4,
        }
        for name, names in selected.items():
            unit = manifest.by_name()[name]
            with self.subTest(unit=name), tempfile.TemporaryDirectory() as directory:
                obj = self.compile(Path(directory), unit, candidate_source(unit, names))
                for claim in unit.functions:
                    if claim.symbol not in names | controls.get(name, set()):
                        continue
                    with self.subTest(function=claim.symbol):
                        actual, calls, targets = linked_words(obj, unit, claim, data, functions)
                        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                     image.require(claim.va, claim.body_size)))
                        expected_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                          | ((word & 0x3FFFFFF) << 2)
                                          for i, word in enumerate(expected) if word >> 26 == 3]
                        self.assertEqual(calls, expected_calls)
                        self.assertEqual(actual, expected)
                        if (claim.symbol in controls.get(name, set())
                                or any(ORIGIN <= target < ORIGIN + EXTENT for target in targets)):
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, {**data, 'graphics_owner_probe': ORIGIN + 4}, functions)
                            if claim.symbol in controls.get(name, set()):
                                self.assertEqual(wrong, expected)
                            else:
                                self.assertNotEqual(wrong, expected)
                            if claim.symbol in pool_base_lows:
                                self.assertEqual(targets, [0x800910C0])
                                self.assertEqual(
                                    [4 * i for i, (a, b) in enumerate(zip(wrong, expected)) if a != b],
                                    [pool_base_lows[claim.symbol]],
                                )
                            self.assertEqual(same_calls, calls)

    def test_floor_item_counter_requires_direct_member_access(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.item']
        claim = next(c for c in unit.functions if c.symbol == 'item_load_floor_placements')
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        for owner in (False, True):
            prefix = 'graphics_owner_probe.' if owner else ''
            source = (candidate_source(unit, {claim.symbol}) if owner
                      else standalone_source(unit))
            for direct in (False, True):
                candidate = source
                if direct:
                    declaration = f'    u16 *count = &{prefix}floor_item_count;\n'
                    self.assertEqual(candidate.count(declaration), 1)
                    self.assertEqual(candidate.count('    *count = 0;'), 1)
                    self.assertEqual(candidate.count('        (*count)++;'), 1)
                    candidate = candidate.replace(declaration, '').replace(
                        '    *count = 0;', f'    {prefix}floor_item_count = 0;').replace(
                        '        (*count)++;', f'        {prefix}floor_item_count++;')
                with self.subTest(owner=owner, direct=direct), tempfile.TemporaryDirectory() as directory:
                    obj = self.compile(Path(directory), unit, candidate)
                    actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                    self.assertEqual(calls, [0x8005049C])
                    self.assertEqual(actual == expected, owner and direct)
                    self.assertEqual(len(actual) * 4, 432 if owner and direct else 428)
                    count_uses = 3 if direct and not owner else 1
                    self.assertEqual(addresses, [0x80095090] * count_uses
                                     + [0x80095098, 0x80095900])
                    if owner:
                        wrong, same_calls, _ = linked_words(
                            obj, unit, claim, dict(data, graphics_owner_probe=ORIGIN + 4), functions)
                        self.assertNotEqual(wrong, actual)
                        self.assertEqual(same_calls, calls)

    def test_actor_byte_index_and_shared_owner_are_independent_requirements(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.entity_model_render']
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        for owner in (False, True):
            source = (candidate_source(unit, {'render_actor'}) if owner
                      else standalone_source(unit))
            for byte_index in (False, True):
                candidate = source
                if not byte_index:
                    shift = '    descriptor >>= ACTOR_MODEL_TEXTURE_SHIFT;'
                    self.assertEqual(candidate.count(shift), 1)
                    candidate = candidate.replace('    u16 asset;\n', '    u16 asset;\n    int high;\n')
                    candidate = candidate.replace(
                        shift, '    high = descriptor >> ACTOR_MODEL_TEXTURE_SHIFT;')
                    candidate = candidate.replace('if (descriptor-- == 0)', 'if (high == 0)')
                    candidate = candidate.replace('[descriptor]', '[high - 1]')
                with self.subTest(owner=owner, byte_index=byte_index), tempfile.TemporaryDirectory() as directory:
                    obj = self.compile(Path(directory), unit, candidate)
                    for claim in unit.functions:
                        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                     image.require(claim.va, claim.body_size)))
                        actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                        retail_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                        | ((word & 0x3FFFFFF) << 2)
                                        for i, word in enumerate(expected) if word >> 26 == 3]
                        self.assertEqual(calls, retail_calls)
                        if claim.symbol != 'render_actor':
                            self.assertEqual(actual, expected, claim.symbol)
                            continue
                        self.assertEqual(actual == expected, owner and byte_index)
                        self.assertEqual(len(actual) * 4, 532 if byte_index else 516 if owner else 520)
                        self.assertEqual(len(addresses), 10 if owner else 11)
                        if owner and byte_index:
                            self.assertEqual(addresses, [0x800956A0, 0x800956A0, 0x80095744,
                                                        0x80095748, 0x8009574C, 0x80055F68,
                                                        0x8006BD99, 0x80095038, 0x8009505A,
                                                        0x80095058])
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, dict(data, graphics_owner_probe=ORIGIN + 4), functions)
                            self.assertNotEqual(wrong, expected)
                            self.assertEqual(same_calls, calls)

    def test_narrow_material_owner_does_not_preserve_exact_frame(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.render_frame']
        claim = unit.functions[0]
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        data = data_addresses()
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        fields = {
            'active_render_clut': 'clut', 'active_render_tpage': 'tpage',
            'active_render_red': 'color.r', 'active_render_green': 'color.g',
            'active_render_blue': 'color.b', 'active_render_code': 'color.cd',
        }
        canonical = standalone_source(unit)
        narrow = '''#include <kf/game_render.h>
typedef struct KfMaterialProbe {
    u16 clut;
    u16 tpage;
    CVECTOR color;
} KfMaterialProbe;
typedef char check_material_size[sizeof(KfMaterialProbe) == 8 ? 1 : -1];
extern KfMaterialProbe material_probe;
''' + re.sub(r'\b(' + '|'.join(fields) + r')\b',
             lambda match: 'material_probe.' + fields[match[0]], canonical)
        data['material_probe'] = 0x80095058
        for changed, source in ((False, canonical), (True, narrow)):
            with self.subTest(narrow=changed), tempfile.TemporaryDirectory() as directory:
                obj = self.compile(Path(directory), unit, source)
                actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                retail_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                | ((word & 0x3FFFFFF) << 2)
                                for i, word in enumerate(expected) if word >> 26 == 3]
                self.assertEqual(calls, retail_calls)
                if not changed:
                    self.assertEqual(actual, expected)
                    self.assertEqual(len(addresses), 64)
                else:
                    # This rejected owner retains a texture-selector pointer
                    # across calls. A sprite-only exact probe cannot authorize it.
                    self.assertNotEqual(actual, expected)
                    self.assertEqual(len(actual) * 4, 1296)
                    self.assertEqual(actual[0], expected[0])
                    self.assertNotEqual(actual[1], expected[1])
                    self.assertEqual(len(addresses), 59)

    def test_scratch_consumers_preserve_typed_accesses_through_the_shared_owner(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        selected = {
            'game.asset_registry': {
                'asset_registry_load_tmd_archive', 'asset_registry_set',
                'asset_registry_select',
            },
            'game.render': {'tmd_project_vertices', 'tmd_project_vertices_shift',
                            'tmd_transform_vertices'},
            'game.pool': {'render_bind_animated_instance'},
        }
        for unit_name, names in selected.items():
            unit = manifest.by_name()[unit_name]
            for owner in (False, True):
                source = standalone_source(unit)
                if owner:
                    source = candidate_source(unit, names).replace(
                        'asset_registry_entries',
                        '((KfAssetHeader **)graphics_owner_probe.unknown_registry_20134)')
                with self.subTest(unit=unit_name, owner=owner), tempfile.TemporaryDirectory() as directory:
                    obj = self.compile(Path(directory), unit, source)
                    for claim in unit.functions:
                        if claim.symbol not in names:
                            continue
                        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                     image.require(claim.va, claim.body_size)))
                        actual, calls, _ = linked_words(obj, unit, claim, data, functions)
                        self.assertEqual(actual, expected, claim.symbol)
                        if owner:
                            wrong, same_calls, _ = linked_words(
                                obj, unit, claim, dict(data, graphics_owner_probe=ORIGIN + 4), functions)
                            self.assertNotEqual(wrong, expected)
                            self.assertEqual(same_calls, calls)
                        if claim.symbol == 'render_bind_animated_instance':
                            # Retrying at the call skips the real count reload;
                            # all calls still exist, so call-set equality misses it.
                            self.assertEqual(actual[0xB8 // 4], 0x97AA0018)
                            self.assertEqual(actual[0xD4 // 4] & 0x3FFFFFF,
                                             (claim.va + 0xB8) >> 2 & 0x3FFFFFF)
                            wrong = actual.copy()
                            wrong[0xD4 // 4] += 1
                            self.assertNotEqual(wrong, expected)

    def test_polygon_owner_addresses_projected_vertices_through_the_shared_owner(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.render_enqueue']
        names = {'render_enqueue_tmd', 'render_enqueue_model', 'render_enqueue_map',
                 'render_enqueue_sprite'}
        source = candidate_source(unit, names)
        # The file-scope TMD macro sits outside the rewritten function bodies;
        # route it through the shared owner too, so that the projected base and
        # the selected-asset field share one symbol as they do in production.
        macro = '#define VTX(off) ((KfScreenVertex *)((u8 *)tmd_projected_vertices + (off)))'
        self.assertIn(macro, source)
        source = source.replace(macro, macro.replace('(u8 *)tmd_projected_vertices',
                                                     '(u8 *)graphics_owner_probe.tmd_projected_vertices'))
        # The macro and the two byte cursors address the typed member of the
        # shared owner; no local recovers the owner from a member address.
        self.assertEqual(source.count('graphics_owner_probe.tmd_projected_vertices'), 3)
        self.assertNotIn('NULL)->', source)
        data = data_addresses()
        data['graphics_owner_probe'] = ORIGIN
        self.assertEqual(ORIGIN + FIELDS['tmd_projected_vertices'], 0x800911B0)
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        with tempfile.TemporaryDirectory() as directory:
            obj = self.compile(Path(directory), unit, source)
            for claim in unit.functions:
                if claim.symbol not in names:
                    continue
                with self.subTest(function=claim.symbol):
                    actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
                    expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                 image.require(claim.va, claim.body_size)))
                    expected_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                      | ((word & 0x3FFFFFF) << 2)
                                      for i, word in enumerate(expected) if word >> 26 == 3]
                    self.assertEqual(calls, expected_calls)
                    self.assertEqual(actual, expected)
                    if claim.symbol != 'render_enqueue_sprite':
                        # The projected base is derived from the selected-asset
                        # field address; only that member is a relocation target.
                        self.assertIn(ORIGIN + FIELDS['tmd_state'] + 0x20, addresses)
                    shifted = dict(data, graphics_owner_probe=ORIGIN + 4)
                    wrong, same_calls, _ = linked_words(obj, unit, claim, shifted, functions)
                    self.assertNotEqual(wrong, actual)
                    self.assertEqual(same_calls, calls)

    def test_production_owner_preserves_complete_campaign_functions(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        catalog = load_catalog(RETAIL_CONFIG)
        data = {d.symbol: d.va for d in catalog.data['GAME.EXE']}
        functions = {f.symbol: f.va for f in catalog.functions['GAME.EXE']}
        units = (
            'game.asset_registry',
            'game.display_play_transition',
            'game.entity_model_render',
            'game.entity_render',
            'game.game',
            'game.geometry_render',
            'game.item',
            'game.lighting_presets',
            'game.map_event_render',
            'game.map_load',
            'game.map_scripts',
            'game.matrix',
            'game.matrix_lerp',
            'game.menu_map_viewer',
            'game.menu_runtime',
            'game.player_core',
            'game.player_death',
            'game.player_death_fade',
            'game.pool',
            'game.render',
            'game.render_enqueue',
            'game.render_frame',
            'game.render_map_cells',
            'game.render_scene',
            'game.render_sprite',
            'game.save_system',
            'game.sprite_add_ft4',
        )
        # These open functions are checked for calls and target integrity,
        # without accepting partial words as exact or requiring a future residue.
        partial = {
            'item_load_database',
            'map_interaction_dispatch',
            'map_show_screen_image',
            'menu_status_panel',
            'menu_draw_item_detail',
            'menu_draw_item_name_frame',
            'menu_draw_status_details',
            'menu_draw_window',
            'menu_draw_window_backdrop',
            'menu_item_model_preview',
            'player_move_horizontal',
            'render_entities',
            'render_map_cell',
            'talk_show_dialogue_page',
        }
        checked, exact = 0, 0
        for name in units:
            unit = manifest.by_name()[name]
            with self.subTest(unit=name), tempfile.TemporaryDirectory() as directory:
                source = self.compile(Path(directory), unit, unit.source_path.read_text())
                target = _load_object(BUILD / 'delink/game/modules' / unit.object_name)
                for obj in (source, target):
                    addresses = dict(data)
                    for claim in unit.data:
                        symbol = obj.named_symbol(claim.symbol)
                        if symbol.section is None:
                            metadata = json.loads(obj.path.with_suffix('.o.json').read_text())
                            self.assertEqual(metadata['data_symbol_sizes']['sizes'][claim.symbol], claim.size)
                        else:
                            self.assertEqual(symbol.size, claim.size)
                    # This checks complete function instructions at named
                    # referents. Full section placement is checked separately
                    # by data_match and is not established by a function test.
                    for claim in unit.functions:
                        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                     image.require(claim.va, claim.body_size)))
                        actual, calls, refs = linked_words(obj, unit, claim, addresses, functions)
                        expected_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                          | ((word & 0x3FFFFFF) << 2)
                                          for i, word in enumerate(expected) if word >> 26 == 3]
                        self.assertEqual(calls, expected_calls, claim.symbol)
                        if obj is target or claim.symbol not in partial:
                            self.assertEqual(actual, expected, claim.symbol)
                        if obj is source:
                            checked += 1
                            if claim.symbol not in partial:
                                exact += 1
                                if any(ORIGIN <= a < ORIGIN + EXTENT for a in refs):
                                    wrong, same_calls, _ = linked_words(
                                        obj, unit, claim,
                                        {**addresses, 'game_graphics_runtime': ORIGIN + 4}, functions)
                                    self.assertNotEqual(wrong, expected, claim.symbol)
                                    self.assertEqual(same_calls, calls)
        self.assertEqual((checked, exact), (180, 166))


if __name__ == '__main__':
    unittest.main()
