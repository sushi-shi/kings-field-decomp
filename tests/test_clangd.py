from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from scripts.kf.clangd import commands, generate
from scripts.kf.manifest import Manifest, Profile, Unit


def manifest() -> Manifest:
    return Manifest({
        "c": Profile("c", "c", "gcc257-native", "O2", 0, "1.07",
                     ("-mcpu=r2000", "-fno-schedule-insns")),
        "asm": Profile("asm", "assembly", "gnu-as", None, 0, "1.07", ()),
    }, (
        Unit("game.shared", "GAME.EXE", "src/shared.c", "c", ()),
        Unit("open.shared", "OPEN.EXE", "src/shared.c", "c", (), defines=("KF_OPEN",)),
        Unit("open.only", "OPEN.EXE", "src/open/only.c", "c", (), defines=("OPEN_ONLY=1",)),
        Unit("psx.start", "PSX.EXE", "src/psx/start.s", "asm", ()),
    ))


class ClangdTests(unittest.TestCase):
    def test_context_selects_shared_variant_and_preserves_other_images(self) -> None:
        for image in ("game", "open"):
            entries = commands(manifest(), Path("/checkout"), "/toolchain/clang", Path("/sdk"), image)
            by_file = {entry["file"]: entry for entry in entries}
            self.assertEqual(len(entries), 2)
            shared = by_file["/checkout/src/shared.c"]["arguments"]
            self.assertEqual("-DKF_OPEN" in shared, image == "open")
            self.assertIn("-DOPEN_ONLY=1", by_file["/checkout/src/open/only.c"]["arguments"])

    def test_editor_commands_keep_paths_intact_and_omit_codegen_flags(self) -> None:
        entry = commands(manifest(), Path("/checkout with spaces"), "/tools with spaces/clang",
                         Path("/sdk with spaces"), "game")[0]
        arguments = entry["arguments"]
        self.assertEqual(arguments[0], "/tools with spaces/clang")
        self.assertEqual(arguments[arguments.index("-I") + 1], "/checkout with spaces/include")
        self.assertEqual(arguments[arguments.index("-isystem") + 1], "/sdk with spaces")
        self.assertIn("--target=mipsel-none-elf", arguments)
        self.assertIn("-mabi=32", arguments)
        self.assertIn("-std=gnu89", arguments)
        self.assertNotIn("-mcpu=r2000", arguments)
        self.assertNotIn("-fno-schedule-insns", arguments)

    def test_refresh_remembers_context_tracks_environment_and_recreates_database(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            sdk = repo / "sdk"
            sdk.mkdir()
            with (mock.patch.dict("os.environ", {"PSYQ_INCLUDE": str(sdk)}),
                  mock.patch("scripts.kf.clangd.shutil.which", return_value="/tools/clang")):
                self.assertEqual(generate(manifest(), repo=repo), (2, "game"))
                generate(manifest(), image="open", repo=repo)
                path = repo / "compile_commands.json"
                modified = path.stat().st_mtime_ns
                self.assertEqual(generate(manifest(), repo=repo), (2, "open"))
                self.assertEqual(path.stat().st_mtime_ns, modified)
                path.unlink()
                generate(manifest(), repo=repo)
                entries = json.loads(path.read_text())
                self.assertIn("-DKF_OPEN", next(e["arguments"] for e in entries
                                               if e["file"].endswith("/shared.c")))
                with mock.patch("scripts.kf.clangd.shutil.which", return_value="/new/clang"):
                    generate(manifest(), repo=repo)
                self.assertTrue(all(e["arguments"][0] == "/new/clang"
                                    for e in json.loads(path.read_text())))

    def test_invalid_context_does_not_overwrite_database(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            path = repo / "compile_commands.json"
            path.write_text("existing")
            with self.assertRaisesRegex(ValueError, "invalid clangd image"):
                generate(manifest(), image="unknown", repo=repo)
            self.assertEqual(path.read_text(), "existing")
