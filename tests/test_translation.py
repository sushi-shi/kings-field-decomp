"""Translation format and launcher contract tests; no retail assets required."""

import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from scripts.english_patch import MAGIC, embed, replacement_spans


ROOT = Path(__file__).resolve().parents[1]


def record(name=b"KF/TEST.", size=8, spans=((1, b"XY"), (7, b"Z"))):
    return struct.pack("<III", len(name), size, len(spans)) + name + b"".join(
        struct.pack("<II", offset, len(data)) + data for offset, data in spans
    )


def delta(*records):
    return MAGIC + struct.pack("<I", len(records)) + b"".join(records)


class Translation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="kf-translation-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = Path(cls.temp.name)
        (cls.directory / "english_patch.inc").write_text(embed(b""))
        cls.binary = cls.directory / "test"
        subprocess.run([
            "clang++", "-std=c++20", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
            "-fsanitize=address,undefined", "-ffunction-sections", "-fdata-sections",
            "-Wl,--gc-sections", "-I", str(ROOT / "include"), "-I", str(cls.directory),
            str(ROOT / "tests/translation_regressions.cpp"),
            str(ROOT / "src/platform/assets.cpp"), str(ROOT / "src/platform/translation.cpp"),
            str(ROOT / "src/platform/language.cpp"), "-o", str(cls.binary),
        ], check=True, capture_output=True, text=True)

    def check_patch(self, payload, success=False):
        path = self.directory / "fixture.kfdelta"
        path.write_bytes(payload)
        result = subprocess.run([self.binary, path], capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0 if success else 1, result.stderr.decode())
        self.assertEqual(result.stderr, b"")
        if success:
            self.assertEqual(result.stdout, b"aXYdefgZ")

    def test_valid_sparse_replacements(self):
        self.check_patch(delta(record()), True)

    def test_every_truncation(self):
        payload = delta(record())
        for length in range(len(payload)):
            with self.subTest(length=length):
                self.check_patch(payload[:length])

    def test_invalid_paths_and_sizes(self):
        for item in [record(name=b"../TEST."), record(name=b"KF/MISSING."),
                     record(name=b"kf/test."), record(name=b"KF/TEST.\0"),
                     record(size=7), record(spans=((8, b"X"),)),
                     record(spans=((0xffffffff, b"X"),)),
                     record(spans=((1, b"XX"), (2, b"Y"))),
                     record(spans=((2, b"X"), (1, b"Y"))), record(spans=((0, b""),))]:
            with self.subTest(record=item):
                self.check_patch(delta(item))

    def test_version_duplicates_and_trailing_data(self):
        payload = delta(record())
        for invalid in [b"BAD!" + payload[4:], payload[:4] + b"\x02" + payload[5:],
                        delta(record(), record()), payload + b"x", delta()]:
            self.check_patch(invalid)

    def test_generator_reconstructs_changed_and_unchanged_bytes(self):
        for before, after in [(b"abcdefgh", b"aXYdefgZ"), (b"a" * 100, b"X" + b"a" * 98 + b"Z"),
                              (b"unchanged", b"unchanged"), (b"", b"")]:
            output = bytearray(before)
            for offset, data in replacement_spans(before, after):
                output[offset:offset + len(data)] = data
            self.assertEqual(output, after)


class Launcher(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="kf-launch-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.log = self.root / "calls"
        self.game = self.root / "game"
        self.game.write_text('''#!/usr/bin/env python3
import json, os, pathlib, sys
args = sys.argv[1:]
with open(os.environ['TEST_CALLS'], 'a') as log:
    log.write(json.dumps(args) + '\\n')
if '--extract-to' in args:
    if '--language' in args and args[args.index('--language') + 1] == os.environ.get('TEST_FAIL'):
        sys.exit(1)
    target = pathlib.Path(args[args.index('--extract-to') + 1])
    target.mkdir()
    (target / 'test').write_text('verified')
''')
        self.game.chmod(0o700)
        self.disc = self.root / "Japanese disc.iso"
        self.disc.touch()
        self.env = dict(os.environ, XDG_CACHE_HOME=str(self.root / "cache"), TEST_CALLS=str(self.log))
        for key in ["KF_DISC", "KF_LANGUAGE", "TEST_FAIL"]:
            self.env.pop(key, None)

    def launch(self, *args, success=True, **env):
        import json
        result = subprocess.run([
            "bash", "-euc", 'game_binary="$1"; launcher="$2"; shift 2; source "$launcher" "$@"',
            "launcher-test", str(self.game), str(ROOT / "scripts/launch.sh"), *args,
        ], env=dict(self.env, **env), capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0 if success else 1, result.stderr)
        return [json.loads(line) for line in self.log.read_text().splitlines()] if self.log.exists() else []

    def test_first_english_launch_imports_japanese_then_translates(self):
        calls = self.launch("--language", "en", KF_DISC=str(self.disc))
        self.assertEqual(len(calls), 3)
        self.assertEqual(calls[0][:4], ["--language", "ja", "--disc", str(self.disc)])
        self.assertEqual(calls[1][:3], ["--language", "en", "--data"])

    def test_switch_to_english_and_back_without_disc(self):
        self.launch(KF_DISC=str(self.disc))
        self.disc.unlink()
        self.log.unlink()
        calls = self.launch("--language", "en")
        self.assertEqual(len(calls), 2)
        self.assertNotIn("--disc", calls[0])
        self.log.unlink()
        self.assertEqual(len(self.launch("--language", "ja")), 1)
        self.log.unlink()
        self.assertEqual(len(self.launch("--language", "en")), 1)

    def test_failed_translation_preserves_japanese_and_does_not_publish(self):
        self.launch("--language", "en", success=False, KF_DISC=str(self.disc), TEST_FAIL="en")
        cache = self.root / "cache/kings-field/SLPS-00017"
        self.assertTrue((cache / "resources-v1/test").is_file())
        self.assertFalse((cache / "resources-en-v1").exists())
        self.launch("--language", "ja")

    def test_missing_disc_and_invalid_language(self):
        self.assertEqual(self.launch("--language", "en", success=False), [])
        self.assertEqual(self.launch("--language", "invalid", success=False), [])


if __name__ == "__main__":
    unittest.main()
