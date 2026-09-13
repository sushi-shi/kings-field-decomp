"""Run the original GCC MIPS varargs headers across O32 argument boundaries."""

import importlib.util
import os
from pathlib import Path
import shutil
import tempfile
import unittest

from scripts.kf.executable import cpe_loads
from scripts.psxbuild.sdk import assemble, compile_classic, dos_run, tool_succeeded


REPO = Path(__file__).resolve().parents[1]


class ClassicVarargsControls(unittest.TestCase):
    @unittest.skipUnless(os.environ.get('PSYQ_C_INCLUDE') and os.environ.get('PSYQ_BIN')
                         and shutil.which('cpppsx-257') and shutil.which('cc1psx-257')
                         and shutil.which('dosbox-x') and importlib.util.find_spec('unicorn'),
                         'original compiler headers and native PS1 build tools required')
    def test_original_headers_handle_promotions_registers_stack_and_wide_alignment(self):
        from unicorn import Uc, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_LITTLE_ENDIAN
        from unicorn.mips_const import UC_MIPS_REG_SP, UC_MIPS_REG_RA, UC_MIPS_REG_V0, UC_MIPS_REG_PC

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            # A project header must never override the compiler's original.
            shadow = root / 'shadow'
            shadow.mkdir()
            (shadow / 'stdarg.h').write_text('#error project stdarg was used\n')
            report = compile_classic(
                REPO / 'tests/fixtures/classic_varargs.c', root, 'CONTROL',
                compiler='gcc257-native', optimization='O2', small_data=0,
                include_dirs=(shadow,))
            text = (root / 'CONTROL.I').read_text()
            headers = Path(os.environ['PSYQ_C_INCLUDE'])
            for name in ('stdarg.h', 'va-mips.h'):
                self.assertIn(str(headers / name), text)
                self.assertIn(name, report['compiler_headers'])
            assemble(root, 'CONTROL', 0)
            shutil.copyfile(Path(os.environ['PSYQ_BIN']) / 'PSYLINK.EXE', root / 'PSYLINK.EXE')
            (root / 'CONTROL.LNK').write_bytes(
                b'\torg $80010000\r\n\tinclude "CONTROL.OBJ"\r\n\tregs pc=control_entry\r\n')
            dos_run(root, ['psylink /c @CONTROL.LNK,CONTROL.CPE > LINK.TXT'], 'link')
            tool_succeeded(root, 'LINK.TXT', 'CONTROL.CPE', b'CPE\x01')
            entry, loads = cpe_loads((root / 'CONTROL.CPE').read_bytes())
            self.assertIsNotNone(entry)
            machine = Uc(UC_ARCH_MIPS, UC_MODE_MIPS32 | UC_MODE_LITTLE_ENDIAN)
            machine.mem_map(0, 0x200000)
            for address, data in loads:
                machine.mem_write(address & 0x1fffffff, data)
            stop = 0x801ff000
            machine.reg_write(UC_MIPS_REG_SP, 0x801fe000)
            machine.reg_write(UC_MIPS_REG_RA, stop)
            machine.emu_start(entry, stop, count=10000)
            self.assertEqual(machine.reg_read(UC_MIPS_REG_PC), stop)
            self.assertEqual(machine.reg_read(UC_MIPS_REG_V0), 0)

    def test_header_version_must_match_compiler(self):
        # The public build helper must fail rather than silently use a mismatched version.
        with self.assertRaisesRegex(ValueError, 'GCC 2.5.7'):
            compile_classic(Path('unused.c'), Path('.'), 'UNUSED',
                            compiler='gcc260-native', include_dirs=())
