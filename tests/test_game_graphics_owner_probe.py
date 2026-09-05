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
    'DAT_80095060': 0x241C8, 'DAT_80095062': 0x241CA,
    'DAT_80095064': 0x241CC, 'unknown_241cd': 0x241CD,
    'DAT_80095066': 0x241CE, 'DAT_80095068': 0x241D0,
    'DAT_8009506a': 0x241D2, 'DAT_8009506c': 0x241D4,
    'notification_message_ids': 0x241D6, 'notification_state': 0x241DE,
    'DAT_8009508c': 0x241F4, 'DAT_8009508e': 0x241F6,
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
        old = 'memset(&graphics_owner_probe.display_state.buffer_index, 0, 0x249cc);'
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
                        self.assertEqual(len(actual) * 4, 576)  # Retail is 592; not an exact claim.
                        self.assertEqual(actual[0], 0x27BDFFA8)  # 88-byte frame, retail 120.
                        # The complete matrix-base/call sequence is exact; the
                        # epilogue is not, so do not claim an exact suffix/body.
                        self.assertEqual(actual[-24:-6], expected[-24:-6])
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


if __name__ == '__main__':
    unittest.main()
