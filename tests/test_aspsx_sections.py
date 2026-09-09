"""ASPSX section constraints come from native tools, never retail claims."""

from __future__ import annotations

import io
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.compile import aspsx_section_alignment
from scripts.kf.executable import cpe_loads, dos_run, tool_succeeded


@unittest.skipUnless(os.environ.get('PSYQ_ASPSX') and os.environ.get('PSYQ_BIN')
                     and all(shutil.which(tool) for tool in (
                         'dosbox-x', 'psyk', 'maspsx', 'mipsel-linux-gnu-as')),
                     'pinned original assembler/linker environment required')
class NativeAspsxSectionTests(unittest.TestCase):
    def test_section_constraints_and_relative_alignment_against_original_tools(self):
        with tempfile.TemporaryDirectory(prefix='kf-aspsx-sections-') as directory:
            root = Path(directory)
            shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
            shutil.copyfile(Path(os.environ['PSYQ_BIN']) / 'PSYLINK.EXE',
                            root / 'PSYLINK.EXE')
            commands = []
            sources = {}
            for exponent in range(5):
                stem = f'A{exponent}'
                source = (f'\t.data\n\t.globl first\nfirst:\n\t.byte 17\n'
                          f'\t.align {exponent}\n\t.globl second\nsecond:\n'
                          '\t.byte 34,0,51\n')
                sources[stem] = source
                (root / (stem + '.S')).write_bytes(source.replace('\n', '\r\n').encode())
                commands.extend((
                    f'aspsx -G0 -o {stem}.OBJ {stem}.S > {stem}.TXT',
                    f'psylink /o$80010004 {stem}.OBJ,{stem}.CPE,,{stem}.MAP > {stem}.LOG',
                ))
            dos_run(root, commands, 'probe')
            for exponent in range(5):
                with self.subTest(exponent=exponent):
                    stem = f'A{exponent}'
                    tool_succeeded(root, stem + '.TXT', stem + '.OBJ', b'LNK\x02')
                    tool_succeeded(root, stem + '.LOG', stem + '.CPE', b'CPE\x01')
                    listing = subprocess.check_output(
                        ['psyk', 'list', '--code', str(root / (stem + '.OBJ'))], text=True)
                    declarations = re.findall(
                        r"Section symbol number [0-9a-f]+ '([^']+)' in group 0 alignment (\d+)",
                        listing)
                    self.assertEqual(dict(declarations), {
                        name: '8' for name in ('.rdata', '.text', '.data', '.sdata', '.sbss', '.bss')
                    })
                    expected = b'\x11' + bytes((1 << exponent) - 1) + b'\x22\x00\x33'
                    _, loads = cpe_loads((root / (stem + '.CPE')).read_bytes())
                    # In particular, .align 4 does not require a 16-byte base:
                    # PSYLINK places this complete contribution at ...0004.
                    self.assertEqual(loads, [(0x80010004, expected)])
                    output = root / (stem + '.o')
                    subprocess.run([
                        'maspsx', '--aspsx-version=1.07', '--run-assembler', '--force-stdin',
                        '-march=r3000', '-mabi=32', '-no-pad-sections', '-o', str(output),
                    ], input=sources[stem].encode(), capture_output=True, check=True)
                    original = output.read_bytes()
                    converted, report = aspsx_section_alignment(original, '1.07')
                    native_elf = ELFFile(io.BytesIO(converted))
                    self.assertEqual(native_elf.get_section_by_name('.data').data(), expected)
                    self.assertEqual(native_elf.get_section_by_name('.data')['sh_addralign'], 4)
                    self.assertEqual(report['method'], 'aspsx-1.07-section-declarations')
                    # The conversion changes only the declared alignment fields.
                    self.assertEqual(len(original), len(converted))
                    old_elf = ELFFile(io.BytesIO(original))
                    for old, new in zip(old_elf.iter_sections(), native_elf.iter_sections()):
                        self.assertEqual(old.name, new.name)
                        self.assertEqual(old.data(), new.data())
                        for key in old.header:
                            if key != 'sh_addralign':
                                self.assertEqual(old[key], new[key])

    def test_unknown_assembler_version_cannot_borrow_alignment_calibration(self):
        with self.assertRaisesRegex(ValueError, 'ASPSX 1.07'):
            aspsx_section_alignment(b'', '2.21')

    def test_small_private_reservations_and_absolute_references(self):
        """ASPSX 1.07 -G chooses private storage; it does not enable GP macros."""
        sizes = (1, 3, 2, 4, 5, 8, 9, 16, 1)
        source = ('\t.sdata\n\t.globl initialized\ninitialized:\n\t.word 3\n'
                  '\t.text\n\t.set noreorder\n\t.extern external,4\n'
                  '\t.globl probe\nprobe:\n\tlw $2,initialized\n'
                  '\tnop\n\tsw $2,local0\n\tlw $3,global0\n\tnop\n\tsw $3,initialized\n'
                  '\tla $4,initialized\n\tlw $5,external\n\tla $6,local0\n'
                  '\tlw $7,local0+4\n\tjr $31\n\tnop\n')
        for index, size in enumerate(sizes):
            source += f'\t.lcomm local{index},{size}\n\t.comm global{index},{size}\n'
        expected_text = bytes.fromhex(
            '0000023c0000428c000000000000013c000022ac0000033c0000638c000000000000013c000023ac'
            '0000043c000084240000053c0000a58c0000063c0000c6240000073c0000e78c'
            '0800e00300000000')
        with tempfile.TemporaryDirectory(prefix='kf-aspsx-small-') as directory:
            root = Path(directory)
            shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
            (root / 'INPUT.S').write_bytes(source.replace('\n', '\r\n').encode())
            dos_run(root, [f'aspsx -G{g} -o G{g}.OBJ INPUT.S > G{g}.TXT'
                           for g in (0, 8)], 'small')
            for g in (0, 8):
                with self.subTest(g=g):
                    tool_succeeded(root, f'G{g}.TXT', f'G{g}.OBJ', b'LNK\x02')
                    listing = subprocess.check_output(
                        ['psyk', 'list', '--code', str(root / f'G{g}.OBJ')], text=True)
                    section_ids = dict(re.findall(
                        r"Section symbol number ([0-9a-f]+) '([^']+)'", listing))
                    locals_native = {
                        name: (section_ids[section], int(offset, 16))
                        for name, offset, section in re.findall(
                            r"Local symbol '([^']+)' at offset ([0-9a-f]+) in section ([0-9a-f]+)",
                            listing)
                    }
                    expected = {}
                    offsets = {'.bss': 0, '.sbss': 0}
                    for index, size in enumerate(sizes):
                        section = '.sbss' if size <= g else '.bss'
                        expected[f'local{index}'] = (section, offsets[section])
                        offsets[section] += size
                    self.assertEqual(locals_native, expected)
                    common_native = {
                        name: (section_ids[section], int(size, 16))
                        for name, size, section in re.findall(
                            r"XBSS symbol number [0-9a-f]+ '([^']+)' size ([0-9a-f]+) in section ([0-9a-f]+)",
                            listing)
                    }
                    self.assertEqual(common_native,
                                     {f'global{i}': ('.bss', n) for i, n in enumerate(sizes)})
                    native_code = re.search(r'Code 80 bytes\n\n(.*?)\n\n', listing, re.S)
                    self.assertIsNotNone(native_code)
                    native_bytes = bytes.fromhex(' '.join(
                        line.split(': ', 1)[1] for line in native_code[1].splitlines()))
                    self.assertEqual(native_bytes, expected_text)
                    self.assertEqual(re.findall(r'Patch type (\d+)', listing), ['82', '84'] * 8)
                    output = root / f'G{g}.o'
                    subprocess.run([
                        'maspsx', '--aspsx-version=1.07', '--use-comm-section',
                        f'-G{g}', '--run-assembler', '--force-stdin', '-march=r3000',
                        '-mabi=32', '-no-pad-sections', '-o', str(output),
                    ], input=source.encode(), capture_output=True, check=True)
                    elf = ELFFile(io.BytesIO(output.read_bytes()))
                    symbols = {symbol.name: symbol for symbol in
                               elf.get_section_by_name('.symtab').iter_symbols()}
                    for name, (section, offset) in expected.items():
                        symbol = symbols[name]
                        self.assertEqual(elf.get_section(symbol['st_shndx']).name, section)
                        self.assertEqual(symbol['st_value'], offset)
                        self.assertEqual(symbol['st_info']['bind'], 'STB_LOCAL')
                    for index, size in enumerate(sizes):
                        symbol = symbols[f'global{index}']
                        self.assertEqual(symbol['st_shndx'], 'SHN_COMMON')
                        self.assertEqual(symbol['st_size'], size)
                    for section, size in offsets.items():
                        entry = elf.get_section_by_name(section)
                        self.assertEqual(entry['sh_size'] if entry else 0, size)
                    # ELF REL carries +4 in the low instruction; LNK stores it
                    # in the patch expression. All eight references stay absolute.
                    adapter_text = bytearray(elf.get_section_by_name('.text').data())
                    self.assertEqual(adapter_text[68:70], b'\x04\x00')
                    adapter_text[68:70] = b'\x00\x00'
                    self.assertEqual(adapter_text, expected_text)
                    relocs = elf.get_section_by_name('.rel.text')
                    self.assertEqual([r['r_info_type'] for r in relocs.iter_relocations()],
                                     [5, 6] * 8)
