"""Complete-file comparison and PS-X serialization controls."""

from __future__ import annotations

import io
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

from elftools.elf.elffile import ELFFile

from scripts.kf.executable import compare, serialize, undefined
from scripts.kf.mips_elf import DefinedSymbol, write_mips_elf
from scripts.kf.sema.image import RetailImage


class ComparisonTests(unittest.TestCase):
    def test_file_tail_and_header_are_compared_without_masks(self):
        image = RetailImage.synthetic('PSX.EXE', 0x80010000, b'abcdefgh')
        self.assertTrue(compare(image.data, image)['file_equal'])
        for actual, count in ((image.data[:-3], 3), (image.data + b'\0\0', 2)):
            self.assertEqual(compare(actual, image)['differing_bytes'], count)
        changed = bytearray(image.data)
        changed[0x10] ^= 4
        changed[-1] ^= 1
        report = compare(bytes(changed), image)
        self.assertEqual(report['differing_bytes'], 2)
        self.assertEqual(report['header_differing_bytes'], 1)
        self.assertEqual(report['load_differing_bytes'], 1)
        self.assertIsNone(report['first_difference_va'])

    def test_payload_offset_is_qualified_by_selected_image_base(self):
        image = RetailImage.synthetic('OPEN.EXE', 0x80012000, b'abcdefgh')
        report = compare(image.data[:-1] + b'!', image)
        self.assertEqual(report['first_difference_va'], 0x80012007)

    def test_linker_missing_symbols_are_deduplicated(self):
        self.assertEqual(undefined("undefined reference to `x'\nundefined reference to `x'\n"
                                   "undefined reference to `y'"), ['x', 'y'])

    def test_relocatable_object_cannot_be_packaged_as_an_executable(self):
        image = RetailImage.synthetic('PSX.EXE', 0x80010000, bytes(8))
        obj = write_mips_elf(bytes(8), 'entry', 8)
        with self.assertRaisesRegex(ValueError, 'linked little-endian'):
            serialize(ELFFile(io.BytesIO(obj)), image)


@unittest.skipUnless(shutil.which('mipsel-linux-gnu-ld'), 'requires pinned GNU MIPS linker')
class SerializationTests(unittest.TestCase):
    def test_linked_load_bytes_entry_sector_padding_and_bss_extent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            text = struct.pack('<2I', 0x03e00008, 0)
            obj = write_mips_elf(text, 'entry', len(text), data=b'DATA', bss_size=32,
                                 bss_symbols=(DefinedSymbol('state', 0, 32),))
            (root / 'input.o').write_bytes(obj)
            (root / 'link.ld').write_text(
                'SECTIONS { .text 0x80010000 : { *(.text) } '
                '.data 0x80010008 : { *(.data) } '
                '.bss 0x80011000 : { *(.bss) } /DISCARD/ : { *(*) } }')
            subprocess.run(['mipsel-linux-gnu-ld', '-EL', '--entry', 'entry',
                            '-T', str(root / 'link.ld'), '-o', str(root / 'linked.elf'),
                            str(root / 'input.o')], check=True, capture_output=True)
            image = RetailImage.synthetic('PSX.EXE', 0x80010000, bytes(8))
            linked = ELFFile(io.BytesIO((root / 'linked.elf').read_bytes()))
            actual, sections = serialize(linked, image)
            self.assertEqual(len(actual), 0x1000)
            self.assertEqual(actual[:8], b'PS-X EXE')
            self.assertEqual(struct.unpack_from('<I', actual, 0x10)[0], 0x80010000)
            self.assertEqual(struct.unpack_from('<II', actual, 0x18), (0x80010000, 0x800))
            self.assertEqual(actual[0x800:0x80c], text + b'DATA')
            self.assertEqual(actual[0x80c:], bytes(0x7f4))
            self.assertIn({'name': '.bss', 'address': 0x80011000, 'size': 32,
                           'storage': 'bss'}, sections)


if __name__ == '__main__':
    unittest.main()
