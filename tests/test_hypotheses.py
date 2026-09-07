from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.kf.hypotheses import Axis, Edit, Option, parse_manifest, render, variants


class HypothesisTests(unittest.TestCase):
    def test_axes_form_deduplicated_cartesian_product(self) -> None:
        original = b"left middle right\n"
        axes = (
            Axis("left", (Option("base", (Edit(0, 4, b"left"),)),
                          Option("upper", (Edit(0, 4, b"LEFT"),)))),
            Axis("right", (Option("base", (Edit(12, 17, b"right"),)),
                           Option("upper", (Edit(12, 17, b"RIGHT"),)))),
        )
        result = variants(original, axes)
        self.assertEqual(len(result), 4)
        self.assertEqual(result[-1].source, b"LEFT middle RIGHT\n")

    def test_render_applies_offsets_against_original(self) -> None:
        self.assertEqual(
            render(b"abc def ghi", (Edit(0, 3, b"A"), Edit(8, 11, b"G"))),
            b"A def G",
        )

    def test_manifest_rejects_non_unique_span(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.c"
            source.write_text("same same\n")
            manifest = root / "manifest.json"
            manifest.write_text(json.dumps({
                "schema": 1, "unit": "open.sample", "function": "sample",
                "axes": [{"name": "x", "find": "same", "options": [{"name": "base"}]}],
            }))
            unit = mock.Mock(source="sample.c", functions=(mock.Mock(symbol="sample"),))
            loaded = mock.Mock()
            loaded.by_name.return_value = {"open.sample": unit}
            with mock.patch("scripts.kf.hypotheses.load_manifest", return_value=loaded):
                with self.assertRaisesRegex(ValueError, "occurs 2 times"):
                    parse_manifest(manifest, root=root)


if __name__ == "__main__":
    unittest.main()
