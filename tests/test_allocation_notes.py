"""Native COMMON requests cannot masquerade as fixed analysis BSS."""

import io
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

from scripts.kf.allocation_notes import compiler_requests, read_requests
from scripts.kf.compile import compile_source
from scripts.kf.data_match import Elf, diff_unit
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
    'cpppsx-257', 'cc1psx-257', 'maspsx', 'mipsel-linux-gnu-as',
)), 'pinned compiler and assembler required')
class CompiledAllocationTests(unittest.TestCase):
    def test_identical_flattened_objects_do_not_prove_exported_allocation(self):
        for exported in (False, True):
            with self.subTest(exported=exported), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = root / 'control.c'
                source.write_text('#define DATA(va, size)\nDATA(0x80060000, 8)\n'
                                  + ('' if exported else 'static ')
                                  + 'unsigned int storage[2];\n'
                                  'int witness(void) { return storage[0]; }\n')
                function = Function('OPEN.EXE', 0x80010000, 8, 8, 1, 'witness', 'test', 'test')
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
                plain = root / 'plain' / unit.object_name
                with patch('scripts.kf.compile.assembly_note', return_value=''):
                    compile_source(source, 'OPEN.EXE', plain, root / 'delink',
                                   optimization='O2', compiler='gcc257-native')
                before, after = Elf(plain), Elf(base)
                allocated = {name for name, section in before.sections.items() if section.flags & 2}
                self.assertEqual(allocated, {name for name, section in after.sections.items()
                                             if section.flags & 2})
                for name in allocated:
                    self.assertEqual(before.sections[name], after.sections[name])
                    self.assertEqual(before.allocations.get(name), after.allocations.get(name))
                    self.assertEqual(before.relocations(name), after.relocations(name))
                target.write_bytes(base.read_bytes())
                elf = ELFFile(io.BytesIO(base.read_bytes()))
                requests = read_requests(elf)
                self.assertEqual(requests, [{'name': 'storage', 'reservation_size': 8}]
                                 if exported else [])
                self.assertEqual(elf.get_section_by_name('.bss')['sh_size'], 8)
                result = diff_unit(unit, root / 'delink', root / 'objdiff')
                self.assertEqual(result.matches, not exported, result)
                if exported:
                    self.assertIn('unplaced-exported-allocation', result.divergent[0].detail)
                    damaged = base.read_bytes().replace(b'"version":1', b'"version":9')
                    self.assertNotEqual(damaged, base.read_bytes())
                    with self.assertRaises(ValueError):
                        read_requests(ELFFile(io.BytesIO(damaged)))
                    target.write_bytes(plain.read_bytes())
                    base.write_bytes(damaged)
                    result = diff_unit(unit, root / 'delink', root / 'objdiff')
                    self.assertFalse(result.matches)
                    self.assertTrue(any('invalid-allocation-provenance' in row.detail
                                        for row in result.divergent))
                    # The old representation alone still appears exact: the
                    # regression is specifically loss of reservation class.
                    target.write_bytes(plain.read_bytes())
                    base.write_bytes(plain.read_bytes())
                    self.assertTrue(diff_unit(unit, root / 'delink', root / 'objdiff').matches)


if __name__ == '__main__':
    unittest.main()
