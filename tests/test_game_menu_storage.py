"""Menu storage must retain native small sections and complete object widths."""

import os
import shutil
import tempfile
import unittest
from dataclasses import replace
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.compile import compile_source
from scripts.kf.manifest import load
from scripts.kf.paths import REPO
from scripts.kf.roundtrip import UnitResult, plan


class GameMenuStorageTests(unittest.TestCase):
    def test_native_sections_keep_cursor_private_and_initializer_separate(self):
        if not shutil.which('cc1psx-257') or 'PSYQ_INCLUDE' not in os.environ:
            self.skipTest('pinned compiler and SDK headers required')
        manifest = load()
        unit = manifest.by_name()['game.menu_runtime']
        profile = manifest.profiles[unit.profile]
        self.assertEqual(len(unit.functions), 35)
        self.assertEqual((unit.functions[0].va, unit.functions[-1].end),
                         (0x8002430C, 0x8002B078))
        for left, right in zip(unit.functions, unit.functions[1:]):
            self.assertEqual(left.end, right.va)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            index = root / 'delink/game/objects.tsv'
            index.parent.mkdir(parents=True)
            index.write_text(f'object\tscope\nmodules/{unit.object_name}\tmodule\n')
            output = root / unit.object_name
            compile_source(
                unit.source_path, unit.image, output, root / 'delink',
                profile.optimization, profile.small_data, profile.aspsx_version,
                (REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
                profile.cc1_flags, profile.compiler,
                defines=unit.defines,
            )
            with output.open('rb') as stream:
                elf = ELFFile(stream)
                sdata, sbss = (elf.get_section_by_name(name)
                               for name in ('.sdata', '.sbss'))
                self.assertEqual(sdata.data(), bytes(12))
                self.assertEqual((sbss['sh_type'], sbss['sh_size']), ('SHT_NOBITS', 8))
                symbols = elf.get_section_by_name('.symtab')
                cursor = symbols.get_symbol_by_name('current_poly_ft4')[0]
                self.assertEqual((cursor['st_value'], cursor['st_size'],
                                  cursor['st_info']['bind']), (0, 4, 'STB_LOCAL'))
                self.assertEqual(elf.get_section(cursor['st_shndx']).name, '.sbss')
                expected = {'.sdata': 0x80057B6C, '.sbss': 0x80057E88,
                            '.rodata': 0x80012350}
                # Function residues do not establish or invalidate data layout.
                data_unit = replace(unit, functions=())
                result = UnitResult(unit.image, unit.unit, str(output))
                self.assertEqual(plan(elf, data_unit, result), expected)
                self.assertEqual([i for i in result.issues if i.get('section') != '.text'], [])
                # Moving one initialized vector cannot be hidden by equal bytes.
                wrong = replace(data_unit, data=tuple(
                    replace(d, va=d.va + 4) if d.symbol == 'menu_item_preview_rotation' else d
                    for d in data_unit.data))
                result = UnitResult(unit.image, unit.unit, str(output))
                plan(elf, wrong, result)
                self.assertIn('conflicting-section-bases', {i['kind'] for i in result.issues})
