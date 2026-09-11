"""Complete initialized contributions and private workspace allocation controls."""

import os
import json
from pathlib import Path
import shutil
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.compile import compile_source
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.paths import REPO
from scripts.kf.sema.image import RetailImage


# Whole independently placed initialized contributions, including SDK padding.
INITIALIZED = {
    'game.game': {'.sdata': (0x80057B0C, 8)},
    'game.cd_file': {'.sdata': (0x80057B3C, 11)},
    'game.render': {'.data': (0x80055DAC, 240), '.sdata': (0x80057B50, 7)},
    'game.geometry_render': {'.data': (0x80055C5C, 336)},
    'game.render_frame': {'.data': (0x80055F68, 192)},
    'game.audio': {'.sdata': (0x80057B84, 4)},
    'open.resources': {'.sdata': (0x800372DC, 20)},
    'open.render_init': {'.data': (0x80035944, 192)},
    'open.entity_render': {'.data': (0x800358E0, 84)},
    'open.audio': {},
}


class RemainingOwnerStorageTests(unittest.TestCase):
    def test_complete_bytes_c_widths_and_private_linkage(self):
        if not shutil.which('cc1psx-257') or 'PSYQ_INCLUDE' not in os.environ:
            self.skipTest('pinned compiler and SDK headers required')
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest('configured retail images required')
        manifest = load()
        images = {key: RetailImage.load(key) for key in ('GAME.EXE', 'OPEN.EXE')}
        for name, contributions in INITIALIZED.items():
            unit = manifest.by_name()[name]
            profile = manifest.profiles[unit.profile]
            with self.subTest(unit=name), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                index = root / f'delink/{unit.image_key}/objects.tsv'
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
                    for section_name in ('.data', '.sdata'):
                        section = elf.get_section_by_name(section_name)
                        if section_name not in contributions:
                            self.assertTrue(section is None or section['sh_size'] == 0)
                            continue
                        va, size = contributions[section_name]
                        self.assertEqual(section['sh_type'], 'SHT_PROGBITS')
                        self.assertEqual(section.data(), images[unit.image].require(va, size))
                    symbols = elf.get_section_by_name('.symtab')
                    for datum in unit.data:
                        symbol = symbols.get_symbol_by_name(datum.symbol)[0]
                        if symbol['st_shndx'] == 'SHN_COMMON':
                            metadata = json.loads(output.with_suffix('.o.json').read_text())
                            self.assertEqual(metadata['data_symbol_sizes']['sizes'][datum.symbol], datum.size)
                        else:
                            self.assertEqual(symbol['st_size'], datum.size, datum.symbol)
                        if datum.scope == 'static':
                            self.assertEqual(symbol['st_info']['bind'], 'STB_LOCAL', datum.symbol)
                    if name == 'open.resources':
                        sbss = elf.get_section_by_name('.sbss')
                        self.assertEqual((sbss['sh_type'], sbss['sh_size']), ('SHT_NOBITS', 24))
                        for symbol_name, offset in (('cd_read_location', 0),
                                                    ('opening_scene1_arena_cursor', 8),
                                                    ('opening_ending_arena_cursor', 16)):
                            symbol = symbols.get_symbol_by_name(symbol_name)[0]
                            self.assertEqual((symbol['st_value'], symbol['st_size']), (offset, 4))
                            self.assertEqual(elf.get_section(symbol['st_shndx']).name, '.sbss')
                    if name.endswith('.audio'):
                        workspace = symbols.get_symbol_by_name('audio_sequence_table')[0]
                        runtime = symbols.get_symbol_by_name('audio_state')[0]
                        self.assertEqual(workspace['st_info']['bind'], 'STB_LOCAL')
                        self.assertEqual(runtime['st_info']['bind'], 'STB_GLOBAL')
                        self.assertEqual((workspace['st_size'], runtime['st_size']), (344, 144))
                        self.assertEqual(elf.get_section(workspace['st_shndx'])['sh_type'], 'SHT_NOBITS')
