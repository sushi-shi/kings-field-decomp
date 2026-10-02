"""Exercise audio and TIM decoding with checked C++ readers."""

from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class CodecRecords(unittest.TestCase):
    def test_codec_records(self):
        with tempfile.TemporaryDirectory(prefix="kf-resource-codecs-") as temporary:
            directory = Path(temporary)
            binary = directory / "test"
            subprocess.run([
                "clang++", "-std=c++20", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(ROOT / "include"),
                str(ROOT / "tests/codec_records.cpp"),
                *(str(ROOT / source) for source in ("src/audio/codec.cpp", "src/renderer/tim.cpp",
                                                   "src/lib/resource_decode.cpp")), "-o", str(binary),
            ], check=True)
            result = subprocess.run([binary], check=True, timeout=20, capture_output=True, text=True)
            for module in ("codec", "tim"):
                self.assertRegex(result.stderr,
                                 rf"kf-codec: [^\n]*{module}\.cpp:\d+:\d+: truncated input at \d+: need \d+ bytes, have \d+")
            self.assertNotRegex(result.stderr, r"kf-codec: [^\n]*byte_reader\.h:")
            self.assertRegex(result.stderr,
                             r"kf-codec: [^\n]*tim\.cpp:\d+:\d+: invalid TIM rectangle")
            self.assertRegex(result.stderr,
                             r"kf-codec: [^\n]*tim\.cpp:\d+:\d+: codec output is full")
            self.assertNotIn("end of input", result.stderr)
            self.assertRegex(result.stderr,
                             r"kf-codec: [^\n]*codec\.cpp:\d+:\d+: unsupported SEQ channel message")

    def test_recoverable_operation_boundaries(self):
        with tempfile.TemporaryDirectory(prefix="kf-codec-boundaries-") as temporary:
            binary = Path(temporary) / "test"
            sdl = shlex.split(subprocess.check_output(
                ["pkg-config", "--cflags", "--libs", "sdl3"], text=True))
            subprocess.run([
                "clang++", "-std=c++20", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",
                "-fsanitize=address,undefined", "-I", str(ROOT / "include"),
                str(ROOT / "tests/codec_boundaries.cpp"),
                str(ROOT / "src/audio/codec.cpp"), str(ROOT / "src/renderer/tim.cpp"),
                *sdl, "-o", str(binary),
            ], check=True)
            subprocess.run([binary], check=True, timeout=20, capture_output=True, text=True)
