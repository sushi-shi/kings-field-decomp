"""Check that the compatibility layer enables Clang's enum constraints."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.clangd import FLAGS, MODES
from scripts.kf.paths import REPO


SOURCE = """
#include <kf/game_actor.h>
void control(KfActor *actor) { BODY }
"""


class EnumTypeTests(unittest.TestCase):
    def test_menu_selection_accepts_controls_and_spells_but_rejects_other_domains(self) -> None:
        sdk = os.environ.get("PSYQ_INCLUDE")
        compiler = shutil.which("clang")
        if not sdk or not compiler:
            self.skipTest("Clang and pinned SDK headers are required")
        source = """
            #include <kf/game_menu.h>
            void control(void) { BODY }
        """
        cases = {
            "valid": """
                KfMagicPanelResult selection = KF_MENU_RESULT_PENDING;
                selection = KF_MAGIC_HEALING;
                if (selection == KF_MAGIC_HEALING)
                    selection = KF_MENU_RESULT_CANCELLED;
                s32 encoded = KF_ENUM_ENCODE(s32, selection);
                static_assert(KF_ENUM_ENCODE(s32,
                    KfMagicPanelResult(KF_MENU_RESULT_PENDING)) == -99);
                static_assert(KF_ENUM_ENCODE(s32,
                    KfMagicPanelResult(KF_MAGIC_BLESS)) == 3);
            """,
            "wrong_payload": "KfMagicPanelResult selection = KF_ITEM_MEDICINAL_HERB;",
            "raw_integer": "KfMagicPanelResult selection = -99;",
            "control_as_spell": "KfEffectKind spell = KF_MENU_RESULT_PENDING;",
        }
        with TemporaryDirectory(prefix="kf-menu-selection-") as directory:
            path = Path(directory) / "control.c"
            for name, body in cases.items():
                with self.subTest(control=name):
                    path.write_text(source.replace("BODY", body))
                    result = subprocess.run(
                        [compiler, *FLAGS, *MODES["modern"], "-fsyntax-only",
                         "-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"),
                         "-isystem", sdk, str(path)],
                        capture_output=True, text=True,
                    )
                    if name == "valid":
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0, name)

    def test_promoted_arguments_keep_distinct_enum_domains(self) -> None:
        sdk = os.environ.get("PSYQ_INCLUDE")
        compiler = shutil.which("clang")
        if not sdk or not compiler:
            self.skipTest("Clang and pinned SDK headers are required")
        source = """
            #include <kf/game_save.h>
            #include <kf/game_effect.h>
            void control(int value) { BODY }
        """
        cases = {
            "valid": """
                save_system_read_slot(KF_ENUM_DECODE(KfSaveSlotArgument, value + 1));
                effect_pool_construct(0, KF_EFFECT_TYPE_NONE,
                    KF_ENUM_DECODE(KfEffectKindArgument, value & 31), 0, 0);
            """,
            "wrong_slot_domain": """
                save_system_read_slot(KF_ENUM_DECODE(KfEffectKindArgument, value));
            """,
            "wrong_kind_domain": """
                effect_pool_construct(0, KF_EFFECT_TYPE_NONE,
                    KF_ENUM_DECODE(KfSaveSlotArgument, value), 0, 0);
            """,
        }
        with TemporaryDirectory(prefix="kf-promoted-enums-") as directory:
            path = Path(directory) / "control.c"
            for name, body in cases.items():
                with self.subTest(control=name):
                    path.write_text(source.replace("BODY", body))
                    result = subprocess.run(
                        [compiler, *FLAGS, *MODES["modern"], "-fsyntax-only",
                         "-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"), "-isystem", sdk, str(path)],
                        capture_output=True, text=True,
                    )
                    if name == "valid":
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0, name)

    def test_modern_enum_constraints_are_enabled(self) -> None:
        sdk = os.environ.get("PSYQ_INCLUDE")
        if not sdk or not Path(sdk).is_dir():
            self.skipTest("pinned SDK headers are required")
        compiler = shutil.which("clang")
        if not compiler:
            self.skipTest("Clang is required")
        controls = {
            "valid": """
                actor_set_action(actor, KF_ACTOR_ACTION_JUMP_ATTACK);
                actor->lifecycle = KF_ENUM_DECODE(KfActorLifecycle,
                    KF_ENUM_ENCODE(u8, actor->lifecycle));
            """,
            "wrong_field_domain": "actor->action = KF_ACTOR_LIFECYCLE_ACTIVE;",
            "wrong_decode": "actor->action = KF_ENUM_DECODE(KfActorAction, actor->lifecycle);",
            "wrong_encode": "actor->action = KF_ENUM_ENCODE(KfActorAction, actor->lifecycle);",
        }
        with TemporaryDirectory(prefix="kf-enum-types-") as directory:
            source = Path(directory) / "control.c"
            for name, body in controls.items():
                with self.subTest(control=name):
                    source.write_text(SOURCE.replace("BODY", body))
                    result = subprocess.run(
                        [compiler, *FLAGS, *MODES["modern"], "-fsyntax-only",
                         "-I", str(REPO / "include"), "-I", str(REPO / "vendor/include"), "-isystem", sdk, str(source)],
                        capture_output=True, text=True,
                    )
                    if name == "valid":
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0, name)
