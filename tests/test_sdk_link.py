"""Original-PSYLINK and GNU-ld controls for executable library inputs."""

from __future__ import annotations

import io
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.sdk_link import convert, expression, parse, verify_linked


def native_control() -> bytes:
    blob = bytearray(b'LNK\x02\x2e\x07')
    blob += struct.pack('<BHHBB', 16, 1, 0, 8, 5) + b'.text'
    blob += struct.pack('<BH', 6, 1)
    payload = struct.pack('<8I', 0x3c080000, 0x25080000, 0x0c000000, 0, 0, 0, 0, 0)
    blob += struct.pack('<BH', 2, len(payload)) + payload
    symbol = b'\2' + struct.pack('<H', 2)

    def constant(value):
        return b'\0' + struct.pack('<I', value)

    high = b'\x2c' + constant(0x8004) + symbol
    patches = ((82, 0, high), (84, 4, high), (74, 8, symbol),
               (16, 12, b'\x2e' + constant(4) + symbol),
               (16, 16, b'\x2e' + constant(4) + constant(20)))
    for kind, offset, expr in patches:
        blob += struct.pack('<BBH', 10, kind, offset) + expr
    blob += struct.pack('<BHHIB', 12, 2, 1, 24, 8) + b'external'
    return bytes(blob) + b'\0'


LISTING = """Header : LNK version 2
46 : Processor type 7
16 : Section symbol number 1 '.text' in group 0 alignment 8
6 : Switch to section 1
2 : Code 8 bytes
0000: 00 00 08 3c 00 00 08 25
10 : Patch type 82 at offset 0 with ($4-[2])
10 : Patch type 84 at offset 4 with ($4-[2])
14 : XREF symbol number 2 'external'
0 : End of file
"""


class ParserTests(unittest.TestCase):
    def test_native_subtraction_order_and_negative_addend(self):
        member = parse(LISTING)
        self.assertEqual(expression('($4-[2])', member, 'test'), ('external', 0xfffffffc))
        self.assertEqual(expression('($4-$14)', member, 'test'), (None, 16))

    def test_unknown_alignment_version_and_patch_are_rejected(self):
        for old, new in (('alignment 8', 'alignment 32'), ('version 2', 'version 3'),
                         ('Patch type 82', 'Patch type 99')):
            with self.subTest(new=new), self.assertRaises(ValueError):
                parse(LISTING.replace(old, new))

    def test_truncated_extra_and_duplicate_patch_records_are_rejected(self):
        for listing in (LISTING.replace('00 00 08 3c 00 00 08 25', '00 00 08 3c'),
                        LISTING.replace('0 : End of file', ''),
                        LISTING + '6 : Switch to section 1\n',
                        LISTING.replace('at offset 4', 'at offset 0'),
                        LISTING.replace('at offset 4', 'at offset 8')):
            with self.subTest(listing=listing), self.assertRaises(ValueError):
                parse(listing)

    def test_patch_offsets_follow_the_current_code_record(self):
        listing = LISTING.replace('0 : End of file',
                                 '2 : Code 4 bytes\n0000: 00 00 00 00\n'
                                 '10 : Patch type 16 at offset 0 with [2]\n0 : End of file')
        self.assertEqual(parse(listing).sections['1'].patches[-1][:2], (8, 16))

    def test_sdk_reserved_holes_remain_distinct_from_supplied_bytes(self):
        listing = LISTING.replace("'.text'", "'.data'").replace(
            '0 : End of file', '8 : Uninitialized data, 11 bytes\n0 : End of file')
        section = parse(listing).sections['1']
        self.assertEqual(section.size, 19)
        self.assertEqual(section.reservations, [(8, 11)])


@unittest.skipUnless(shutil.which('psyk') and shutil.which('mipsel-linux-gnu-ld'),
                     'requires pinned SDK and GNU MIPS tools')
class LinkControls(unittest.TestCase):
    def test_signed_carry_local_jump_and_pointer_addends(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'TEST.OBJ').write_bytes(native_control())
            listing = subprocess.run(['psyk', 'list', '--code', str(root / 'TEST.OBJ')],
                                     check=True, capture_output=True, text=True).stdout
            (root / 'test.txt').write_text(listing)
            conversion = convert(parse(listing), root / 'test.o', identity='synthetic')
            (root / 'test.ld').write_text(
                'SECTIONS { .text 0x80010000 : { *(.text) } /DISCARD/ : { *(*) } }')
            subprocess.run(['mipsel-linux-gnu-ld', '-EL', '-G0', '-T', str(root / 'test.ld'),
                            '-o', str(root / 'linked.elf'), str(root / 'test.o')], check=True,
                           capture_output=True)
            linked = ELFFile(io.BytesIO((root / 'linked.elf').read_bytes()))
            expected = struct.pack('<8I', 0x3c088002, 0x2508801c, 0x0c004006,
                                   0x80010014, 16, 0, 0, 0)
            self.assertEqual(linked.get_section_by_name('.text').data(), expected)
            report = verify_linked(linked, {'members': [conversion]})
            self.assertEqual(report['provided_bytes_verified'], 32)
            self.assertEqual(report['relocations_verified'], 5)
            corrupted = bytearray((root / 'linked.elf').read_bytes())
            corrupted[linked.get_section_by_name('.text')['sh_offset'] + 12] ^= 4
            with self.assertRaisesRegex(ValueError, 'SDK linked-byte/fixup mismatch'):
                verify_linked(ELFFile(io.BytesIO(corrupted)), {'members': [conversion]})

    @unittest.skipUnless(shutil.which('dosbox-x') and os.environ.get('PSYQ_BIN'),
                         'requires original PSYLINK through flake DOSBox-X')
    def test_original_psylink_independently_proves_the_complete_control(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'TEST.OBJ').write_bytes(native_control())
            shutil.copy2(Path(os.environ['PSYQ_BIN']) / 'PSYLINK.EXE', root / 'PSYLINK.EXE')
            (root / 'RUN.BAT').write_bytes(
                b'psylink /o$80010000 TEST.OBJ,TEST.CPE,TEST.SYM,TEST.MAP > TEST.TXT\r\n')
            environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                               XDG_CONFIG_HOME=str(root / 'config'))
            subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                            '-c', f'mount c {root}', '-c', 'c:', '-c', 'RUN.BAT', '-exit'],
                           cwd=root, env=environment, capture_output=True, check=True, timeout=30)
            transcript = (root / 'TEST.TXT').read_text()
            self.assertIn('PSYLINK version 1.17', transcript)
            self.assertIn('0 error(s)', transcript)
            payload = struct.pack('<8I', 0x3c088002, 0x2508801c, 0x0c004006,
                                  0x80010014, 16, 0, 0, 0)
            self.assertEqual((root / 'TEST.CPE').read_bytes(),
                             b'CPE\x01\x08\x00\x01' + struct.pack('<II', 0x80010000, 32)
                             + payload + b'\0')


if __name__ == '__main__':
    unittest.main()
