from __future__ import annotations

import hashlib
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "create-toolchain.py"
SPEC = importlib.util.spec_from_file_location("create_toolchain", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class ToolchainTests(unittest.TestCase):
    def test_sha256_file(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.bin"
            path.write_bytes(b"King's Field\0")
            self.assertEqual(module.sha256_file(path), hashlib.sha256(path.read_bytes()).hexdigest())

    def test_hash_mismatch_is_fatal(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "medium.img"
            path.write_bytes(b"wrong")
            with self.assertRaisesRegex(RuntimeError, "hash mismatch"):
                module.verify_hash(path, "0" * 64, "test medium")

    def test_casefold_resolution(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target = root / "ISA BOARD" / "PsxLib" / "LIB"
            target.mkdir(parents=True)
            self.assertEqual(module.resolve_casefold(root, "isa board/PSXLIB/lib"), target)

    def test_manifest_is_sorted_and_reproducible(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            stage = root / "stage"
            (stage / "psyq" / "bin").mkdir(parents=True)
            (stage / "psyq" / "bin" / "PSYLINK.EXE").write_bytes(b"link")
            (stage / "psyq" / "bin" / "CCPSX.EXE").write_bytes(b"compile")
            module.write_manifest(stage)
            first = (stage / "MANIFEST.tsv").read_text()
            module.write_manifest(stage)
            second = (stage / "MANIFEST.tsv").read_text()
            self.assertEqual(first, second)
            self.assertLess(first.index("CCPSX.EXE"), first.index("PSYLINK.EXE"))


if __name__ == "__main__":
    unittest.main()
