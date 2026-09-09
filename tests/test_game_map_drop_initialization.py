"""Retail control-flow witness for the dropped-object velocity reset."""

from __future__ import annotations

import os
import shutil
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.compile import compile_source
from scripts.kf.manifest import load
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO
from scripts.kf.sema.image import RetailImage


class GameMapDropInitializationTests(unittest.TestCase):
    def test_ids_outside_action_bands_still_clear_vertical_velocity(self):
        if not all(shutil.which(tool) for tool in ('cpppsx-257', 'cc1psx-257', 'maspsx')):
            self.skipTest('pinned compiler tools required')
        if not os.environ.get('PSYQ_INCLUDE'):
            self.skipTest('pinned SDK headers required')
        try:
            image = RetailImage.load('GAME.EXE')
        except (OSError, ValueError):
            self.skipTest('hash-checked local GAME.EXE required')

        # The >=65 branch reaches the zero halfword store, before the restores.
        self.assertEqual(image.require(0x80031994, 4), struct.pack('<I', 0x10400003))
        self.assertEqual(image.require(0x800319A4, 4), struct.pack('<I', 0xA6200024))
        manifest = load()
        unit = manifest.by_name()['game.map_object']
        profile = manifest.profiles[unit.profile]
        source = unit.source_path.read_text()
        start = source.index('ADDRESS(0x80031834,')
        end = source.index('ADDRESS(0x800319c8,', start)
        body = source[start:end]
        reset = '    object->link.fields.vertical_velocity = 0;\n'
        self.assertEqual(body.count(reset), 1)
        old_body = body.replace(reset, '')
        for action in ('FALL_AND_TIP', 'FALL_AND_SPIN', 'BOUNCE'):
            call = f'        map_object_start_action_if_idle(object, KF_MAP_OBJECT_ACTION_{action});\n'
            self.assertEqual(old_body.count(call), 1)
            old_body = old_body.replace(call, call + '    ' + reset)

        for old_scope in (False, True):
            with self.subTest(old_scope=old_scope), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                candidate = root / unit.source_path.name
                candidate.write_text(source[:start] + old_body + source[end:] if old_scope else source)
                output = root / unit.object_name
                compile_source(
                    candidate, unit.image, output, BUILD / 'delink', profile.optimization,
                    profile.small_data, profile.aspsx_version,
                    (REPO / 'include', Path(os.environ['PSYQ_INCLUDE'])),
                    profile.cc1_flags, profile.compiler, profile.maspsx_flags, defines=unit.defines,
                )
                obj = _load_object(output)
                function = obj.named_symbol('map_object_spawn_effect')
                words = struct.unpack_from(f'<{function.size // 4}I',
                                           obj.sections['.text'], function.value)
                # Its final conditional skips only the optional bounce call.
                branches = [i for i, word in enumerate(words) if word >> 26 in (4, 5, 6, 7)]
                branch = branches[-1]
                # The predicate register may differ in a non-exact source body.
                self.assertEqual(words[branch] & 0xFC1F0000, 0x10000000)  # beq rs,zero
                self.assertNotEqual((words[branch] >> 21) & 31, 0)
                destination = branch + 1 + (words[branch] & 0xFFFF)
                if old_scope:
                    self.assertEqual(words[destination] >> 26, 0x23)  # Old path skips to lw ra.
                else:
                    # Ignore only the still-unmatched base register for this CFG witness.
                    self.assertEqual(words[destination] & 0xFC1FFFFF, 0xA4000024)
                    self.assertEqual(words[destination + 1] >> 26, 0x23)
