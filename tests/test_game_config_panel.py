"""Static ABI, input-call, and reviewed-jump controls for the config panel."""

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
from scripts.kf.relocations import decode_mips26_target
from scripts.kf.retail import read_tsv
from scripts.kf.sema.image import RetailImage
from tests.test_open_runtime_owner_probe import linked_words


class GameConfigPanelTests(unittest.TestCase):
    def retail(self):
        try:
            return RetailImage.load("GAME.EXE")
        except (OSError, ValueError):
            self.skipTest("hash-checked local GAME.EXE required")

    def test_reviewed_jumps_keep_numeric_targets_and_delay_slots(self):
        image = self.retail()
        rows = {int(row["site_va"], 16): row for row in
                read_tsv(RETAIL_CONFIG / "relocs.tsv")[1]
                if row["image"] == "GAME.EXE" and row["kind"] == "mips26"}
        expected = (
            (0x80025BA4, 0x80025C88, 0x34110004),
            (0x80025BD4, 0x80025C88, 0x00008821),
            (0x80025C18, 0x80025C50, 0x00111880),
            (0x80025C48, 0x80025C84, 0x34140001),
            (0x80025C60, 0x80025C88, 0xAC620000),
            (0x80025D0C, 0x800259F0, 0),
            (0x80025D64, 0x80025D74, 0),
        )
        for site, target, slot in expected:
            with self.subTest(site=hex(site)):
                instruction, delay = struct.unpack("<II", image.require(site, 8))
                self.assertEqual(instruction >> 26, 2)
                self.assertEqual(decode_mips26_target(site, instruction), target)
                self.assertEqual(delay, slot)
                self.assertEqual(int(rows[site]["target_va"], 16), target)
                self.assertEqual(rows[site]["status"], "reviewed")
                self.assertEqual(rows[site]["channel"], "reachable-code")

    def test_label_abi_and_shared_exit_keep_both_functions_exact(self):
        image = self.retail()
        if not shutil.which("cc1psx-257") or "PSYQ_INCLUDE" not in os.environ:
            self.skipTest("pinned compiler and SDK headers required")
        manifest = load_manifest()
        unit = manifest.by_name()["game.menu_config_panel"]
        profile = manifest.profiles[unit.profile]
        target_path = BUILD / "delink/game/modules" / unit.object_name
        if not target_path.is_file():
            self.skipTest("delinked config panel required")
        data = {item.name: item.va for (key, _), item in
                load_data_identities(RETAIL_CONFIG).items() if key == "GAME.EXE"}
        functions = {item.symbol: item.va for item in
                     load_catalog(RETAIL_CONFIG).functions["GAME.EXE"]}
        target_obj = _load_object(target_path)
        with TemporaryDirectory(prefix="kf-config-panel-") as directory:
            output = Path(directory) / unit.object_name
            compile_source(
                unit.source_path, unit.image, output, BUILD / "delink",
                profile.optimization, profile.small_data, profile.aspsx_version,
                (REPO / "include", Path(os.environ["PSYQ_INCLUDE"])),
                profile.cc1_flags, profile.compiler, profile.maspsx_flags,
                defines=unit.defines,
            )
            candidate = _load_object(output)
            for claim in unit.functions:
                with self.subTest(function=claim.symbol):
                    actual, calls, addresses = linked_words(
                        candidate, unit, claim, data, functions)
                    target, target_calls, target_addresses = linked_words(
                        target_obj, unit, claim, data, functions)
                    retail = list(struct.unpack(
                        f"<{claim.body_size // 4}I", image.require(claim.va, claim.body_size)))
                    self.assertEqual(target, retail)
                    self.assertEqual(actual, retail)
                    self.assertEqual(calls, target_calls)
                    self.assertEqual(addresses, target_addresses)
                    if claim.symbol == "menu_config_panel_draw":
                        wrong = dict(functions)
                        wrong["menu_draw_string"] += 4
                        corrupted, _, _ = linked_words(candidate, unit, claim, data, wrong)
                        self.assertNotEqual(corrupted, retail)
                    else:
                        # These direct arguments are in call delay slots on both sides.
                        for name, arguments in (
                            ("PadRead", [0x34040001] * 3),
                            ("menu_play_input_sound",
                             [0x00002021, 0x00002021, 0x34040001, 0x34040001, 0x34040002]),
                        ):
                            for words in (actual, retail):
                                slots = [words[index + 1] for index, word in enumerate(words)
                                         if word >> 26 == 3 and decode_mips26_target(
                                             claim.va + 4 * index, word) == functions[name]]
                                self.assertEqual(slots, arguments)
