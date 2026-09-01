from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.kf.delink import Catalog, Function
from scripts.kf.manifest import _bind_claims
from scripts.kf.model import Claim, scan_claims


def _function(va: int, size: int, name: str) -> Function:
    return Function("GAME.EXE", va, size, size, 1, name, "test", "test")


def _catalog(*functions: Function) -> Catalog:
    return Catalog(
        functions={"GAME.EXE": tuple(sorted(functions, key=lambda f: f.va))},
        function_starts={"GAME.EXE": {f.va: f for f in functions}},
        data={"GAME.EXE": ()},
    )


class ClaimScanTests(unittest.TestCase):
    def _scan(self, text: str) -> tuple[Claim, ...]:
        with tempfile.TemporaryDirectory(prefix="kf-model-") as directory:
            source = Path(directory) / "unit.c"
            source.write_text(text, encoding="utf-8")
            return scan_claims(source)

    def test_claim_binds_the_definition_that_follows(self) -> None:
        claims = self._scan(
            "#include <kf/address.h>\n\n"
            "ADDRESS(0x80010000)\n"
            "void first(void)\n{\n}\n\n"
            "ADDRESS(0x80010010) /* second */\n"
            "struct KfActor *second(\n    int argument)\n{\n    return 0;\n}\n"
        )
        self.assertEqual(
            claims,
            (Claim(0x80010000, "first", 3), Claim(0x80010010, "second", 8)),
        )

    def test_claim_without_definition_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            self._scan("ADDRESS(0x80010000)\nextern int x;\n")


class ClaimBindingTests(unittest.TestCase):
    def setUp(self) -> None:
        self.first = _function(0x80010000, 0x10, "first")
        self.second = _function(0x80010010, 0x20, "second")
        self.third = _function(0x80010030, 0x10, "third")
        self.catalog = _catalog(self.first, self.second, self.third)
        self.identities = {("GAME.EXE", 0x80010010): "second"}

    def _bind(self, claims: tuple[Claim, ...], claimed: dict | None = None):
        return _bind_claims(
            Path("units.toml"), "game.unit", "GAME.EXE", Path("src/game/unit.c"),
            claims, self.catalog, self.identities, claimed if claimed is not None else {},
        )

    def test_contiguous_ascending_run_binds(self) -> None:
        functions = self._bind((Claim(0x80010000, "first", 1), Claim(0x80010010, "second", 5)))
        self.assertEqual([f.va for f in functions], [0x80010000, 0x80010010])

    def test_descending_claims_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "does not ascend"):
            self._bind((Claim(0x80010010, "second", 1), Claim(0x80010000, "first", 5)))

    def test_identity_name_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "identity"):
            self._bind((Claim(0x80010010, "func_80010010", 1),))

    def test_unclaimed_interior_function_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "without claiming"):
            self._bind((Claim(0x80010000, "first", 1), Claim(0x80010030, "third", 9)))

    def test_double_claim_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "already claimed"):
            self._bind((Claim(0x80010000, "first", 1),), {("GAME.EXE", 0x80010000): "other"})


if __name__ == "__main__":
    unittest.main()
