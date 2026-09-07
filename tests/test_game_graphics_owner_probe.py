"""GAME graphics clear bounds and a shared-owner pilot; no production claim."""

from __future__ import annotations

import os
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
    'unknown_projection_morph_20318': 0x20318,
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
                    rewrite, unit.source_path.read_text(), flags=re.M | re.S)
    if 'game_main_loop' in selected:
        old = ('memset(&graphics_owner_probe.display_state.buffer_index, 0, '
               'INITIAL_GRAPHICS_CLEAR_BYTES);')
        assert source.count(old) == 1
        source = source.replace(old, 'memset(&graphics_owner_probe, 0, sizeof graphics_owner_probe);')
    return '#include "game_graphics_owner_probe.h"\n' + source


class GameGraphicsOwnerProbeTests(unittest.TestCase):
    def tools(self):
        if not all(shutil.which(name) for name in ('cpppsx-257', 'cc1psx-257', 'maspsx')):
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
        compile_source(path, unit.image, output, BUILD / 'delink', profile.optimization,
                       profile.small_data, profile.aspsx_version,
                       (REPO / 'include', root, HEADER.parent, Path(os.environ['PSYQ_INCLUDE'])),
                       profile.cc1_flags, profile.compiler, profile.maspsx_flags, defines=unit.defines)
        return _load_object(output)

    def test_clear_extent_and_existing_fields_are_independent_of_candidate(self):
        image = self.retail()
        self.assertEqual(image.require(0x800146CC, 24), struct.pack(
            '<6I', 0x3C048007, 0x24840E98, 0x00002821, 0x3C060002, 0x0C0140C3, 0x34C649CC))
        self.assertEqual(ORIGIN + EXTENT, 0x80095864)
        identities = load_data_identities(RETAIL_CONFIG)
        by_name = {item.name: item for (image, _), item in identities.items() if image == 'GAME.EXE'}
        for field, offset in FIELDS.items():
            if not field.startswith('unknown_'):
                self.assertEqual(by_name[field].va, ORIGIN + offset, field)
        self.assertEqual(by_name['audio_state'].va, ORIGIN + EXTENT + 4)
        # This pilot must not relabel array prefixes as proved capacities.
        self.assertEqual(by_name['tmd_projected_vertices'].size, 8)
        self.assertEqual(by_name['tmd_morph_scratch'].size, 24)

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
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
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
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        correct = '''    if (rotation != 0) {
        render_state.view_rotation = *rotation;
    }
    RotMatrix(&render_state.view_rotation, &render_state.view_matrix);
    angles.vz = 0;
    angles.vy = 0;'''
        old_bug = '''    if (rotation != 0) {
        render_state.view_rotation = *rotation;
        RotMatrix(&render_state.view_rotation, &render_state.view_matrix);
        angles.vz = 0;
        angles.vy = 0;
    }'''
        canonical = unit.source_path.read_text()
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

    def test_extended_display_tmd_pilot_preserves_exacts_but_does_not_close_owner(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
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
        }
        # Size and first raw divergence are observed symptoms, not attributed
        # compiler mechanisms. Neither the pilot nor a partial suffix is banked.
        partial = {
            'display_initialize': (336, 0xA4, 0x3C048009, 0x3C108009, [
                0x80090EC0, 0x80090F78, 0x80090F1C, 0x80090F8C, 0x80090F32, 0x80090ED6,
                0x80090ED8, 0x80090F34, 0x80090ED9, 0x80090EDA, 0x80090EDB, 0x80090F35,
                0x80090F36, 0x80090F37, 0x80095740,
            ]),
            'tmd_prepare_primitive_indices': (768, 8, 0, 0x27BDFFF8, [
                0x80090FC8, 0x80090FC8, 0x80090FC8, 0x800121B4,
            ]),
        }
        for name, names in selected.items():
            unit = manifest.by_name()[name]
            with self.subTest(unit=name), tempfile.TemporaryDirectory() as directory:
                obj = self.compile(Path(directory), unit, candidate_source(unit, names))
                for claim in unit.functions:
                    if claim.symbol not in names:
                        continue
                    with self.subTest(function=claim.symbol):
                        actual, calls, targets = linked_words(obj, unit, claim, data, functions)
                        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                                     image.require(claim.va, claim.body_size)))
                        expected_calls = [((claim.va + i * 4 + 4) & 0xF0000000)
                                          | ((word & 0x3FFFFFF) << 2)
                                          for i, word in enumerate(expected) if word >> 26 == 3]
                        self.assertEqual(calls, expected_calls)
                        if claim.symbol in partial:
                            size, first, left, right, addresses = partial[claim.symbol]
                            self.assertNotEqual(actual, expected)
                            self.assertEqual(len(actual) * 4, size)
                            self.assertEqual(actual[:first // 4], expected[:first // 4])
                            self.assertEqual((actual[first // 4], expected[first // 4]), (left, right))
                            self.assertEqual(targets, addresses)
                            if claim.symbol == 'tmd_prepare_primitive_indices':
                                self.assertEqual(
                                    [(i * 4, a, b) for i, (a, b) in
                                     enumerate(zip(actual, expected)) if a != b],
                                    [(8, 0, 0x27BDFFF8), (0x2FC, 0, 0x27BD0008)])
                        else:
                            self.assertEqual(actual, expected)
                            if any(ORIGIN <= target < ORIGIN + EXTENT for target in targets):
                                wrong, same_calls, _ = linked_words(
                                    obj, unit, claim, {**data, 'graphics_owner_probe': ORIGIN + 4}, functions)
                                self.assertNotEqual(wrong, expected)
                                self.assertEqual(same_calls, calls)

    def test_floor_item_counter_requires_direct_member_access(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.item']
        claim = next(c for c in unit.functions if c.symbol == 'item_load_floor_placements')
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        for owner in (False, True):
            prefix = 'graphics_owner_probe.' if owner else ''
            source = (candidate_source(unit, {claim.symbol}) if owner
                      else unit.source_path.read_text())
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

    def test_narrow_material_owner_does_not_preserve_exact_frame(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.render_frame']
        claim = unit.functions[0]
        expected = list(struct.unpack(f'<{claim.body_size // 4}I',
                                     image.require(claim.va, claim.body_size)))
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        fields = {
            'active_render_clut': 'clut', 'active_render_tpage': 'tpage',
            'active_render_red': 'color.r', 'active_render_green': 'color.g',
            'active_render_blue': 'color.b', 'active_render_code': 'color.cd',
        }
        canonical = unit.source_path.read_text()
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

    def test_scratch_consumers_preserve_typed_accesses_without_capacity_claims(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
        data['graphics_owner_probe'] = ORIGIN
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        selected = {
            'game.render': {'tmd_project_vertices', 'tmd_project_vertices_shift',
                            'tmd_transform_vertices'},
            'game.pool': {'render_bind_animated_instance'},
        }
        for unit_name, names in selected.items():
            unit = manifest.by_name()[unit_name]
            for owner in (False, True):
                source = unit.source_path.read_text()
                if owner:
                    source = candidate_source(unit, names).replace(
                        'tmd_projected_vertices',
                        '(KfScreenVertex *)graphics_owner_probe.unknown_projection_morph_20318')
                    source = source.replace(
                        'tmd_morph_scratch',
                        '((SVECTOR *)(graphics_owner_probe.unknown_projection_morph_20318 + 0x1f40))')
                    source = source.replace(
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

    def test_polygon_owner_recovers_addresses_without_claiming_array_capacity(self):
        self.tools()
        image, manifest = self.retail(), load_manifest()
        unit = manifest.by_name()['game.render_enqueue']
        names = {'render_enqueue_tmd', 'render_enqueue_model', 'render_enqueue_map',
                 'render_enqueue_sprite'}
        source = candidate_source(unit, names)
        self.assertEqual(source.count('= tmd_projected_vertices;'), 3)
        source = source.replace(
            '= tmd_projected_vertices;',
            '= (KfScreenVertex *)graphics_owner_probe.unknown_projection_morph_20318;')
        data = {item.name: item.va for (key, _), item in load_data_identities(RETAIL_CONFIG).items()
                if key == 'GAME.EXE'}
        data['graphics_owner_probe'] = ORIGIN
        self.assertEqual(ORIGIN + FIELDS['unknown_projection_morph_20318'], 0x800911B0)
        functions = {item.symbol: item.va for item in load_catalog(RETAIL_CONFIG).functions['GAME.EXE']}
        # Observed complete-body residues. These are not permitted masks for banking.
        spills = {
            'render_enqueue_model': {
                0x44: (0xAFA80020, 0xAFA80028), 0x58: (0x8FA30020, 0x8FA30028),
                0x64: (0xAFA80020, 0xAFA80028), 0xA0: (0xAFA80028, 0xAFA80020),
                0x68C: (0x8FA80028, 0x8FA80020), 0x6A0: (0x8FA20020, 0x8FA20028),
                0x6B4: (0xAFA80020, 0xAFA80028),
            },
            'render_enqueue_map': {
                0x58: (0xAFA20018, 0xAFA20020), 0x9C: (0xAFA70020, 0xAFA70018),
                0x1A8: (0x8FA70018, 0x8FA70020), 0x318: (0x8FA70018, 0x8FA70020),
                0x3C8: (0x8FA70020, 0x8FA70018),
            },
        }
        pairs = {'render_enqueue_tmd': 36, 'render_enqueue_model': 10,
                 'render_enqueue_map': 6, 'render_enqueue_sprite': 7}
        first_pass = {}
        for loop_local in (False, True):
            candidate = source
            if loop_local:
                self.assertEqual(candidate.count('    u32 header;\n'), 3)
                candidate = candidate.replace('    u32 header;\n', '').replace(
                    '        header = *(u32 *)packet;', '        u32 header = *(u32 *)packet;')
            with self.subTest(loop_local=loop_local), tempfile.TemporaryDirectory() as directory:
                obj = self.compile(Path(directory), unit, candidate)
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
                        self.assertEqual(len(addresses), pairs[claim.symbol])
                        self.assertEqual(actual[-12:], expected[-12:])
                        if claim.symbol == 'render_enqueue_sprite':
                            self.assertEqual(actual, expected)
                        elif claim.symbol in spills:
                            self.assertNotEqual(actual, expected)
                            self.assertEqual(len(actual), len(expected))
                            differences = {i * 4: (a, b) for i, (a, b) in enumerate(zip(actual, expected))
                                           if a != b}
                            self.assertEqual(differences, spills[claim.symbol])
                        else:
                            self.assertEqual(len(actual) * 4, 3900)
                            self.assertEqual(actual[:17], expected[:17])
                            self.assertEqual((actual[17], expected[17]), (0xAFA80020, 0xAFA80028))
                            self.assertEqual(addresses[-1], 0x80090EBC)
                        shifted = dict(data, graphics_owner_probe=ORIGIN + 4)
                        wrong, same_calls, _ = linked_words(obj, unit, claim, shifted, functions)
                        self.assertNotEqual(wrong, actual)
                        self.assertEqual(same_calls, calls)
                        if loop_local:
                            self.assertEqual(actual, first_pass[claim.symbol])
                        else:
                            first_pass[claim.symbol] = actual


if __name__ == '__main__':
    unittest.main()
