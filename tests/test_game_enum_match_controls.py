"""Retail-word controls for the enum conversion corrections in GAME."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.compile import compile_source
from scripts.kf.delink import load_catalog
from scripts.kf.hypotheses import strict_score
from scripts.kf.inventory import load_data_identities
from scripts.kf.manifest import load
from scripts.kf.parser_machine import _load_object
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.sema.image import RetailImage
from tests.test_open_runtime_owner_probe import linked_words


class GameEnumMatchControls(unittest.TestCase):
    def test_complete_bodies_and_ordered_referents(self):
        try:
            image = RetailImage.load("GAME.EXE")
        except (OSError, ValueError):
            self.skipTest("hash-checked local GAME.EXE required")
        if not shutil.which("cc1psx-257") or "PSYQ_INCLUDE" not in os.environ:
            self.skipTest("pinned compiler and SDK headers required")
        manifest = load()
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["GAME.EXE"]}
        data = {item.name: item.va for (owner, _), item in
                load_data_identities(RETAIL_CONFIG).items() if owner == "GAME.EXE"}
        controls = {
            "game.player_core": {"player_equip_weapon": (244, 24, 4, 12)},
            "game.actor_behavior": {"actor_spawn_action_effect": (1108, 200, 11, 3)},
        }
        with TemporaryDirectory(prefix="kf-enum-match-") as directory:
            for name, selected in controls.items():
                unit = manifest.by_name()[name]
                target = BUILD / "delink/game/modules" / unit.object_name
                if not target.is_file():
                    self.skipTest(f"delinked {name} required")
                profile = manifest.profiles[unit.profile]
                output = Path(directory) / unit.object_name
                compile_source(
                    unit.source_path, unit.image, output, BUILD / "delink",
                    profile.optimization, profile.small_data, profile.aspsx_version,
                    (REPO / "include", REPO / "vendor/include", Path(os.environ["PSYQ_INCLUDE"])),
                    profile.cc1_flags, profile.compiler,
                    defines=unit.defines,
                )
                actual, expected = _load_object(output), _load_object(target)
                for claim in unit.functions:
                    if claim.symbol not in selected:
                        continue
                    with self.subTest(function=claim.symbol):
                        size, frame, calls, addresses = selected[claim.symbol]
                        self.assertEqual(strict_score(target, output, claim.symbol)[:2],
                                         (100.0, size))
                        retail = list(struct.unpack(f"<{size // 4}I",
                                                    image.require(claim.va, size)))
                        reference = linked_words(expected, unit, claim, data, functions)
                        result = linked_words(actual, unit, claim, data, functions)
                        self.assertEqual(reference[0], retail)
                        self.assertEqual(result, reference)
                        self.assertEqual((len(result[1]), len(result[2])),
                                         (calls, addresses))
                        self.assertEqual(retail[0], 0x27BD0000 | (-frame & 65535))
                        self.assertEqual(retail[-2:], [0x03E00008, 0x27BD0000 | frame])
                        # Changing a real referent must fail even when the
                        # instruction and relocation kinds are unchanged.
                        wrong = dict(functions)
                        callee = next(key for key, va in functions.items()
                                      if va == result[1][0])
                        wrong[callee] += 4
                        self.assertNotEqual(
                            linked_words(actual, unit, claim, data, wrong)[0], retail)


if __name__ == "__main__":
    unittest.main()
