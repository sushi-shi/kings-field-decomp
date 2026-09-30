"""Exercise audio and TIM decoding through the generated C/Rust boundary."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class CodecRecords(unittest.TestCase):
    def test_codec_records(self):
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
                str(ROOT / "tests/codec_records.cpp"),
                str(directory / "rust/release/libkf_codec.a"), "-o", str(binary),
            ], check=True)
            result = subprocess.run([binary], check=True, timeout=20, capture_output=True, text=True)
            for module in ("audio", "tim"):
                self.assertRegex(result.stdout,
                                 rf"kf-codec: [^\n]*{module}\.rs:\d+:\d+: truncated input at \d+: need \d+ bytes, have \d+")
            self.assertNotRegex(result.stdout, r"kf-codec: [^\n]*(?:bytes|error)\.rs:")
            self.assertRegex(result.stdout,
                             r"kf-codec: [^\n]*tim\.rs:\d+:\d+: invalid rectangle -1x1 at 8")
            self.assertRegex(result.stdout,
                             r"kf-codec: [^\n]*ffi/mod\.rs:\d+:\d+: codec output is full")
            self.assertNotIn("end of input", result.stdout)
            self.assertRegex(result.stdout,
                             r"kf-codec: [^\n]*ffi/audio\.rs:\d+:\d+: unsupported SEQ channel message")
