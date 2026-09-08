"""Complete-file comparison and PS-X serialization controls."""

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

from scripts.kf.executable import compare, sector_padding, serialize, undefined
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

    def test_container_prefix_is_bounded_by_the_remaining_sector(self):
        prefix = b'CPE\x01\x08\x00\x03\x90\x00'
        for remaining in (0, 1, 4, 8, 9, 10, 2047):
            with self.subTest(remaining=remaining):
                padding = sector_padding(4096 - remaining, 'cpe-v1-prefix')
                self.assertEqual(len(padding), remaining)
                self.assertEqual(padding[:9], prefix[:remaining])
                self.assertEqual(padding[9:], bytes(max(0, remaining - 9)))
        with self.assertRaises(ValueError):
            sector_padding(0, 'unknown')


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
            # The compatibility tail follows this synthetic link's actual end,
            # with no retail data available to copy or fixed retail VA to use.
            compatible, compatible_sections = serialize(linked, image, padding_policy='cpe-v1-prefix')
            self.assertEqual(compatible[:0x80c], actual[:0x80c])
            self.assertEqual(compatible[0x80c:0x815], b'CPE\x01\x08\x00\x03\x90\x00')
            self.assertEqual(compatible[0x815:], bytes(0x7eb))
            self.assertEqual(compatible_sections, sections)


@unittest.skipUnless(shutil.which('dosbox-x') and os.environ.get('PSYQ_RUNTIME26_BIN')
                     and os.environ.get('PSYQ_BIN'), 'requires both pinned native CPE2X candidates')
class NativeConverterControls(unittest.TestCase):
    def test_both_native_converters_emit_zero_tail_under_this_control(self):
        # This deliberately does NOT establish native provenance for our
        # inferred retail compatibility rule, despite the matching CPE prefix.
        for variable in ('PSYQ_BIN', 'PSYQ_RUNTIME26_BIN'):
            with self.subTest(candidate=variable), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                shutil.copy2(Path(os.environ[variable]) / 'CPE2X.EXE', root / 'CPE2X.EXE')
                entry = 0x80010000
                payload = struct.pack('<II', 0x03e00008, 0)
                cpe = (b'CPE\x01\x08\x00\x03\x90\x00' + struct.pack('<I', entry)
                       + b'\x01' + struct.pack('<II', entry, len(payload)) + payload + b'\0')
                (root / 'TEST.CPE').write_bytes(cpe)
                (root / 'RUN.BAT').write_bytes(b'cpe2x TEST.CPE > TEST.TXT\r\n')
                environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                                   XDG_CONFIG_HOME=str(root / 'config'))
                subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                                '-c', f'mount c {root}', '-c', 'c:', '-c', 'RUN.BAT', '-exit'],
                               cwd=root, env=environment, check=True, capture_output=True, timeout=30)
                self.assertIn('CPE2X Ver1.3', (root / 'TEST.TXT').read_text())
                actual = (root / 'TEST.EXE').read_bytes()
                self.assertEqual(len(actual), 4096)
                self.assertEqual(struct.unpack_from('<I', actual, 0x10)[0], entry)
                self.assertEqual(struct.unpack_from('<II', actual, 0x18), (entry, 2048))
                self.assertEqual(actual[2048:2056], payload)
                self.assertEqual(actual[2056:], bytes(2040))


if __name__ == '__main__':
    unittest.main()
