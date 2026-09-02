"""Name binding of shared sources against another image's identities."""

from __future__ import annotations

import unittest

from scripts.kf.manifest import _rebind_claims_by_name, _rebind_data_claims_by_name
from scripts.kf.model import Claim, DataClaim, DataIdentity


def _datum(image: str, va: int, name: str) -> DataIdentity:
    fields = DataIdentity.__dataclass_fields__
    values = {
        "image": image,
        "va": va,
        "size": 4,
        "name": name,
        "datatype": "u32",
        "storage": "bss",
        "scope": "global",
    }
    return DataIdentity(**{key: values.get(key, "") for key in fields})


class NameBindingTests(unittest.TestCase):
    def test_functions_rebind_to_the_image_address(self) -> None:
        identities = {
            ("GAME.EXE", 0x8001AAB0): "memory_malloc_checked",
            ("OPEN.EXE", 0x80015DD4): "memory_malloc_checked",
        }
        claims = (Claim(0x8001AAB0, 0x38, "memory_malloc_checked", 12),)
        rebound = _rebind_claims_by_name(None, "OPEN.EXE", claims, identities)
        self.assertEqual(rebound[0].va, 0x80015DD4)
        self.assertEqual(
            (rebound[0].size, rebound[0].name, rebound[0].line),
            (0x38, "memory_malloc_checked", 12),
        )

    def test_unknown_or_ambiguous_names_are_rejected(self) -> None:
        identities = {("OPEN.EXE", 0x80015DD4): "memory_malloc_checked"}
        with self.assertRaises(ValueError):
            _rebind_claims_by_name(
                None, "OPEN.EXE", (Claim(0x1000, 4, "missing", 1),), identities
            )
        identities[("OPEN.EXE", 0x80015E0C)] = "memory_malloc_checked"
        with self.assertRaises(ValueError):
            _rebind_claims_by_name(
                None, "OPEN.EXE", (Claim(0x1000, 4, "memory_malloc_checked", 1),), identities
            )

    def test_data_rebinds_by_name(self) -> None:
        identities = {
            ("GAME.EXE", 0x800A01FC): _datum("GAME.EXE", 0x800A01FC, "memory_allocation_stack"),
            ("OPEN.EXE", 0x80075854): _datum("OPEN.EXE", 0x80075854, "memory_allocation_stack"),
        }
        claims = (DataClaim(0x800A01FC, 0x44, "memory_allocation_stack", 3),)
        rebound = _rebind_data_claims_by_name(None, "OPEN.EXE", claims, identities)
        self.assertEqual(rebound[0].va, 0x80075854)


if __name__ == "__main__":
    unittest.main()
