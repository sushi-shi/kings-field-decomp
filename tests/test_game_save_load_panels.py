"""Complete raw-word controls for the GAME save/load slot panels."""

from __future__ import annotations

import os
import shutil
import struct
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.sema.image import RetailImage
from tests.test_open_runtime_owner_probe import linked_words


class GameSaveLoadPanelTests(unittest.TestCase):
    def test_complete_panel_words_calls_and_exit_paths(self):
        try:
            image = RetailImage.load("GAME.EXE")
        except (OSError, ValueError):
            self.skipTest("hash-checked local GAME.EXE required")
        if not shutil.which("cc1psx-257") or "PSYQ_INCLUDE" not in os.environ:
            self.skipTest("pinned compiler and SDK headers required")
        manifest = load_manifest()
        unit = manifest.by_name()["game.menu_runtime"]
        profile = manifest.profiles[unit.profile]
        target_path = BUILD / "delink/game/modules" / unit.object_name
        if not target_path.is_file():
            self.skipTest("delinked menu unit required")
        target_obj = _load_object(target_path)
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["GAME.EXE"]}
        controls = {
            "menu_save_panel": (1128, 58, {
                0x244: 0x1622001D,  # explicit format-row guard
                0x344: 0x1642003E,  # exit to final result move
                0x348: 0x00009821,  # confirmation reset on both paths
                0x440: 0x02401021,  # return dialog result
            }),
            "menu_load_panel": (880, 42, {
                0x214: 0x1642004B,  # exit to final result move
                0x218: 0x00009821,  # confirmation reset on both paths
                0x2C4: 0x14400005,  # empty-slot arm precedes confirmation
                0x344: 0x02401021,  # return dialog result
            }),
        }
        self.assertTrue(controls.keys() <= {claim.symbol for claim in unit.functions})
        with TemporaryDirectory(prefix="kf-save-load-panels-") as directory:
            output = Path(directory) / unit.object_name
            compile_source(
                unit.source_path, unit.image, output, BUILD / "delink",
                profile.optimization, profile.small_data, profile.aspsx_version,
                (REPO / "include", REPO / "vendor/include", Path(os.environ["PSYQ_INCLUDE"])),
                profile.cc1_flags, profile.compiler,
                defines=unit.defines,
            )
            obj = _load_object(output)
            for claim in unit.functions:
                if claim.symbol not in controls:
                    continue
                with self.subTest(function=claim.symbol):
                    size, call_count, slots = controls[claim.symbol]
                    self.assertEqual(claim.body_size, size)
                    expected = list(struct.unpack(
                        f"<{size // 4}I", image.require(claim.va, size)))
                    target, calls, addresses = linked_words(
                        target_obj, unit, claim, {}, functions)
                    self.assertEqual(target, expected)
                    self.assertEqual(len(calls), call_count)
                    self.assertEqual(addresses, [])
                    actual = linked_words(obj, unit, claim, {}, functions)
                    self.assertEqual(actual, (expected, calls, []))
                    self.assertEqual(expected[0], 0x27BDFF88)  # 120-byte frame
                    self.assertEqual(expected[-2:], [0x03E00008, 0x27BD0078])
                    for offset, word in slots.items():
                        self.assertEqual(expected[offset // 4], word)
                        corrupted = list(actual[0])
                        corrupted[offset // 4] = 0
                        self.assertNotEqual(corrupted, expected)
                    wrong = dict(functions)
                    wrong["save_system_read_catalog"] += 4
                    self.assertNotEqual(
                        linked_words(obj, unit, claim, {}, wrong)[0], expected)
