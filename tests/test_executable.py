"""Direct source -> ASPSX -> PSYLINK -> CPE2X executable controls."""

from __future__ import annotations

import contextlib
from dataclasses import replace
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import unittest
from unittest import mock

from scripts.kf import executable
from scripts.kf.executable import build_image, compare, cpe_loads
from scripts.kf.manifest import Manifest, Profile, Unit
from scripts.kf.sema.image import RetailImage


class ComparisonTests(unittest.TestCase):
    def test_normal_build_does_not_load_retail_compare_or_refresh_readme(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/psx'
            root.mkdir(parents=True)
            report = {'image': 'PSX.EXE', 'linked': True, 'executable': str(root / 'PSX.EXE')}
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(executable, 'load_manifest'),
                  mock.patch.object(executable, 'build_image', return_value=report),
                  mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail used')),
                  mock.patch.object(executable, 'compare', side_effect=AssertionError('comparison used')),
                  mock.patch.object(executable, 'compare_executables', side_effect=AssertionError('diff used')),
                  mock.patch.object(executable, 'refresh_executable', side_effect=AssertionError('README used')),
                  contextlib.redirect_stdout(io.StringIO())):
                self.assertEqual(executable.main(['--image', 'psx']), 0)
            self.assertEqual(json.loads((root / 'build.json').read_text()), report)
            self.assertFalse((root / 'fuzzy-comparison.json').exists())

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

    def test_compare_only_preserves_executable_and_native_build_provenance(self):
        image = RetailImage.synthetic('GAME.EXE', 0x80012000, bytes(range(128)))
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/game'
            root.mkdir(parents=True)
            exe = root / 'GAME.EXE'
            exe.write_bytes(image.data)
            original_report = root / 'build.json'
            original_report.write_bytes(b'original native build provenance\n')
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(RetailImage, 'load', return_value=image),
                  mock.patch.object(executable, 'load_manifest', side_effect=AssertionError('manifest used')),
                  mock.patch.object(executable, 'build_image', side_effect=AssertionError('build used')),
                  mock.patch.object(executable, 'refresh_executable') as refresh,
                  contextlib.redirect_stdout(io.StringIO())):
                self.assertEqual(executable.main(['--image', 'game', '--compare-only']), 0)
                refresh.assert_called_once_with(build / 'link')
            self.assertEqual(exe.read_bytes(), image.data)
            self.assertEqual(original_report.read_bytes(), b'original native build provenance\n')
            report = json.loads((root / 'fuzzy-comparison.json').read_text())
            self.assertEqual(report['mode'], 'compare-existing')
            self.assertNotIn('linked', report)
            self.assertTrue(report['comparison']['file_equal'])
            self.assertEqual(report['layout_tolerant']['byte_similarity_percent'], 100)
            self.assertEqual(report['layout_tolerant']['retail_bytes'], 128)
            html = (root / 'fuzzy-comparison.html').read_text()
            self.assertIn('Movement map', html)
            self.assertIn('80012000', html)
            self.assertIn('id="minimum"', html)


@unittest.skipUnless(all(shutil.which(tool) for tool in ('cpppsx-257', 'cc1psx-257', 'dosbox-x'))
                     and all(os.environ.get(key) for key in ('PSYQ_ASPSX', 'PSYQ_BIN', 'PSYQ_LIB',
                                                             'PSYQ_INCLUDE', 'PSYQ_H2000_LIB')),
                     'requires pinned compiler, original DOS tools and SDK libraries')
class NativeBuildControls(unittest.TestCase):
    def manifest(self, root: Path, sources: tuple[str, ...], image: str = 'PSX.EXE') -> Manifest:
        profile = Profile('native-control', 'c', 'gcc257-native', 'O2', 0, '1.07', ('-mcpu=r2000',))
        units = []
        for index, source in enumerate(sources):
            path = root / f'source{index}.c'
            path.write_text(source)
            units.append(Unit(f'{image[:-4].lower()}.control{index}', image, str(path), profile.name, ()))
        return Manifest({profile.name: profile}, tuple(units))

    def test_comparison_view_uses_the_identical_native_linker_input(self):
        from scripts.kf.compile import compile_source
        from scripts.kf.paths import REPO
        from scripts.kf.retail import write_tsv

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            manifest = self.manifest(root, (
                'static int values[2] = {7, 9}; int main(void) { return values[1]; }\n',))
            report = build_image('PSX.EXE', manifest, root / 'exe')
            self.assertTrue(report['linked'], report.get('error'))
            write_tsv(root / 'delink/psx/objects.tsv', ('object', 'scope'),
                      ({'object': 'objects/view.o', 'scope': 'decomp'},), ())
            unit, profile = manifest.units[0], next(iter(manifest.profiles.values()))
            compile_source(unit.source_path, unit.image, root / 'view.o', root / 'delink',
                           profile.optimization, profile.small_data, profile.aspsx_version,
                           (REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
                           profile.cc1_flags, profile.compiler)
            self.assertEqual((root / 'view.OBJ').read_bytes(), (root / 'exe/U0000.OBJ').read_bytes())
            self.assertTrue((root / 'view.o').read_bytes().startswith(b'\x7fELF'))

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

    def test_overlay_startup_sdk_aliases_and_bss_use_native_linker_inputs(self):
        for image in ('OPEN.EXE', 'GAME.EXE'):
            with self.subTest(image=image), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source = ('unsigned char buffer[8192]; int initialized_zero = 0;\n'
                          'extern void InitCARD2(long); extern void StartCARD2(void);\n'
                          'extern void StopCARD2(void); extern long erase(const char *);\n'
                          'int main(void) { buffer[4] = 7; ')
                if image == 'GAME.EXE':
                    source += 'InitCARD2(0); StartCARD2(); StopCARD2(); erase("bu00:test"); '
                source += 'return buffer[2] + initialized_zero; }\n'
                output = root / 'output'
                manifest = self.manifest(root, (source,), image)
                with mock.patch.object(RetailImage, 'load', side_effect=AssertionError('retail input used')):
                    report = build_image(image, manifest, output)
                self.assertTrue(report['linked'], report.get('error'))
                self.assertEqual(report['startup']['file'], executable.OVERLAY_STARTUP)
                self.assertEqual(
                    (output / executable.OVERLAY_STARTUP).read_bytes(),
                    Path(report['startup']['path']).read_bytes(),
                )
                self.assertEqual(report['retail_payload_inputs'], [])
                actual = (output / image).read_bytes()
                base = struct.unpack_from('<I', actual, 0x18)[0]
                entry, loads = cpe_loads((output / (image[:-4] + '.CPE')).read_bytes())
                sections = {name: (int(start, 16), int(size, 16)) for start, size, name in re.findall(
                    r'(?m)^ ([0-9A-F]{8}) [0-9A-F]{8} ([0-9A-F]{8})  (\.[a-z]+)',
                    (output / (image[:-4] + '.MAP')).read_text())}
                offset = 2048 + entry - base
                self.assertEqual(actual[offset - 8:offset], bytes.fromhex('0800e00300000000'))
                high, low, jump, delay = struct.unpack_from('<4I', actual, offset)
                self.assertEqual(high & 0xffff0000, 0x3c1c0000)
                self.assertEqual(low & 0xffff0000, 0x279c0000)
                signed_low = (low & 0xffff) - (0x10000 if low & 0x8000 else 0)
                # PSYLINK omits empty sections from MAP. With these BIOS-only
                # controls the empty .sdata anchor follows initialized data.
                gp_base = sections.get('.sdata', (sum(sections['.data']), 0))[0]
                self.assertEqual(((high & 0xffff) << 16) + signed_low, gp_base)
                self.assertEqual(jump >> 26, 2)
                self.assertEqual(((jump & 0x3ffffff) << 2) | (entry & 0xf0000000), sections['.text'][0])
                self.assertEqual(delay, 0)
                self.assertGreaterEqual(sections['.bss'][1], 8192)
                self.assertTrue(all(address + len(data) <= sections['.bss'][0]
                                    for address, data in loads))
                self.assertLess(len(actual), 8192)
                data_base, data_size = sections['.data']
                self.assertGreaterEqual(data_size, 4)
                self.assertTrue(any(address <= data_base and address + len(data) >= data_base + 4
                                    for address, data in loads))
                if image == 'GAME.EXE':
                    for selector in (0x4a, 0x4b, 0x4c, 0x45):
                        self.assertIn(struct.pack('<3I', 0x240a00b0, 0x01400008,
                                                  0x24090000 | selector), actual[2048:])

    def test_overlay_startup_uses_first_linked_small_data_address(self):
        for image, origin in (('OPEN.EXE', 0x80012000), ('GAME.EXE', 0x80017ff0)):
            with self.subTest(image=image), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                manifest = self.manifest(root, (
                    'int first = 0x12345678; extern int helper(void); '
                    'int main(void) { return first + helper(); }\n',
                    'int second = 0x76543210; unsigned char large[12] = {3}; '
                    'int helper(void) { return second + large[0]; }\n',
                ), image)
                manifest = replace(manifest, profiles={
                    key: replace(profile, small_data=8)
                    for key, profile in manifest.profiles.items()})
                layout = replace(executable.IMAGE_LAYOUTS[image], load_address=origin)
                output = root / 'output'
                with (mock.patch.dict(executable.IMAGE_LAYOUTS, {image: layout}),
                      mock.patch.object(RetailImage, 'load',
                                        side_effect=AssertionError('retail input used'))):
                    report = build_image(image, manifest, output)
                self.assertTrue(report['linked'], report.get('error'))
                for unit in report['units']:
                    self.assertRegex((output / unit['assembly']).read_text(), r'(?m)^\s*\.sdata\b')
                commands = (output / 'LINK.LNK').read_text()
                self.assertGreater(commands.index('NONE2.OBJ'), commands.index('U0001.OBJ'))
                startup = subprocess.run(
                    ['psyk', 'list', '--code', str(output / 'NONE2.OBJ')],
                    capture_output=True,
                    text=True,
                    check=True,
                ).stdout
                self.assertIn('sectstart', startup)
                self.assertIn("'__SN_ENTRY_POINT'", startup)
                stem = image[:-4]
                mapping = (output / (stem + '.MAP')).read_text()
                match = re.search(r'(?m)^ ([0-9A-F]{8}) [0-9A-F]{8} ([0-9A-F]{8})  \.sdata',
                                  mapping)
                self.assertIsNotNone(match)
                gp_base, extent = (int(value, 16) for value in match.groups())
                self.assertEqual(extent, 8)
                entry, loads = cpe_loads((output / (stem + '.CPE')).read_bytes())
                actual = (output / image).read_bytes()
                self.assertEqual(actual[2048 + gp_base - origin:2048 + gp_base - origin + 8],
                                 struct.pack('<2I', 0x12345678, 0x76543210))
                high, low, jump, delay = struct.unpack_from('<4I', actual, 2048 + entry - origin)
                self.assertEqual(high & 0xffff0000, 0x3c1c0000)
                self.assertEqual(low & 0xffff0000, 0x279c0000)
                signed_low = (low & 0xffff) - (0x10000 if low & 0x8000 else 0)
                self.assertEqual(((high & 0xffff) << 16) + signed_low, gp_base)
                if image == 'GAME.EXE':
                    self.assertLess(signed_low, 0)
                self.assertEqual(jump >> 26, 2)
                self.assertEqual(delay, 0)
                for address, payload in loads:
                    offset = 2048 + address - origin
                    self.assertEqual(actual[offset:offset + len(payload)], payload)

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
