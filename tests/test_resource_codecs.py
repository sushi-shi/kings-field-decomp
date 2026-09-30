"""Exercise resource decoding through the generated C/Rust boundary."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ResourceCodecs(unittest.TestCase):
    def test_animation_and_placement_boundaries(self):
        with tempfile.TemporaryDirectory(prefix="kf-resource-codecs-") as temporary:
            directory = Path(temporary)
            subprocess.run([
                "cargo", "build", "--offline", "--locked", "--release",
                "--manifest-path", str(ROOT / "codecs/Cargo.toml"),
                "--target-dir", str(directory / "rust"),
            ], check=True)
            binary = directory / "test"
            subprocess.run([
                "clang++", "-std=c++20", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(ROOT / "include"),
                str(ROOT / "tests/resource_codecs.cpp"),
                str(directory / "rust/release/libkf_codec.a"), "-o", str(binary),
            ], check=True)
            result = subprocess.run([binary], check=True, timeout=20, capture_output=True, text=True)
            self.assertRegex(result.stdout,
                             r"kf-codec: [^\n]*resources\.rs:\d+:\d+: truncated resource input")
