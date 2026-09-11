"""Native COMMON requests cannot masquerade as fixed analysis BSS."""

import io
from pathlib import Path
import shutil
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.allocation_notes import compiler_requests
from scripts.kf.lnk import read
from scripts.kf.compile import compile_source
from scripts.kf.data_match import diff_unit
from scripts.kf.delink import Datum, Function
from scripts.kf.manifest import Unit
from scripts.kf.retail import write_tsv


class DirectiveTests(unittest.TestCase):
    def test_requests_are_independent_of_claims_and_include_zero_sizes(self):
        self.assertEqual(compiler_requests(' .comm unclaimed,8\n .lcomm local,16\n .comm zero,0\n'),
                         [{'name': 'unclaimed', 'reservation_size': 8},
                          {'name': 'zero', 'reservation_size': 0}])
        with self.assertRaises(ValueError):
            compiler_requests('.comm unknown,8,unproved_alignment')


@unittest.skipUnless(all(shutil.which(name) for name in (
    'cpppsx-257', 'cc1psx-257', 'dosbox-x', 'mipsel-linux-gnu-as',
)), 'pinned compiler and assembler required')
class CompiledAllocationTests(unittest.TestCase):
    def test_native_common_requests_remain_unplaced_and_private_storage_stays_fixed(self):
        for exported in (False, True):
            with self.subTest(exported=exported), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = root / 'control.c'
                source.write_text('#define DATA(va, size)\nDATA(0x80060000, 8)\n'
                                  + ('' if exported else 'static ')
                                  + 'unsigned int storage[2];\n'
                                  'int witness(void) { return storage[0]; }\n')
                function = Function('OPEN.EXE', 0x80010000, 16, 16, 4, 'witness', 'test', 'test')
                unit = Unit('open.control', 'OPEN.EXE', str(source), 'probe', (function,),
                            (Datum(0x80060000, 8, 'storage', 'bss',
                                   'global' if exported else 'static'),))
                target = root / 'delink/open/modules' / unit.object_name
                base = root / 'objdiff/open/base' / unit.object_name
                target.parent.mkdir(parents=True)
                write_tsv(root / 'delink/open/objects.tsv', ('object', 'scope'),
                          ({'object': 'modules/' + unit.object_name, 'scope': 'module'},), ())
                compile_source(source, 'OPEN.EXE', base, root / 'delink',
                               optimization='O2', compiler='gcc257-native')
                native = read(base.with_suffix('.OBJ').read_bytes())
                elf = ELFFile(io.BytesIO(base.read_bytes()))
                symbol = elf.get_section_by_name('.symtab').get_symbol_by_name('storage')[0]
                self.assertEqual(symbol['st_size'], 8)
                self.assertIsNone(elf.get_section_by_name('.note.kf.allocations'))
                if exported:
                    request = next(s for s in native.symbols.values() if s.name == 'storage')
                    self.assertTrue(request.common)
                    self.assertEqual(request.value, 8)
                    self.assertEqual(symbol['st_shndx'], 'SHN_COMMON')
                    self.assertIsNone(elf.get_section_by_name('.bss'))
                else:
                    self.assertEqual(elf.get_section(symbol['st_shndx']).name, '.bss')
                    self.assertEqual(symbol['st_info']['bind'], 'STB_LOCAL')
                    self.assertEqual(len(next(s for s in native.sections.values()
                                              if s.name == '.bss').data), 8)
                target.write_bytes(base.read_bytes())
                result = diff_unit(unit, root / 'delink', root / 'objdiff')
                self.assertEqual(result.matches, not exported, result)
                if exported:
                    self.assertTrue(any('unsupported-common-allocation' in row.detail
                                        for row in result.divergent), result)


if __name__ == '__main__':
    unittest.main()
