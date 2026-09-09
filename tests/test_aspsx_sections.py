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
