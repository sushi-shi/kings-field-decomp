"""Native SDK interrupt dispatch must leave new VBlank events for the BIOS."""

import importlib.util
import os
from pathlib import Path
import shutil
import tempfile
import unittest

from scripts.kf.executable import cpe_loads
from scripts.psxbuild.link import build_image
from scripts.psxbuild.sdk import compile_c
from scripts.psxbuild.sdk_compat import IRQ_REPEAT_BRANCH, library_input
from tests.psylink_order_smoke import symbol_address


REPO = Path(__file__).resolve().parents[1]


class SdkInterruptControls(unittest.TestCase):
    def test_unknown_archive_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'pinned Release 2.5'):
            library_input('LIBETC.LIB', b'unknown SDK version')
        self.assertEqual(library_input('LIBGPU.LIB', b'unchanged'), (b'unchanged', []))

    @unittest.skipUnless(os.environ.get('PSYQ_LIB') and shutil.which('cc1psx-257')
                         and shutil.which('dosbox-x') and importlib.util.find_spec('unicorn'),
                         'pinned compiler, SDK, DOS tools and Unicorn required')
    def test_new_vblank_reaches_bios_after_native_sdk_dispatch(self):
        from unicorn import (Uc, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_LITTLE_ENDIAN,
                             UC_HOOK_CODE, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE)
        from unicorn.mips_const import UC_MIPS_REG_SP, UC_MIPS_REG_S3

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'control.c'
            source.write_text('extern void ResetCallback(void);\n'
                              'int main(void) { ResetCallback(); return 0; }\n')

            def compile_one(unit, output, index):
                return compile_c(source, output, 'CONTROL', compiler='gcc257-native',
                                 optimization='O2', small_data=0, include_dirs=())

            output = root / 'linked'
            report = build_image('GAME.EXE', output, [{}], compile_one, repo=REPO,
                                 load_address=0x80012000,
                                 bounds_source='config/link/overlay_bounds.asm')
            self.assertTrue(report['linked'], report.get('error'))
            library = next(x for x in report['libraries'] if x['file'] == 'LIBETC.LIB')
            self.assertNotEqual(library['sha256'], library['link_input_sha256'])
            self.assertEqual(library['corrections'][0]['name'], 'bios-interrupt-return')
            _, loads = cpe_loads((output / 'GAME.CPE').read_bytes())
            symbols = output / 'GAME.SYM'
            dispatcher = symbol_address(symbols, 'intInit', case_sensitive=True)
            bios_return = symbol_address(symbols, 'ReturnFromException', case_sensitive=True)

            def dispatch(original, pending):
                cpu = Uc(UC_ARCH_MIPS, UC_MODE_MIPS32 | UC_MODE_LITTLE_ENDIAN)
                cpu.mem_map(0, 0x200000)
                cpu.mem_map(0x1f801000, 0x1000)
                for address, data in loads:
                    cpu.mem_write(address & 0x1fffffff, data)
                branch = dispatcher + 0x190
                self.assertEqual(bytes(cpu.mem_read(branch & 0x1fffffff, 4)), b'\0' * 4)
                if original:
                    cpu.mem_write(branch & 0x1fffffff, IRQ_REPEAT_BRANCH)
                cpu.reg_write(UC_MIPS_REG_SP, 0x801fe000)
                cpu.reg_write(UC_MIPS_REG_S3, 0x801fd000)
                cpu.mem_write(0x1f801074, (1).to_bytes(4, 'little'))
                state = {'pending': pending, 'returned': False}

                def read_irq(machine, access, address, size, value, user):
                    if address == 0x1f801070:
                        machine.mem_write(address, state['pending'].to_bytes(size, 'little'))

                def acknowledge_irq(machine, access, address, size, value, user):
                    if address == 0x1f801070:
                        state['pending'] &= value

                def returned(machine, address, size, user):
                    if address == bios_return:
                        state['returned'] = True
                        machine.emu_stop()

                cpu.hook_add(UC_HOOK_MEM_READ, read_irq)
                cpu.hook_add(UC_HOOK_MEM_WRITE, acknowledge_irq)
                cpu.hook_add(UC_HOOK_CODE, returned)
                # A VBlank arrives after its slot was checked during this pass.
                cpu.emu_start(dispatcher + 0x170, 0, count=1000)
                self.assertTrue(state['returned'])
                return state['pending']

            self.assertEqual(dispatch(original=True, pending=1), 0)
            self.assertEqual(dispatch(original=False, pending=1), 1)
            self.assertEqual(dispatch(original=False, pending=0), 0)
