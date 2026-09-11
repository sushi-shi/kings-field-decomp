"""Native object records and lossless diagnostic relocation conversion."""

import io
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.lnk import Reader, affine, data_referent_spellings, elf_view, read
from scripts.kf.sdk import assemble_many, dos_run, tool_succeeded
from tests.psylink_order_smoke import symbol_address


class RecordTests(unittest.TestCase):
    def test_truncated_and_unknown_records_are_rejected(self):
        for blob in (b'', b'LNK\x01\0', b'LNK\x02', b'LNK\x02\xff', b'LNK\x02\0\0'):
            with self.subTest(blob=blob), self.assertRaises(ValueError):
                read(blob)

    def test_subtraction_uses_native_right_first_encoding(self):
        expression = Reader(b'\x2e\x00\x04\x80\x00\x00\x02\x01\x00').expression()
        self.assertEqual(affine(expression), (('symbol', 1), -0x8004))

    def test_two_referents_cannot_be_silently_flattened(self):
        with self.assertRaises(ValueError):
            affine(('add', ('symbol', 1), ('section', 2)))

    def test_compiler_spellings_come_from_instruction_operands_only(self):
        assembly = (
            '\t.file 1 "unit.c"\n\t.data\n\t.globl floor_entry_cells\nfloor_entry_cells:\n'
            '\t.half 1,2\n\t.text\n$L3:\n\tla $3,floor_entry_cells-2\n'
            '\tlw $2,pointer_table+4($4)\n\tbeq $2,$0,$L3-8\n'
            '\taddiu $sp,$sp,-24 # pad+8\n\t.def x; .val 4; .endef\n'
            '\t.stabs "line-1",1,0,0,LM7-4\n')
        self.assertEqual(data_referent_spellings(assembly),
                         frozenset({('floor_entry_cells', -2), ('pointer_table', 4)}))


@unittest.skipUnless(shutil.which('dosbox-x') and os.environ.get('PSYQ_ASPSX')
                     and os.environ.get('PSYQ_BIN'), 'native SDK tools required')
class NativeRelocationTests(unittest.TestCase):
    def test_biased_reference_to_own_datum_keeps_the_compiler_referent(self):
        # ASPSX records `table-2` as `.data+14`; the view restores the symbol
        # the compiler named, with its negative addend, leaving bytes intact.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            assembly = (
                '\t.file 1 "control.s"\n\t.text\n\t.globl witness\n\t.ent witness\nwitness:\n'
                '\t.set noreorder\n\tla $2,table-2\n\tjr $31\n\tnop\n\t.end witness\n'
                '\t.data\n\t.globl pad\npad:\n\t.word 0,0,0,0\n'
                '\t.globl table\ntable:\n\t.half 1,2,3,4,5\n')
            (root / 'UNIT.S').write_text(assembly)
            native = assemble_many(root, ('UNIT',), 0)['UNIT']
            for spellings, expected in ((frozenset(), '.data'),
                                        (data_referent_spellings(assembly), 'table')):
                with self.subTest(expected=expected):
                    view = ELFFile(io.BytesIO(elf_view(native, spellings=spellings)))
                    symtab = view.get_section_by_name('.symtab')
                    names = []
                    for relocation in view.get_section_by_name('.rel.text').iter_relocations():
                        symbol = symtab.get_symbol(relocation['r_info_sym'])
                        # Section symbols are unnamed; report their section instead.
                        names.append(symbol.name or view.get_section(symbol['st_shndx']).name)
                    self.assertEqual(names, [expected, expected])
                    high, low = struct.unpack_from('<2I', view.get_section_by_name('.text').data())
                    self.assertEqual((high & 0xffff, low & 0xffff),
                                     (0x0000, 0x000e) if expected == '.data' else (0x0000, 0xfffe))
                    self.assertEqual(view.get_section_by_name('.data').data()[16:20],
                                     struct.pack('<2H', 1, 2))

    def test_negative_addends_match_psylink_and_native_bytes_stay_unchanged(self):
        # Synthetic assembler control: branch/call, high/low carry, and pointer
        # expressions are checked against the real linker, with no retail input.
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'UNIT.S').write_text(
                '\t.file 1 "control.s"\n\t.text\n\t.globl witness\n'
                '\t.loc 1 1\n\t.ent witness\nwitness:\n\t.set noreorder\n'
                '\tla $2,external-32772\n'
                '\tjal callee\n\tnop\n\t.loc 1 328\n\tj witness\n\tnop\n\t.end witness\n'
                '\t.data\n\t.globl pointer\npointer:\n\t.word external-4\n')
            (root / 'TARGETS.S').write_text(
                '\t.text\n\t.globl callee\ncallee:\n\t.set noreorder\n'
                '\tjr $31\n\tnop\n\t.data\n\t.globl external\nexternal:\n\t.word 1\n')
            native = assemble_many(root, ('UNIT', 'TARGETS'), 0)['UNIT']
            obj = read(native)
            view = ELFFile(io.BytesIO(elf_view(native)))
            self.assertEqual((root / 'UNIT.OBJ').read_bytes(), native)
            pairs = [(section.name, offset, kind, affine(expr)) for section in obj.sections.values()
                     for offset, kind, expr in section.patches]
            self.assertEqual([kind for _, _, kind, _ in pairs], [82, 84, 74, 74, 16])
            self.assertEqual([value for _, _, kind, (_, value) in pairs if kind in (82, 84)],
                             [-32772, -32772])
            text = view.get_section_by_name('.text').data()
            high, low = struct.unpack_from('<2I', text)
            self.assertEqual((high & 0xffff, low & 0xffff), (0xffff, 0x7ffc))
            self.assertEqual(view.get_section_by_name('.data').data(), struct.pack('<I', 0xfffffffc))
            shutil.copyfile(Path(os.environ['PSYQ_BIN']) / 'PSYLINK.EXE', root / 'PSYLINK.EXE')
            (root / 'LINK.LNK').write_bytes(
                b'\torg $80010000\r\n\tinclude "UNIT.OBJ"\r\n'
                b'\tinclude "TARGETS.OBJ"\r\n')
            dos_run(root, ['psylink /c @LINK.LNK,OUT.CPE,OUT.SYM,OUT.MAP > LINK.TXT'], 'link')
            tool_succeeded(root, 'LINK.TXT', 'OUT.CPE', b'CPE\x01')
            from scripts.kf.executable import cpe_loads

            _, loads = cpe_loads((root / 'OUT.CPE').read_bytes())
            linked = {address + index: byte for address, payload in loads
                      for index, byte in enumerate(payload)}
            start = symbol_address(root / 'OUT.SYM', 'witness', case_sensitive=True)
            words = struct.unpack('<6I', bytes(linked[start + index] for index in range(24)))
            external = symbol_address(root / 'OUT.SYM', 'external', case_sensitive=True)
            callee = symbol_address(root / 'OUT.SYM', 'callee', case_sensitive=True)
            value = external - 32772
            self.assertEqual(words[:2], (0x3c020000 | (((value + 0x8000) >> 16) & 0xffff),
                                        0x24420000 | (value & 0xffff)))
            self.assertEqual(words[2], 0x0c000000 | ((callee >> 2) & 0x3ffffff))
            self.assertEqual(words[4], 0x08000000 | ((start >> 2) & 0x3ffffff))
            pointer = symbol_address(root / 'OUT.SYM', 'pointer', case_sensitive=True)
            self.assertEqual(bytes(linked[pointer + index] for index in range(4)),
                             struct.pack('<I', external - 4))
            self.assertEqual((root / 'UNIT.OBJ').read_bytes(), native)
