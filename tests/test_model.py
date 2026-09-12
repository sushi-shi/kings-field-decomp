from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.kf.delink import Catalog, Function
from scripts.kf.manifest import _bind_claims, _bind_data_claims
from scripts.kf.model import (
    Claim,
    DataClaim,
    DataIdentity,
    scan_claims,
    scan_rodata_claims,
    scan_source,
    stale_address_names,
)


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
            "ADDRESS(0x80010000, 0x10)\n"
            "void first(void)\n{\n}\n\n"
            "ADDRESS(0x80010010, 32) /* second */\n"
            "struct KfActor *second(\n    int argument)\n{\n    return 0;\n}\n"
        )
        self.assertEqual(
            claims,
            (Claim(0x80010000, 0x10, "first", 3), Claim(0x80010010, 0x20, "second", 8)),
        )

    def test_parameter_type_macro_does_not_mask_function_name(self) -> None:
        claims = self._scan(
            "ADDRESS(0x80010000, 0x10)\n"
            "void talk_show_dialogue_page(\n"
            "    KF_ENUM_PARAM(KfFloorId, u8) floor, u8 page)\n"
            "{\n}\n"
        )
        self.assertEqual(claims, (Claim(0x80010000, 0x10, "talk_show_dialogue_page", 1),))

    def test_return_type_macro_does_not_mask_function_name(self) -> None:
        claims = self._scan(
            "ADDRESS(0x80010000, 0x10)\n"
            "KF_RESULT(KfStatus) read_status(void)\n"
            "{\n}\n"
        )
        self.assertEqual(claims, (Claim(0x80010000, 0x10, "read_status", 1),))

    def test_claim_without_definition_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            self._scan("ADDRESS(0x80010000, 0x10)\nextern int x;\n")

    def test_address_at_stacks_per_image_claims_on_one_definition(self) -> None:
        claims = self._scan(
            "#include <kf/address.h>\n\n"
            'ADDRESS_AT("GAME", 0x800202fc, 0x68)\n'
            'ADDRESS_AT("OPEN", 0x80019598, 0x68)\n'
            "void matrix_interpolate(void)\n{\n}\n"
        )
        self.assertEqual(
            claims,
            (
                Claim(0x800202FC, 0x68, "matrix_interpolate", 3, "GAME"),
                Claim(0x80019598, 0x68, "matrix_interpolate", 4, "OPEN"),
            ),
        )

    def test_plain_address_carries_no_image(self) -> None:
        (claim,) = self._scan("ADDRESS(0x80010000, 0x10)\nvoid first(void)\n{\n}\n")
        self.assertIsNone(claim.image)

    def _scan_data(self, text: str) -> tuple[DataClaim, ...]:
        with tempfile.TemporaryDirectory(prefix="kf-model-") as directory:
            source = Path(directory) / "unit.c"
            source.write_text(text, encoding="utf-8")
            return scan_source(source)[1]

    def test_data_claim_binds_the_declarator_that_follows(self) -> None:
        claims = self._scan_data(
            "DATA(0x80057b0c, 0x4)\n"
            "static u32 counter = 0;\n\n"
            "DATA(0x80057b10, 0x10)\n"
            "u32 table[4] = {\n    1, 2, 3, 4,\n};\n\n"
            "DATA(0x80057b20, 0x4)\n"
            "struct KfActor *current;\n\n"
            "DATA(0x80057b24, 0x4)\n"
            "void (*handler)(s32 argument);\n"
        )
        self.assertEqual(
            [(claim.va, claim.size, claim.name) for claim in claims],
            [
                (0x80057b0c, 4, "counter"),
                (0x80057b10, 0x10, "table"),
                (0x80057b20, 4, "current"),
                (0x80057b24, 4, "handler"),
            ],
        )

    def test_data_claim_before_extern_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "global definition"):
            self._scan_data("DATA(0x80057b0c, 0x4)\nextern u32 counter;\n")


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
        functions = self._bind((Claim(0x80010000, 0x10, "first", 1), Claim(0x80010010, 0x20, "second", 5)))
        self.assertEqual([f.va for f in functions], [0x80010000, 0x80010010])

    def test_descending_claims_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "does not ascend"):
            self._bind((Claim(0x80010010, 0x20, "second", 1), Claim(0x80010000, 0x10, "first", 5)))

    def test_identity_name_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "identity"):
            self._bind((Claim(0x80010010, 0x20, "func_80010010", 1),))

    def test_unclaimed_interior_function_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "without claiming"):
            self._bind((Claim(0x80010000, 0x10, "first", 1), Claim(0x80010030, 0x10, "third", 9)))

    def test_size_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "body size"):
            self._bind((Claim(0x80010000, 0x14, "first", 1),))

    def test_double_claim_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "already claimed"):
            self._bind((Claim(0x80010000, 0x10, "first", 1),), {("GAME.EXE", 0x80010000): "other"})

    def test_claim_scope_must_match_function_ownership(self) -> None:
        vendored = Function(
            "GAME.EXE",
            0x80010000,
            0x10,
            0x10,
            1,
            "vendor_entry",
            "test",
            "test",
            "Sony Computer Entertainment",
            "LIBETC.LIB",
        )
        catalog = _catalog(vendored)
        claim = (Claim(0x80010000, 0x10, "vendor_entry", 1),)
        arguments = (
            Path("units.toml"),
            "game.vendor",
            "GAME.EXE",
            Path("vendor/src/unit.c"),
            claim,
            catalog,
            {},
            {},
        )
        with self.assertRaisesRegex(ValueError, "scope = .vendored."):
            _bind_claims(*arguments)
        self.assertEqual(_bind_claims(*arguments, "vendored"), (vendored,))

        with self.assertRaisesRegex(ValueError, "claims non-vendored"):
            _bind_claims(
                Path("units.toml"),
                "game.vendor",
                "GAME.EXE",
                Path("vendor/src/unit.c"),
                (Claim(0x80010010, 0x20, "second", 1),),
                self.catalog,
                self.identities,
                {},
                "vendored",
            )



class DataClaimBindingTests(unittest.TestCase):
    def setUp(self) -> None:
        self.identities = {
            ("GAME.EXE", 0x80057b0c): DataIdentity("counter", 4, "load", "static"),
            ("GAME.EXE", 0x800A0000): DataIdentity("buffer", 0x10, "bss", ""),
            ("GAME.EXE", 0x80012000): DataIdentity("table", 8, "text", ""),
        }

    def _bind(self, claims: tuple[DataClaim, ...], claimed: dict | None = None):
        return _bind_data_claims(
            "game.unit", "GAME.EXE", Path("src/game/unit.c"), claims, self.identities,
            claimed if claimed is not None else {},
        )

    def test_load_and_bss_claims_bind_with_storage_and_scope(self) -> None:
        data = self._bind((
            DataClaim(0x80057b0c, 4, "counter", 1), DataClaim(0x800A0000, 0x10, "buffer", 4),
        ))
        self.assertEqual([(d.va, d.symbol, d.storage, d.scope) for d in data], [
            (0x80057b0c, "counter", "load", "static"),
            (0x800A0000, "buffer", "bss", ""),
        ])

    def test_unknown_datum_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "not a curated"):
            self._bind((DataClaim(0x80057b00, 4, "other", 1),))

    def test_name_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "identity"):
            self._bind((DataClaim(0x80057b0c, 4, "DAT_80057b0c", 1),))

    def test_size_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "curated size"):
            self._bind((DataClaim(0x80057b0c, 8, "counter", 1),))

    def test_unsupported_storage_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "storage"):
            self._bind((DataClaim(0x80012000, 8, "table", 1),))

    def test_double_claim_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "already claimed"):
            self._bind((DataClaim(0x80057b0c, 4, "counter", 1),),
                       {("GAME.EXE", 0x80057b0c): "game.other"})

    def test_descending_data_claims_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "does not ascend"):
            self._bind((
                DataClaim(0x800A0000, 0x10, "buffer", 1), DataClaim(0x80057b0c, 4, "counter", 4),
            ))



class StaleNameTests(unittest.TestCase):
    def test_address_spelling_of_labelled_identity_is_reported(self) -> None:
        with tempfile.TemporaryDirectory(prefix="kf-model-") as directory:
            source = Path(directory) / "unit.c"
            source.write_text(
                "extern void func_80010000(void);\nextern int DAT_80020000;\n"
                "extern void func_80010010(void);\n",
                encoding="utf-8",
            )
            stale = stale_address_names(
                source,
                "GAME.EXE",
                {("GAME.EXE", 0x80010000): "named", ("GAME.EXE", 0x80010010): "func_80010010"},
                {("GAME.EXE", 0x80020000): "counter"},
            )
        self.assertEqual(stale, [("func_80010000", "named"), ("DAT_80020000", "counter")])


class RodataClaimTests(unittest.TestCase):
    def test_rodata_claim_is_scanned_without_a_definition(self) -> None:
        with tempfile.TemporaryDirectory(prefix="kf-model-") as directory:
            source = Path(directory) / "unit.c"
            source.write_text(
                "#include <kf/address.h>\n\nRODATA(0x8001235c, 0x178)\n\n"
                "ADDRESS(0x80010000, 0x10)\nvoid first(void)\n{\n}\n",
                encoding="utf-8",
            )
            claims = scan_rodata_claims(source)
            functions, data = scan_source(source)
        self.assertEqual([(c.va, c.size, c.line) for c in claims], [(0x8001235C, 0x178, 3)])
        self.assertEqual(len(functions), 1)
        self.assertEqual(data, ())


if __name__ == "__main__":
    unittest.main()
