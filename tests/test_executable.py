"""Direct source -> ASPSX -> PSYLINK -> CPE2X executable controls."""

from __future__ import annotations

import contextlib
import io
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest import mock

from scripts.kf import executable
from scripts.kf.executable import build_image, compare, cpe_loads
from scripts.kf.manifest import Manifest, Profile, Unit
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

    def test_inferred_padding_mode_is_no_longer_available(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
            executable.main(['--diagnostic-cpe-padding'])
        self.assertEqual(error.exception.code, 2)

    def test_cpe_inspection_keeps_load_order_and_rejects_truncation(self):
        entry = 0x80010020
        cpe = (b'CPE\x01\x08\x00\x03\x90\x00' + struct.pack('<I', entry)
               + b'\x01' + struct.pack('<II', entry, 3) + b'ABC'
               + b'\x01' + struct.pack('<II', 0x80010000, 4) + b'DATA' + b'\0')
        self.assertEqual(cpe_loads(cpe), (entry, [(entry, b'ABC'), (0x80010000, b'DATA')]))
        for broken in (cpe[:-1], cpe[:-2], cpe + b'\0', b'CPE\x01\xff\0'):
            with self.subTest(broken=broken), self.assertRaises(ValueError):
                cpe_loads(broken)


@unittest.skipUnless(all(shutil.which(tool) for tool in ('cpppsx-257', 'cc1psx-257', 'dosbox-x'))
                     and all(os.environ.get(key) for key in ('PSYQ_ASPSX', 'PSYQ_BIN', 'PSYQ_LIB',
                                                             'PSYQ_INCLUDE')),
                     'requires pinned compiler, original DOS tools and SDK libraries')
class NativeBuildControls(unittest.TestCase):
    def manifest(self, root: Path, sources: tuple[str, ...]) -> Manifest:
        profile = Profile('native-control', 'c', 'gcc257-native', 'O2', 0, '1.07', ('-mcpu=r2000',))
        units = []
        for index, source in enumerate(sources):
            path = root / f'source{index}.c'
            path.write_text(source)
            units.append(Unit(f'psx.control{index}', 'PSX.EXE', str(path), profile.name, ()))
        return Manifest({profile.name: profile}, tuple(units))

    def test_source_objects_and_original_archives_link_without_retail_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'extern int helper(void); int main(void) { return helper(); }\n',
                'int value = 7; int helper(void) { return value; }\n',
            ))
            output = root / 'output'
            with mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail input used')):
                report = build_image('PSX.EXE', manifest, output)
            self.assertTrue(report['linked'], report.get('error'))
            self.assertFalse(report['output_rewritten'])
            self.assertEqual(report['retail_payload_inputs'], [])
            self.assertEqual([row['file'] for row in report['libraries']], ['LIBSN.LIB', 'LIBAPI.LIB'])
            self.assertTrue((output / 'U0000.OBJ').read_bytes().startswith(b'LNK\x02'))
            self.assertTrue((output / 'U0001.OBJ').read_bytes().startswith(b'LNK\x02'))
            entry, loads = cpe_loads((output / 'PSX.CPE').read_bytes())
            actual = (output / 'PSX.EXE').read_bytes()
            self.assertEqual(actual[:8], b'PS-X EXE')
            self.assertEqual(struct.unpack_from('<I', actual, 0x10)[0], entry)
            base = struct.unpack_from('<I', actual, 0x18)[0]
            for address, payload in loads:
                offset = 2048 + address - base
                self.assertEqual(actual[offset:offset + len(payload)], payload)
            end = max(address + len(payload) for address, payload in loads)
            self.assertEqual(actual[2048 + end - base:], bytes(len(actual) - 2048 - end + base))
            for row in report['libraries']:
                self.assertEqual((output / row['file']).read_bytes(), Path(row['path']).read_bytes())

    def test_unresolved_source_symbol_fails_without_fallback_or_stale_executable(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'extern int missing_function(void); int main(void) { return missing_function(); }\n',
            ))
            output = root / 'output'
            output.mkdir()
            (output / 'PSX.EXE').write_bytes(b'stale result')
            report = build_image('PSX.EXE', manifest, output)
            self.assertFalse(report['linked'])
            self.assertEqual(report['phase'], 'link')
            self.assertIn('missing_function', report['error'])
            self.assertFalse((output / 'PSX.EXE').exists())
            self.assertEqual(report['retail_payload_inputs'], [])


if __name__ == '__main__':
    unittest.main()
