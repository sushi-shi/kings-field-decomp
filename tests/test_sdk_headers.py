"""Original SDK header ownership and historical C compiler controls."""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


REPO = Path(__file__).resolve().parents[1]


class SdkHeaderControls(unittest.TestCase):
    @unittest.skipUnless(os.environ.get('PSYQ_INCLUDE') and shutil.which('cpppsx-257')
                         and shutil.which('cc1psx-257'), 'pinned SDK and GCC 2.5.7 required')
    def test_kernel_layout_comes_from_sdk_and_compiles_with_historical_c(self):
        sdk = Path(os.environ['PSYQ_INCLUDE'])
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'kernel.c'
            source.write_text('''#include <psyq/kernel.h>
#include <psyq/kernel.h>
typedef char exec_size[sizeof(struct EXEC) == 60 ? 1 : -1];
unsigned long text_address(struct EXEC *header) { return header->t_addr; }
unsigned long stack_address(struct EXEC *header) { return header->s_addr; }
unsigned long base_address(struct EXEC *header) { return header->base; }
''')
            text = subprocess.check_output([
                'cpppsx-257', '-lang-c', '-undef', '-nostdinc',
                '-I', str(REPO / 'include'), '-I', str(REPO / 'vendor/include'),
                '-I', str(sdk), str(source)], text=True)
            owner = None
            definitions = []
            for line in text.splitlines():
                if marker := re.match(r'#\s+\d+\s+"([^"]+)"', line):
                    owner = Path(marker[1])
                if re.match(r'struct EXEC\s*\{', line):
                    definitions.append(owner)
            self.assertEqual(definitions, [sdk / 'KERNEL.H'])
            preprocessed = root / 'kernel.i'
            preprocessed.write_text(text)
            assembly = root / 'kernel.s'
            subprocess.run(['cc1psx-257', '-quiet', '-O2', '-G0', str(preprocessed),
                            '-o', str(assembly)], check=True, capture_output=True)
            for offset in (8, 32, 56):
                self.assertRegex(assembly.read_text(), rf'lw\s+\$2,{offset}\(\$4\)')
