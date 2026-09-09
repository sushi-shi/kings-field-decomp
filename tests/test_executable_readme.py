"""Executable README scores track outputs without overwriting function status."""

from dataclasses import replace
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from scripts.kf import readme
from scripts.kf.executable import compare
from scripts.kf.executable_diff import compare_executables
from scripts.kf.sema.image import RetailImage


class ExecutableReadmeTests(unittest.TestCase):
    def write_report(self, root, key, retail, candidate, filename='comparison.json'):
        directory = root / key
        directory.mkdir(parents=True, exist_ok=True)
        (directory / retail.image).write_bytes(candidate)
        report = {'image': retail.image, 'comparison': compare(candidate, retail),
                  'layout_tolerant': compare_executables(retail.data, candidate)}
        (directory / filename).write_text(json.dumps(report))
        return report

    def test_scores_require_matching_current_executable_and_retail_hashes(self):
        image = RetailImage.synthetic('GAME.EXE', 0x80012000, bytes(range(128)))
        # Synthetic layouts do not carry hashes; supply the verified identity
        # expected by the production report reader independently of its JSON.
        layout = replace(image.layout, sha256=hashlib.sha256(image.data).hexdigest())
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            report = self.write_report(root, 'game', image, image.data)
            with mock.patch.object(readme, 'IMAGE_LAYOUTS', {'GAME.EXE': layout}):
                self.assertEqual(set(readme.executable_snapshot(root)), {'game'})
                # A stale comparison-only report must not hide a valid native
                # build report for the current EXE.
                stale = json.loads(json.dumps(report))
                stale['layout_tolerant']['candidate_sha256'] = 'stale'
                (root / 'game/fuzzy-comparison.json').write_text(json.dumps(stale))
                self.assertEqual(set(readme.executable_snapshot(root)), {'game'})
                (root / 'game/GAME.EXE').write_bytes(image.data + b'changed')
                self.assertEqual(readme.executable_snapshot(root), {})
                (root / 'game/GAME.EXE').write_bytes(image.data)
                report['layout_tolerant']['equal_paired_bytes'] -= 1
                (root / 'game/comparison.json').write_text(json.dumps(report))
                self.assertEqual(readme.executable_snapshot(root), {})
                report['layout_tolerant']['equal_paired_bytes'] += 1
                report['layout_tolerant']['retail_sha256'] = 'wrong retail'
                (root / 'game/comparison.json').write_text(json.dumps(report))
                self.assertEqual(readme.executable_snapshot(root), {})
                (root / 'game/GAME.EXE').unlink()
                self.assertEqual(readme.executable_snapshot(root), {})

    def test_refresh_preserves_function_block_and_is_idempotent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / 'README.md'
            function_block = f'{readme.START}\nfunction status\n{readme.END}'
            path.write_text(f'# Project\n\n{function_block}\n\n## Usage\nmanual text\n')
            with mock.patch.object(readme, 'BUILD', root):
                self.assertTrue(readme.refresh_executable(root / 'link', path))
                self.assertFalse(readme.refresh_executable(root / 'link', path))
            text = path.read_text()
            self.assertIn(function_block, text)
            self.assertIn('## Usage\nmanual text\n', text)
            self.assertLess(text.index(readme.END), text.index(readme.EXECUTABLE_START))
            self.assertLess(text.index(readme.EXECUTABLE_END), text.index('## Usage'))
            executable_block = text[text.index(readme.EXECUTABLE_START):text.index(readme.EXECUTABLE_END)]
            readme.write_block(f'{readme.START}\nnew function status\n{readme.END}', path)
            self.assertIn(executable_block, path.read_text())

    def test_total_weights_bytes_and_labels_partial_image_availability(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            a = RetailImage.synthetic('PSX.EXE', 0x80010000, bytes(range(64)))
            b = RetailImage.synthetic('OPEN.EXE', 0x80012000, bytes(range(256)) * 4)
            candidate = RetailImage.synthetic('OPEN.EXE', 0x80012000, b'\xff' * 1024)
            first = self.write_report(root, 'psx', a, a.data)
            second = self.write_report(root, 'open', b, candidate.data)
            images = {'psx': {'strict': first['comparison'], 'fuzzy': first['layout_tolerant']},
                      'open': {'strict': second['comparison'], 'fuzzy': second['layout_tolerant']}}
            text = readme.render_executable_block(images)
            matched = sum(row['fuzzy']['equal_paired_bytes'] for row in images.values())
            size = sum(row['fuzzy']['retail_bytes'] + row['fuzzy']['candidate_bytes']
                       for row in images.values())
            self.assertIn(f'Overall (2/3 images): {200 * matched / size:.2f}%', text)
            self.assertIn('| `GAME.EXE` | — | — | — |', text)
            self.assertIn('All columns show similarity, not differences.', text)
            self.assertIn('`kf link --compare-only`', text)


if __name__ == '__main__':
    unittest.main()
