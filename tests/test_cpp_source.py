"""Controls for the standalone C++ compiler and native linker bridge."""

import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest

from scripts.kf.lnk import affine, read
from scripts.kf.mips_elf import DefinedSymbol, MipsRelocation, write_mips_elf
from scripts.psxbuild.clang import compile_program
from scripts.psxbuild.elf_to_lnk import convert


class NativeBridgeControls(unittest.TestCase):
    def test_negative_addend_carry_calls_and_large_code_records(self):
        # Cross a native 16-bit code-record boundary, then reference an external
        # symbol with an addend that needs carry-adjusted HI16/LO16 relocation.
        offset = 65532
        code = bytes(offset) + struct.pack('<4I', 0x3c02ffff, 0x24427ffc, 0x0c000000, 0)
        elf = write_mips_elf(code, 'witness', len(code), [
            MipsRelocation(offset, 'R_MIPS_HI16', 'external'),
            MipsRelocation(offset + 4, 'R_MIPS_LO16', 'external'),
            MipsRelocation(offset + 8, 'R_MIPS_26', 'callee'),
        ], data=struct.pack('<I', 0xfffffffc),
            data_symbols=[DefinedSymbol('pointer', 0, 4)],
            data_relocations=[MipsRelocation(0, 'R_MIPS_32', 'external')])
        obj = read(convert(elf))
        text = next(s for s in obj.sections.values() if s.name == '.text')
        self.assertEqual(text.data, code)
        self.assertEqual([p[:2] for p in text.patches],
                         [(offset, 82), (offset + 4, 84), (offset + 8, 74)])
        self.assertEqual(affine(text.patches[0][2])[1], -32772)
        self.assertEqual(affine(text.patches[1][2])[1] & 0xffff, 0x7ffc)
        data = next(s for s in obj.sections.values() if s.name == '.data')
        self.assertEqual(affine(data.patches[0][2])[1], -4)
        self.assertFalse(set(obj.sections) & set(obj.symbols))

    def test_mips_two_division_trap_is_rejected(self):
        elf = write_mips_elf(struct.pack('<I', 0x004001f4), 'trap', 4)
        with self.assertRaisesRegex(ValueError, 'MIPS-II trap'):
            convert(elf)

    @unittest.skipUnless(shutil.which('clang') and shutil.which('mipsel-linux-gnu-ld')
                         and os.environ.get('PSYQ_INCLUDE'), 'modern PS1 build tools required')
    def test_cpp_division_and_enum_storage_compile_without_traps(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            (repo / 'include').mkdir()
            (repo / 'vendor/include').mkdir(parents=True)
            (repo / 'control.cpp').write_text('''
enum class Count : unsigned short { one = 1 };
struct State { Count count; int total; };
extern State state;
extern "C" int quotient(int divisor) {
    return (state.total + static_cast<int>(state.count)) / divisor;
}
''')
            output = repo / 'build'
            output.mkdir()
            report = compile_program([{'source': 'control.cpp', 'options': {'defines': []}}],
                                     output, 0, repo=repo)
            obj = read((output / report['object']).read_bytes())
            self.assertTrue(any(s.name == 'quotient' for s in obj.symbols.values()))
            code = next(s.data for s in obj.sections.values() if s.name == '.text')
            words = struct.unpack('<' + 'I' * (len(code) // 4), code)
            self.assertTrue(any(word >> 26 == 0 and word & 63 == 26 for word in words))
            self.assertFalse(any(word >> 26 == 0 and word & 63 == 52 for word in words))
