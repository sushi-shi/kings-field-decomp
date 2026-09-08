"""Compiler alignment requirements must survive the GNU ELF format bridge."""

from __future__ import annotations

import io
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.asm_sections import assemble, neutral_sections
from scripts.kf.compile import compile_source
from scripts.kf.retail import write_tsv


def elf(path):
    return ELFFile(io.BytesIO(path.read_bytes()))


class DirectiveTests(unittest.TestCase):
    def test_directives_and_instructions_are_preserved(self):
        text = '.text\n.align 2\nnop\n.section .data # data\n.align 5\n.word external+4\n'
        mapped, names = neutral_sections(text)
        self.assertEqual(names, ('.text', '.data'))
        self.assertIn('.align 5\n.word external+4\n', mapped)
        self.assertIn('.align 2\nnop\n', mapped)
        self.assertIn(' # data\n', mapped)

    def test_unmodeled_section_traversal_is_rejected(self):
        for text in ('.text 1', '.section .data,"aw"', '.previous', '.subsection 2',
                     '.pushsection .data', '.popsection', '.kf_compiler.text:'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                neutral_sections(text)


@unittest.skipUnless(shutil.which('mipsel-linux-gnu-as') and shutil.which('mipsel-linux-gnu-ld'),
                     'requires pinned GNU MIPS tools')
class AssemblyControls(unittest.TestCase):
    def test_explicit_alignment_padding_bss_extent_and_relocation_survive_a_real_link(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            obj = root / 'source.o'
            report = assemble('.text\n.align 2\n.globl entry\nentry:\n.set noreorder\n'
                              'jr $31\nnop\n.data\n.globl pointer\n.byte 127\n.align 3\n'
                              'pointer:\n.word external+4\n.bss\n.balign 32\n.space 9\n', obj)
            self.assertEqual(report, {'.text': {'size': 8, 'alignment': 4},
                                      '.data': {'size': 12, 'alignment': 8},
                                      '.bss': {'size': 9, 'alignment': 32}})
            (root / 'link.ld').write_text('external = 0x80017ffc; SECTIONS { '
                                         '.text 0x80010004 : { *(.text) } '
                                         '.data : { *(.data) } .bss : { *(.bss) } '
                                         '/DISCARD/ : { *(*) } }')
            output = root / 'linked.elf'
            subprocess.run(['mipsel-linux-gnu-ld', '-EL', '-G0', '--entry', 'entry',
                            '-T', str(root / 'link.ld'), '-o', str(output), str(obj)],
                           check=True, capture_output=True)
            linked = elf(output)
            text = linked.get_section_by_name('.text')
            data = linked.get_section_by_name('.data')
            bss = linked.get_section_by_name('.bss')
            self.assertEqual((text['sh_addr'], text.data()),
                             (0x80010004, struct.pack('<II', 0x03e00008, 0)))
            self.assertEqual((data['sh_addr'], data.data()),
                             (0x80010010, b'\x7f' + bytes(7) + struct.pack('<I', 0x80018000)))
            self.assertEqual((bss['sh_addr'], bss['sh_size'], bss['sh_type']),
                             (0x80010020, 9, 'SHT_NOBITS'))

    def test_typed_words_still_align_without_an_explicit_directive(self):
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / 'source.o'
            result = assemble('.data\n.byte 1\n.word 0x12345678\n', obj)
            self.assertEqual(result['.data'], {'size': 8, 'alignment': 4})
            self.assertEqual(elf(obj).get_section_by_name('.data').data(),
                             b'\x01\0\0\0\x78\x56\x34\x12')
            self.assertIsNone(elf(obj).get_section_by_name('.bss'))


@unittest.skipUnless(all(shutil.which(tool) for tool in (
    'cpppsx-257', 'cc1psx-257', 'cpppsx-260', 'cc1psx-260', 'maspsx', 'mipsel-linux-gnu-as',
)), 'requires both pinned native compilers')
class CompilerControls(unittest.TestCase):
    def test_both_compilers_keep_payloads_and_the_separate_common_contract(self):
        for version in ('257', '260'):
            with self.subTest(version=version), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = root / 'source.c'
                source.write_text('int value = 7; double wide = 1.0;\n'
                                  'static int state;\n'
                                  'int read_value(void) { state += value; return state; }\n')
                write_tsv(root / 'delink/psx/objects.tsv', ('object', 'scope'),
                          ({'object': 'objects/source.o', 'scope': 'decomp'},), ())
                output = root / 'source.o'
                args = (source, 'PSX.EXE', output, root / 'delink')
                compile_source(*args, optimization='O2', compiler=f'gcc{version}-native')
                ordinary = elf(output)
                compile_source(*args, optimization='O2', compiler=f'gcc{version}-native',
                               section_alignment='directives')
                direct = elf(output)
                for name in ('.text', '.data', '.bss'):
                    before, after = ordinary.get_section_by_name(name), direct.get_section_by_name(name)
                    self.assertEqual(before['sh_size'], after['sh_size'])
                    self.assertEqual(before.data(), after.data())
                self.assertEqual(direct.get_section_by_name('.text')['sh_addralign'], 4)
                self.assertEqual(direct.get_section_by_name('.data')['sh_addralign'], 8)
                # COMMON's implicit requirements are a separate contract;
                # this bridge only removes unused default BSS sections.
                self.assertEqual(direct.get_section_by_name('.bss')['sh_addralign'], 16)
                names = ('value', 'wide', 'state', 'read_value')
                for name in names:
                    a = ordinary.get_section_by_name('.symtab').get_symbol_by_name(name)[0]
                    b = direct.get_section_by_name('.symtab').get_symbol_by_name(name)[0]
                    self.assertEqual((a['st_value'], a['st_info']), (b['st_value'], b['st_info']))


if __name__ == '__main__':
    unittest.main()
