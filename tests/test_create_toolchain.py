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
    def test_one_source_medium_is_active(self) -> None:
        self.assertEqual(set(module.MEDIA), {"release-2.5"})

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

    def test_stage_release25_preserves_all_object_sets(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "source"
            stage = Path(directory) / "stage"
            for relative in (
                "isa board/PSXBIN/BIN",
                "isa board/PSXLIB/INCLUDE",
                "isa board/PSXLIB/LIB",
            ):
                (root / relative).mkdir(parents=True)
            h2000 = root / "H2000/LIB2000"
            h2000.mkdir(parents=True)
            for name in ("2MBYTE.OBJ", "8MBYTE.OBJ", "LIBAPI.LIB", "NONE2.OBJ"):
                (h2000 / name).write_bytes(b"LNK\x02" + name.encode())
            demo_card = root / "demo/CARD/CARD.OBJ"
            demo_card.parent.mkdir(parents=True)
            demo_card.write_bytes(b"LNK\x02demo-card")
            (root / "one-sdk-marker.txt").write_text("complete media tree")
            compiler = root / "compiler"
            compiler.mkdir()
            for name in (
                "CC1PSX", "CPPPSX", "CC1PSX.EXE", "CPPPSX.EXE",
                "COPYING.txt", "CPP.TEX", "EXTEND.TEX", "GCC.TEX",
                "INVOKE.TEX", "Install.txt",
            ):
                (compiler / name).write_bytes(name.encode())

            module.stage_release25(root, stage)

            for name in ("2MBYTE.OBJ", "8MBYTE.OBJ", "LIBAPI.LIB", "NONE2.OBJ"):
                self.assertEqual(
                    (stage / "release-2.5/H2000/LIB2000" / name).read_bytes(),
                    (h2000 / name).read_bytes(),
                )
            self.assertEqual(
                (stage / "release-2.5/demo/CARD/CARD.OBJ").read_bytes(),
                demo_card.read_bytes(),
            )
            self.assertEqual(
                (stage / "release-2.5/one-sdk-marker.txt").read_text(),
                "complete media tree",
            )
            self.assertFalse((stage / "compilers").exists())


if __name__ == "__main__":
    unittest.main()
