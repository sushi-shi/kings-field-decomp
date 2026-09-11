"""Complete raw-instruction control for the hub's corrected state/exit flow."""

from __future__ import annotations

import os
import shutil
import struct
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load as load_manifest
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.sema.image import RetailImage
from tests.test_open_runtime_owner_probe import linked_words


class GameSaveLoadHubTests(unittest.TestCase):
    def test_complete_hub_words_referents_and_state_update_slots(self):
        try:
            image = RetailImage.load("GAME.EXE")
        except (OSError, ValueError):
            self.skipTest("hash-checked local GAME.EXE required")
        if not shutil.which("cc1psx-257") or "PSYQ_INCLUDE" not in os.environ:
            self.skipTest("pinned compiler and SDK headers required")
        manifest = load_manifest()
        unit = manifest.by_name()["game.menu_runtime"]
        claim = next(fn for fn in unit.functions if fn.symbol == "menu_save_load_hub")
        profile = manifest.profiles[unit.profile]
        target_path = BUILD / "delink/game/modules" / unit.object_name
        if not target_path.is_file():
            self.skipTest("delinked menu unit required")
        data = {item.name: item.va for (key, _), item in
                load_data_identities(RETAIL_CONFIG).items() if key == "GAME.EXE"}
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["GAME.EXE"]}
        expected = list(struct.unpack("<152I", image.require(claim.va, claim.body_size)))
        target, target_calls, target_addresses = linked_words(
            _load_object(target_path), unit, claim, data, functions)
        self.assertEqual(target, expected)
        self.assertEqual(len(target_calls), 20)
        self.assertEqual(target_addresses, [0x800668E0, 0x800668D8])
        with TemporaryDirectory(prefix="kf-save-load-hub-") as directory:
            output = Path(directory) / unit.object_name
            compile_source(
                unit.source_path, unit.image, output, BUILD / "delink",
                profile.optimization, profile.small_data, profile.aspsx_version,
                (REPO / "include", REPO / "vendor/include", Path(os.environ["PSYQ_INCLUDE"])),
                profile.cc1_flags, profile.compiler,
                defines=unit.defines,
            )
            obj = _load_object(output)
            actual, calls, addresses = linked_words(obj, unit, claim, data, functions)
            self.assertEqual(actual, expected)
            self.assertEqual(calls, target_calls)
            self.assertEqual(addresses, target_addresses)
            self.assertEqual(actual[0], 0x27BDD7D0)  # 10288-byte frame
            self.assertEqual(actual[0x144 // 4], 0x2413FFFF)  # action = -1
            self.assertEqual(actual[0x1DC // 4], 0x34150001)  # confirm = 1
            self.assertEqual(actual[0x234 // 4], 0x02201021)  # return result
            # Either lost state update must fail even with unchanged call targets.
            for offset in (0x144, 0x1DC):
                corrupted = list(actual)
                corrupted[offset // 4] = 0
                self.assertNotEqual(corrupted, expected)
            wrong = dict(functions)
            wrong["menu_load_panel"] += 4
            corrupted, _, _ = linked_words(obj, unit, claim, data, wrong)
            self.assertNotEqual(corrupted, expected)
