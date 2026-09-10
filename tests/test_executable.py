"""Controls for comparison of retired executable experiments."""

from __future__ import annotations

import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from scripts.kf import executable
from scripts.kf.executable import compare
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

    def test_build_mode_refuses_the_retired_composed_toolchain(self):
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr), self.assertRaises(SystemExit) as error:
            executable.main([])
        self.assertEqual(error.exception.code, 2)
        self.assertIn("coherent Psy-Q Release 2.5 linking is unavailable", stderr.getvalue())

    def test_compare_only_preserves_executable_and_native_build_provenance(self):
        image = RetailImage.synthetic('GAME.EXE', 0x80012000, bytes(range(128)))
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            root = build / 'link/game'
            root.mkdir(parents=True)
            exe = root / 'GAME.EXE'
            exe.write_bytes(image.data)
            original_report = root / 'comparison.json'
            original_report.write_bytes(b'original native build provenance\n')
            with (mock.patch.object(executable, 'BUILD', build),
                  mock.patch.object(RetailImage, 'load', return_value=image),
                  mock.patch.object(executable, 'refresh_executable') as refresh,
                  contextlib.redirect_stdout(io.StringIO())):
                self.assertEqual(executable.main(['--image', 'game', '--compare-only']), 0)
                refresh.assert_called_once_with(build / 'link')
            self.assertEqual(exe.read_bytes(), image.data)
            self.assertEqual(original_report.read_bytes(), b'original native build provenance\n')
            report = json.loads((root / 'fuzzy-comparison.json').read_text())
            self.assertEqual(report['mode'], 'compare-existing-retired-experiment')
            self.assertTrue(report['comparison']['file_equal'])
            self.assertEqual(report['layout_tolerant']['byte_similarity_percent'], 100)
            self.assertEqual(report['layout_tolerant']['retail_bytes'], 128)
            html = (root / 'fuzzy-comparison.html').read_text()
            self.assertIn('Movement map', html)
            self.assertIn('80012000', html)
            self.assertIn('id="minimum"', html)


if __name__ == '__main__':
    unittest.main()
