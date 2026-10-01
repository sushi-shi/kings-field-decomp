"""Exercise resource decoding with checked C++ readers."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ResourceCodecs(unittest.TestCase):
    def test_resource_boundaries(self):
        with tempfile.TemporaryDirectory(prefix="kf-resource-codecs-") as temporary:
            directory = Path(temporary)
            binary = directory / "test"
            subprocess.run([
                "clang++", "-std=c++20", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                "-fsanitize=address,undefined", "-I", str(ROOT / "include"),
                str(ROOT / "tests/resource_codecs.cpp"),
                *(str(ROOT / source) for source in ("src/audio/codec.cpp", "src/renderer/tim.cpp",
                                                   "src/lib/resource_decode.cpp")), "-o", str(binary),
            ], check=True)
            result = subprocess.run([binary], check=True, timeout=20, capture_output=True, text=True)
            self.assertRegex(result.stderr,
                             r"kf-codec: [^\n]*resource_decode\.cpp:\d+:\d+: truncated input at \d+: need \d+ bytes, have \d+")
            self.assertNotRegex(result.stderr, r"kf-codec: [^\n]*byte_reader\.h:")
            self.assertNotIn("end of input", result.stderr)
