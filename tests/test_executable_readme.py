"""Executable README scores track outputs without overwriting function status."""

from dataclasses import replace
import hashlib
import json
from pathlib import Path
import tempfile
import shutil
import subprocess
import unittest
from unittest import mock

from scripts.kf import readme
from scripts.kf.data_match import ArtifactFailure, SectionDiff, UnitDataDiff
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
            self.assertIn('| `GAME.EXE` | — | — | — | — |', text)
            self.assertIn('## Retired executable experiment', text)
            self.assertIn('The three byte columns show similarity, not differences.', text)
            self.assertIn('`kf link --compare-only`', text)

    def test_data_counts_are_independent_of_executable_availability(self):
        text = readme.render_executable_block({}, {'game': (29, 41), 'open': (0, 20)})
        self.assertIn('| Data modules exact |', text)
        self.assertIn('| `PSX.EXE` | — | — | — | — |', text)
        self.assertIn('| `GAME.EXE` | — | — | — | 29 / 41 |', text)
        self.assertIn('| `OPEN.EXE` | — | — | — | 0 / 20 |', text)
        self.assertIn('relocations, BSS layout and retail placement', text)

    def test_data_count_requires_every_section_and_complete_artifacts(self):
        good = UnitDataDiff('GAME.EXE', 'game.good', [
            SectionDiff('.data', 8, 8, 'match'), SectionDiff('.bss', 4, 4, 'match')])
        bad = UnitDataDiff('GAME.EXE', 'game.bad', [
            SectionDiff('.data', 8, 8, 'match'), SectionDiff('.bss', 4, 4, 'placement')])
        artifact = ArtifactFailure('OPEN.EXE', 'open.missing', 'missing object')
        with (mock.patch('scripts.kf.manifest.load'),
              mock.patch.object(readme, '_data_inputs_current', return_value=True),
              mock.patch('scripts.kf.data_match.diff_image', side_effect=[
                  ([], []), ([good, bad], []), ([good], [artifact])])):
            self.assertEqual(readme.data_snapshot(), {'psx': (0, 0), 'game': (1, 2)})
        with (mock.patch('scripts.kf.manifest.load'),
              mock.patch.object(readme, '_data_inputs_current', return_value=False),
              mock.patch('scripts.kf.data_match.diff_image') as compare):
            self.assertEqual(readme.data_snapshot(), {})
            compare.assert_not_called()
        with (mock.patch('scripts.kf.manifest.load'),
              mock.patch.object(readme, '_data_inputs_current', return_value=True),
              mock.patch('scripts.kf.data_match.diff_image', side_effect=ValueError('bad ELF'))):
            self.assertEqual(readme.data_snapshot(), {})

    def test_stale_inputs_and_unchanged_object_rebuilds_use_ninja_state(self):
        if shutil.which('ninja') is None:
            self.skipTest('Ninja required')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'build').mkdir()
            graph = root / 'build/build.ninja'
            graph.write_text(
                'rule copy\n'
                '  command = cmp -s source result || cp source result\n'
                '  restat = 1\n'
                'build result: copy source | header inventory toolchain\n'
                'build base-game: phony result\n'
                'build target-game: phony result\n')
            for name in ('source', 'header', 'inventory', 'toolchain'):
                (root / name).write_text('initial')
            def build():
                subprocess.run(['ninja', '-f', str(graph), 'base-game'], cwd=root,
                               check=True, capture_output=True)
            with (mock.patch.object(readme, 'REPO', root),
                  mock.patch.object(readme, 'BUILD', root / 'build')):
                self.assertFalse(readme._data_inputs_current('GAME.EXE'))
                build()
                self.assertTrue(readme._data_inputs_current('GAME.EXE'))
                for name in ('header', 'inventory', 'toolchain', 'source'):
                    with self.subTest(input=name):
                        original_mtime = (root / 'result').stat().st_mtime_ns
                        (root / name).write_text('changed')
                        self.assertFalse(readme._data_inputs_current('GAME.EXE'))
                        build()
                        self.assertTrue(readme._data_inputs_current('GAME.EXE'))
                        if name != 'source':
                            self.assertEqual((root / 'result').stat().st_mtime_ns, original_mtime)
                (root / 'result').unlink()
                self.assertFalse(readme._data_inputs_current('GAME.EXE'))

    def test_function_refresh_also_updates_data_column(self):
        with tempfile.TemporaryDirectory() as directory:
            with (mock.patch.object(readme, 'BUILD', Path(directory)),
                  mock.patch('scripts.kf.progress.snapshot', return_value=({}, [])),
                  mock.patch.object(readme, 'render_block', return_value='functions'),
                  mock.patch.object(readme, 'write_block', return_value=False),
                  mock.patch.object(readme, '_write_executable', return_value=True) as refresh):
                self.assertTrue(readme.refresh())
                refresh.assert_called_once_with(Path(directory) / 'link', readme.README)


if __name__ == '__main__':
    unittest.main()
