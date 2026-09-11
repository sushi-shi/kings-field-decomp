"""Source-object sizes must come from C, not the retail DATA expectations."""

from __future__ import annotations

import json
import re
import shutil
import tempfile
import unittest
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.compile import compile_source
from scripts.kf.data_match import Elf, diff_unit
from scripts.kf.delink import Datum, Function
from scripts.kf.manifest import Unit
from scripts.kf.mips_elf import DefinedSymbol, STT_OBJECT, write_mips_elf
from scripts.kf.retail import write_tsv


@unittest.skipUnless(all(shutil.which(tool) for tool in (
    'cpppsx-257', 'cc1psx-257', 'cpppsx-260', 'cc1psx-260',
    'dosbox-x', 'mipsel-linux-gnu-as',
)), 'pinned native compiler environment required')
class CompileDataSizeTests(unittest.TestCase):
    def compile(self, root, content, version, defines=()):
        source, output = root / 'source.c', root / 'source.o'
        source.write_text(content)
        write_tsv(root / 'delink/game/objects.tsv', ('object', 'scope'),
                  ({'object': 'objects/source.o', 'scope': 'decomp'},), ())
        compile_source(source, 'GAME.EXE', output, root / 'delink',
                       optimization='O2', compiler=f'gcc{version}-native', defines=defines)
        return output

    def symbols(self, path):
        with path.open('rb') as stream:
            return {symbol.name: (symbol['st_size'], symbol['st_info']['bind'])
                    for symbol in ELFFile(stream).get_section_by_name('.symtab').iter_symbols()}

    def alignments(self, path):
        with path.open('rb') as stream:
            return {section.name: section['sh_addralign']
                    for section in ELFFile(stream).iter_sections() if section['sh_flags'] & 2}

    def test_wrong_c_width_cannot_match_through_copied_claim_and_common_rounding(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'witness.c'
            source.write_text('''#define DATA(va, size)
DATA(0x80060000, 1)
int narrow;
DATA(0x80060008, 8)
unsigned char tail[8];
int witness(void) { return 0; }
''')
            function = Function('GAME.EXE', 0x80010000, 8, 8, 2,
                                'witness', 'decomp', 'test')
            unit = Unit('game.witness', 'GAME.EXE', str(source), 'probe', (function,), (
                Datum(0x80060000, 1, 'narrow', 'bss', 'global'),
                Datum(0x80060008, 8, 'tail', 'bss', 'global'),
            ))
            target = root / 'delink/game/modules' / unit.object_name
            base = root / 'objdiff/game/base' / unit.object_name
            target.parent.mkdir(parents=True)
            target.write_bytes(write_mips_elf(
                bytes.fromhex('0800e00300000000'), 'witness', 8,
                bss_size=16, bss_alignment=16, bss_symbols=(
                    DefinedSymbol('narrow', 0, 1, STT_OBJECT),
                    DefinedSymbol('tail', 8, 8, STT_OBJECT),
                )))
            write_tsv(root / 'delink/game/objects.tsv', ('object', 'scope'),
                      ({'object': f'modules/{unit.object_name}', 'scope': 'module'},), ())
            compile_source(source, 'GAME.EXE', base, root / 'delink',
                           optimization='O2', compiler='gcc257-native')
            # Both source declarations reserve eight bytes in GCC 2.5.7.
            # Equal total storage and alignment must not hide int versus byte.
            self.assertEqual(Elf(target).sections['.bss'].size, 16)
            self.assertNotIn('.bss', Elf(base).sections)
            sizes = json.loads(base.with_suffix('.o.json').read_text())['data_symbol_sizes']['sizes']
            self.assertEqual(sizes, {'narrow': 4, 'tail': 8})
            result = diff_unit(unit, root / 'delink', root / 'objdiff')
            self.assertFalse(result.matches, result)
            with base.open('rb') as stream:
                symbol = ELFFile(stream).get_section_by_name('.symtab').get_symbol_by_name('narrow')[0]
                self.assertEqual(symbol['st_shndx'], 'SHN_COMMON')
                self.assertEqual(symbol['st_size'], 8)

    def test_both_compilers_measure_types_without_claim_sizes_or_host_abi(self):
        content = '''#define DATA(va, size)
struct Entry { char label[3]; short field; int count; };
DATA(0x80050000, 99)
static struct Entry rows[2] = {{{1, 2, 3}, 4, 5}, {{6, 7, 8}, 9, 10}};
DATA(0x80050100, 99)
const unsigned char text[] = "ok";
DATA(0x80060000, 99)
int (*callback)(void);
DATA(0x80060010, 99)
double bounds[2];
DATA(0x80060020, 99)
signed char narrow[3];
int witness(void) { return rows[1].count + text[0]; }
'''
        expected = {'rows': 24, 'text': 3, 'callback': 4, 'bounds': 16, 'narrow': 3}
        for version in ('257', '260'):
            with self.subTest(version=version), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                path = self.compile(root, content, version)
                symbols = self.symbols(path)
                reservations = {**expected, **({'callback': 8, 'narrow': 8} if version == '257' else {})}
                self.assertEqual({name: symbols[name][0] for name in expected}, reservations)
                self.assertEqual(symbols['rows'][1], 'STB_LOCAL')
                for name in ('text', 'callback', 'bounds', 'narrow'):
                    self.assertEqual(symbols[name][1], 'STB_GLOBAL')
                self.assertFalse(any(name.startswith('__kf_source_data_sizes') for name in symbols))
                metadata = json.loads(path.with_suffix('.o.json').read_text())
                self.assertEqual(metadata['data_symbol_sizes'], {
                    'method': 'pinned-compiler-sizeof-probe', 'sizes': expected,
                })
                self.assertEqual({row['size'] for row in metadata['data_claims']}, {99})

                # Changing only expectations must not change the source object.
                previous = path.read_bytes()
                path = self.compile(root, content.replace(', 99)', ', 137)'), version)
                self.assertEqual(path.read_bytes(), previous)

                # Removing annotations proves the auxiliary table has not
                # leaked into any runtime section, alignment or relocation.
                claimed = Elf(path)
                alignments = self.alignments(path)
                unclaimed = Elf(self.compile(root, re.sub(r'^DATA\(.*\)\n', '', content,
                                                         flags=re.M), version))
                self.assertEqual(self.alignments(path), alignments)
                for name, section in claimed.sections.items():
                    if section.flags & 2:
                        self.assertEqual(unclaimed.sections[name], section)
                        self.assertEqual(unclaimed.relocations(name), claimed.relocations(name))

    def test_query_respects_preprocessing_and_avoids_source_name_collision(self):
        content = '''#define DATA(va, size)
#ifdef WIDE
typedef long Value;
#else
typedef char Value;
#endif
DATA(0x80050000, 19)
const Value __kf_source_data_sizes[3] = {1, 2, 3};
int witness(void) { return __kf_source_data_sizes[0]; }
'''
        for version in ('257', '260'):
            for defines, size in (((), 3), (('WIDE',), 12)):
                with self.subTest(version=version, defines=defines):
                    with tempfile.TemporaryDirectory() as directory:
                        path = self.compile(Path(directory), content, version, defines)
                        symbols = self.symbols(path)
                        self.assertEqual(symbols['__kf_source_data_sizes'], (size, 'STB_GLOBAL'))
                        self.assertNotIn('__kf_source_data_sizes_', symbols)

    def test_unclaimed_sources_do_not_run_a_size_query(self):
        for version in ('257', '260'):
            with self.subTest(version=version), tempfile.TemporaryDirectory() as directory:
                path = self.compile(Path(directory), 'int witness(void) { return 7; }\n', version)
                self.assertFalse(any(name.startswith('__kf_source_data_sizes')
                                     for name in self.symbols(path)))
                metadata = json.loads(path.with_suffix('.o.json').read_text())
                self.assertEqual(metadata['data_symbol_sizes']['sizes'], {})
