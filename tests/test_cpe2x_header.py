"""Execute the pinned CPE2X header writer with two independent stack seeds."""

import hashlib
import os
from pathlib import Path
import struct
import unittest

from unicorn import Uc, UC_ARCH_X86, UC_MODE_16, UC_HOOK_CODE, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (
    UC_X86_REG_AX, UC_X86_REG_DX, UC_X86_REG_CS, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_IP,
)


def trace_writer(image, fill):
    """Execute original field stores and strcpy; stub only printf/file I/O.

    This controls prior stack contents, not a full DOS process or its history.
    Never writes a production EXE or supplies retail bytes to the converter.
    """
    uc = Uc(UC_ARCH_X86, UC_MODE_16)
    uc.mem_map(0, 0x100000)
    uc.mem_write(0, image)
    ds, ss, sp = 0x33a, 0x4000, 0x8000
    for reg, value in [(UC_X86_REG_CS, 0), (UC_X86_REG_DS, ds),
                       (UC_X86_REG_SS, ss), (UC_X86_REG_SP, sp)]:
        uc.reg_write(reg, value)
    uc.mem_write(ss * 16 + sp - 0x300, bytes([fill]) * 0x300)
    header = ss * 16 + sp - 2 - 0x8c
    for off, value in [(0xf77e, 0x800), (0xf782, 0x80010000),
                       (0xf786, 0x80010100)]:
        uc.mem_write(ds * 16 + off, struct.pack('<I', value))
    writes, captured, calls = {}, [], []

    def write_hook(uc, access, address, size, value, _):
        for byte in range(max(header, address), min(header + 0x88, address + size)):
            writes[byte - header] = uc.reg_read(UC_X86_REG_IP)

    def code_hook(uc, address, size, _):
        if address == 0x3274:  # printf; its stack lies below the header object.
            calls.append('printf (stubbed)')
            uc.reg_write(UC_X86_REG_AX, 0)
            uc.reg_write(UC_X86_REG_IP, address + 5)
        elif address == 0x3332:  # fopen: successful synthetic FILE pointer.
            calls.append('fopen (stubbed)')
            uc.reg_write(UC_X86_REG_AX, 1)
            uc.reg_write(UC_X86_REG_DX, 0)
            uc.reg_write(UC_X86_REG_IP, address + 5)
        elif address == 0x337c:  # intercept fwrite before any file mutation.
            args_at = ss * 16 + uc.reg_read(UC_X86_REG_SP)
            off, seg, count, items, file_off, file_seg = struct.unpack(
                '<6H', uc.mem_read(args_at, 12)
            )
            assert seg * 16 + off == header and count == 0x88 and items == 1
            captured.append(bytes(uc.mem_read(header, count)))
            calls.append('fwrite (captured)')
            uc.emu_stop()

    uc.hook_add(UC_HOOK_MEM_WRITE, write_hook)
    uc.hook_add(UC_HOOK_CODE, code_hook)
    uc.emu_start(0x3250, 0x339c, count=100000)
    assert len(captured) == 1
    raw = captured[0]
    untouched = [i for i in range(len(raw)) if i not in writes]
    assert all(raw[i] == fill for i in untouched)
    return {'fill': fill, 'header_hex': raw.hex(), 'untouched': untouched,
            'last_writer': {f'{i:02x}': f'{pc:04x}' for i, pc in writes.items()},
            'calls': calls}


@unittest.skipUnless(os.environ.get('PSYQ_BIN'), 'requires the pinned SDK')
class Cpe2xHeaderControls(unittest.TestCase):
    def test_reserved_words_are_uninitialized_in_the_original_writer(self):
        converter = (Path(os.environ['PSYQ_BIN']) / 'CPE2X.EXE').read_bytes()
        self.assertEqual(hashlib.sha256(converter).hexdigest(),
                         '8ee3df02d30d9269bba8c570d69f3c9d2b59aff98af0fbf796367526bc02ef20')
        image = converter[struct.unpack_from('<H', converter, 8)[0] * 16:]
        for fill in (0xa5, 0x5a):
            report = trace_writer(image, fill)
            header = bytes.fromhex(report['header_hex'])
            self.assertEqual(header[:8], b'PS-X EXE')
            self.assertEqual(header[8:16], bytes([fill]) * 8)
            self.assertTrue(set(range(8, 16)).issubset(report['untouched']))
            self.assertEqual(struct.unpack_from('<I', header, 16)[0], 0x80010100)
            self.assertEqual(struct.unpack_from('<II', header, 24), (0x80010000, 0x800))


if __name__ == '__main__':
    unittest.main()
