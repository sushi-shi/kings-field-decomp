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
                         "-I", str(REPO / "include"), "-isystem", sdk, str(source)],
                        capture_output=True, text=True,
                    )
                    if name == "valid":
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0, name)
