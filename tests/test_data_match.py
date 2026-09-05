"""Tests for strict data-section matching (scripts.kf.data_match)."""

from __future__ import annotations

import unittest
import shutil
import subprocess
from dataclasses import replace
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch

from scripts.kf.data_match import (
    Elf, _diff_bss, _diff_init_section, _subtract, _union, coverage, diff_image, run,
)
from scripts.kf.delink import Datum, Function
from scripts.kf.manifest import Manifest, Unit
from scripts.kf.mips_elf import (
    STB_LOCAL, STT_OBJECT, DefinedSymbol, MipsRelocation, write_mips_elf,
)

_TEXT = b"\x00\x00\x00\x00"


def _obj(path: Path, *, data=b"", rodata=b"", bss=0,
         rodata_relocs=(), data_relocs=(), bss_symbols=()) -> Elf:
    blob = write_mips_elf(
        _TEXT, "fn", 4,
        data=data, rodata=rodata, bss_size=bss,
        rodata_relocations=rodata_relocs, data_relocations=data_relocs,
        bss_symbols=bss_symbols,
    )
    path.write_bytes(blob)
    return Elf(path)


class ElfReaderTest(unittest.TestCase):
    def test_reads_sections_and_relocs(self) -> None:
        with TemporaryDirectory() as td:
            elf = _obj(Path(td) / "o.o", rodata=b"\x00\x00\x00\x00",
                       rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),))
            self.assertEqual(elf.sections[".rodata"].size, 4)
            relocs = elf.relocations(".rodata")
            self.assertEqual(len(relocs), 1)
            self.assertEqual(relocs[0].offset, 0)
            self.assertEqual(relocs[0].symbol, "target")


class InitSectionDiffTest(unittest.TestCase):
    def _pair(self, td: str, **kw):
        return kw  # placeholder for readability

    def test_identical_matches(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"ABCD")
            rc = _obj(Path(td) / "rc.o", rodata=b"ABCD")
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "match")

    def test_trailing_zero_padding_diverges(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"ABCDE")             # 5 B, exact
            rc = _obj(Path(td) / "rc.o", rodata=b"ABCDE\x00\x00\x00")  # padded to 8
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "size")

    def test_zero_padding_beyond_section_alignment_diverges(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"A")
            rc = _obj(Path(td) / "rc.o", rodata=b"A" + b"\0" * 31)
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "size")

    def test_zero_only_retail_tail_is_still_missing(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"A\0\0\0")
            rc = _obj(Path(td) / "rc.o", rodata=b"A")
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "size")

    def test_content_byte_divergence(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"ABC\x0a")
            rc = _obj(Path(td) / "rc.o", rodata=b"ABC\x00")
            diff = _diff_init_section(".rodata", rt, rc)
            self.assertEqual(diff.status, "bytes")
            self.assertIn("+0x3", diff.detail)

    def test_missing_section(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"ABCD")
            rc = _obj(Path(td) / "rc.o")  # no rodata at all
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "missing")

    def test_extra_section(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o")
            rc = _obj(Path(td) / "rc.o", rodata=b"\x01\x02\x03\x04")
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "extra")

    def test_pointer_addend_divergence(self) -> None:
        # These are ELF REL addends, not linked addresses that may be masked.
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"\x04\0\0\0",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),))
            rc = _obj(Path(td) / "rc.o", rodata=b"\x08\0\0\0",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),))
            diff = _diff_init_section(".rodata", rt, rc)
            self.assertEqual(diff.status, "addend")
            self.assertIn("target", diff.detail)
            self.assertIn("0x4", diff.detail)
            self.assertIn("0x8", diff.detail)

    def test_same_pointer_and_addend_match(self) -> None:
        with TemporaryDirectory() as td:
            kwargs = dict(data=b"\xfc\xff\xff\xff", data_relocs=(
                MipsRelocation(0, "R_MIPS_32", "target"),
            ))
            rt = _obj(Path(td) / "rt.o", **kwargs)
            rc = _obj(Path(td) / "rc.o", **kwargs)
            self.assertEqual(_diff_init_section(".data", rt, rc).status, "match")

    def test_jump_table_destination_divergence(self) -> None:
        with TemporaryDirectory() as td:
            relocs = (MipsRelocation(0, "R_MIPS_32", ".text"),)
            rt = _obj(Path(td) / "rt.o", rodata=b"\x10\0\0\0", rodata_relocs=relocs)
            rc = _obj(Path(td) / "rc.o", rodata=b"\x14\0\0\0", rodata_relocs=relocs)
            diff = _diff_init_section(".rodata", rt, rc)
            self.assertEqual(diff.status, "addend")
            self.assertIn(".text", diff.detail)

    def test_relocated_instruction_opcode_is_not_masked(self) -> None:
        with TemporaryDirectory() as td:
            relocs = (MipsRelocation(0, "R_MIPS_26", "target"),)
            rt = _obj(Path(td) / "rt.o", rodata=b"\0\0\0\x08", rodata_relocs=relocs)
            rc = _obj(Path(td) / "rc.o", rodata=b"\0\0\0\x0c", rodata_relocs=relocs)
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "bytes")

    def test_duplicate_relocation_is_not_collapsed(self) -> None:
        with TemporaryDirectory() as td:
            reloc = MipsRelocation(0, "R_MIPS_32", "target")
            rt = _obj(Path(td) / "rt.o", data=b"\0" * 4, data_relocs=(reloc,))
            rc = _obj(Path(td) / "rc.o", data=b"\0" * 4, data_relocs=(reloc, reloc))
            self.assertEqual(_diff_init_section(".data", rt, rc).status, "referent")

    def test_relocation_order_is_not_collapsed(self) -> None:
        with TemporaryDirectory() as td:
            a = MipsRelocation(0, "R_MIPS_32", "a")
            b = MipsRelocation(0, "R_MIPS_32", "b")
            rt = _obj(Path(td) / "rt.o", data=b"\0" * 4, data_relocs=(a, b))
            rc = _obj(Path(td) / "rc.o", data=b"\0" * 4, data_relocs=(b, a))
            self.assertEqual(_diff_init_section(".data", rt, rc).status, "referent")

    def test_referent_divergence(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"\x00\x00\x00\x00",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "left"),))
            rc = _obj(Path(td) / "rc.o", rodata=b"\x00\x00\x00\x00",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "right"),))
            diff = _diff_init_section(".rodata", rt, rc)
            self.assertEqual(diff.status, "referent")
            self.assertIn("left", diff.detail)
            self.assertIn("right", diff.detail)

    def test_relocation_type_divergence(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(
                Path(td) / "rt.o",
                rodata=b"\0\0\0\0",
                rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),),
            )
            rc = _obj(
                Path(td) / "rc.o",
                rodata=b"\0\0\0\0",
                rodata_relocs=(MipsRelocation(0, "R_MIPS_26", "target"),),
            )
            self.assertEqual(
                _diff_init_section(".rodata", rt, rc).status, "referent"
            )


class BssDiffTest(unittest.TestCase):
    def test_equal_named_private_storage_matches(self) -> None:
        with TemporaryDirectory() as td:
            symbols = (DefinedSymbol("word", 4, 4, STT_OBJECT, STB_LOCAL),)
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=symbols)
            rc = _obj(Path(td) / "rc.o", bss=16, bss_symbols=symbols)
            self.assertEqual(_diff_bss(rt, rc).status, "match")

    def test_anonymous_storage_matches(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=16)
            rc = _obj(Path(td) / "rc.o", bss=16)
            self.assertEqual(_diff_bss(rt, rc).status, "match")

    def test_each_named_allocation_property_is_compared(self) -> None:
        with TemporaryDirectory() as td:
            symbol = DefinedSymbol("word", 4, 4, STT_OBJECT)
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=(symbol,))
            for changed in (replace(symbol, name="renamed"), replace(symbol, value=8),
                            replace(symbol, size=8), replace(symbol, size=0)):
                with self.subTest(changed=changed):
                    rc = _obj(Path(td) / "rc.o", bss=16, bss_symbols=(changed,))
                    self.assertEqual(_diff_bss(rt, rc).status, "layout")

    def test_missing_identity_is_not_anonymous_storage(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=(
                DefinedSymbol("word", 0, 4, STT_OBJECT),))
            rc = _obj(Path(td) / "rc.o", bss=16)
            self.assertEqual(_diff_bss(rt, rc).status, "layout")

    def test_incidental_symbol_table_order_is_ignored(self) -> None:
        with TemporaryDirectory() as td:
            symbols = (DefinedSymbol("first", 0, 4, STT_OBJECT),
                       DefinedSymbol("second", 4, 4, STT_OBJECT))
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=symbols)
            rc = _obj(Path(td) / "rc.o", bss=16, bss_symbols=symbols)
            rc.allocations[".bss"].reverse()
            self.assertEqual(_diff_bss(rt, rc).status, "match")

    def test_initialized_or_nonwritable_bss_is_not_same_storage(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=16)
            rc = _obj(Path(td) / "rc.o", bss=16)
            for section in (replace(rc.sections[".bss"], type=1),
                            replace(rc.sections[".bss"], flags=2)):
                with self.subTest(section=section):
                    rc.sections[".bss"] = section
                    self.assertEqual(_diff_bss(rt, rc).status, "storage")

    def test_swapped_named_allocations_diverge(self) -> None:
        with TemporaryDirectory() as td:
            symbols = (DefinedSymbol("first", 0, 4, STT_OBJECT),
                       DefinedSymbol("second", 4, 4, STT_OBJECT))
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=symbols)
            rc = _obj(Path(td) / "rc.o", bss=16, bss_symbols=(
                replace(symbols[0], value=4), replace(symbols[1], value=0)))
            self.assertNotEqual(_diff_bss(rt, rc).status, "match")

    def test_private_allocation_export_diverges(self) -> None:
        with TemporaryDirectory() as td:
            symbol = DefinedSymbol("word", 0, 4, STT_OBJECT)
            rt = _obj(Path(td) / "rt.o", bss=16, bss_symbols=(symbol,))
            rc = _obj(Path(td) / "rc.o", bss=16, bss_symbols=(
                replace(symbol, binding=STB_LOCAL),))
            self.assertNotEqual(_diff_bss(rt, rc).status, "match")

    def test_equal_total_different_storage_class_diverges(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=16)
            rc = _obj(Path(td) / "rc.o", bss=16)
            rc.sections[".sbss"] = replace(rc.sections.pop(".bss"), name=".sbss")
            self.assertNotEqual(_diff_bss(rt, rc).status, "match")

    def test_alignment_padding_diverges(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=24)
            rc = _obj(Path(td) / "rc.o", bss=32)   # padded up to 16-align
            self.assertEqual(_diff_bss(rt, rc).status, "size")

    def test_missing_storage_flagged(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=64)
            rc = _obj(Path(td) / "rc.o", bss=16)
            self.assertEqual(_diff_bss(rt, rc).status, "size")

    def test_extra_storage_beyond_alignment_flagged(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=16)
            rc = _obj(Path(td) / "rc.o", bss=64)
            self.assertEqual(_diff_bss(rt, rc).status, "size")


class StrictGateTest(unittest.TestCase):
    @staticmethod
    def _manifest() -> Manifest:
        function = Function(
            "GAME.EXE", 0x80010000, 4, 4, 1, "sample", "decomp", "test"
        )
        unit = Unit(
            "game.sample", "GAME.EXE", "src/game/sample.c", "probe", (function,)
        )
        return Manifest({}, (unit,))

    def test_missing_comparison_artifacts_are_reported(self) -> None:
        with TemporaryDirectory() as td:
            root = Path(td)
            results, failures = diff_image(
                "GAME.EXE", self._manifest(), root / "delink", root / "objdiff"
            )
            self.assertEqual(results, [])
            self.assertEqual(len(failures), 1)
            self.assertIn("missing", failures[0].detail)

    def test_missing_comparison_artifacts_fail_the_gate(self) -> None:
        with TemporaryDirectory() as td:
            root = Path(td)
            with (patch("scripts.kf.data_match.load_manifest", return_value=self._manifest()),
                  patch("scripts.kf.config_data.run", return_value=0)):
                self.assertEqual(
                    run(
                        ("GAME.EXE",),
                        show_detail=False,
                        show_coverage=False,
                        delink_dir=root / "delink",
                        objdiff_dir=root / "objdiff",
                    ),
                    1,
                )

    def test_pointer_addend_mismatch_fails_default_gate(self) -> None:
        with TemporaryDirectory() as td:
            root = Path(td)
            manifest = self._manifest()
            unit = manifest.units[0]
            target = root / "delink/game/modules" / unit.object_name
            base = root / "objdiff/game/base" / unit.object_name
            target.parent.mkdir(parents=True)
            base.parent.mkdir(parents=True)
            reloc = (MipsRelocation(0, "R_MIPS_32", "target"),)
            _obj(target, data=b"\0\0\0\0", data_relocs=reloc)
            _obj(base, data=b"\x04\0\0\0", data_relocs=reloc)
            with (patch("scripts.kf.data_match.load_manifest", return_value=manifest),
                  patch("scripts.kf.config_data.run", return_value=0)):
                self.assertEqual(run(
                    ("GAME.EXE",), show_detail=True, show_coverage=False,
                    delink_dir=root / "delink", objdiff_dir=root / "objdiff",
                ), 1)

    def test_bss_linkage_mismatch_fails_default_gate(self) -> None:
        with TemporaryDirectory() as td:
            root = Path(td)
            manifest = self._manifest()
            target = root / "delink/game/modules" / manifest.units[0].object_name
            base = root / "objdiff/game/base" / manifest.units[0].object_name
            target.parent.mkdir(parents=True)
            base.parent.mkdir(parents=True)
            symbol = DefinedSymbol("word", 0, 4, STT_OBJECT)
            _obj(target, bss=16, bss_symbols=(symbol,))
            _obj(base, bss=16, bss_symbols=(replace(symbol, binding=STB_LOCAL),))
            with (patch("scripts.kf.data_match.load_manifest", return_value=manifest),
                  patch("scripts.kf.config_data.run", return_value=0)):
                self.assertEqual(run(
                    ("GAME.EXE",), show_detail=True, show_coverage=False,
                    delink_dir=root / "delink", objdiff_dir=root / "objdiff",
                ), 1)

    @unittest.skipUnless(shutil.which("mipsel-linux-gnu-objcopy"), "pinned binutils required")
    def test_named_and_custom_nobits_sections_are_not_omitted(self) -> None:
        with TemporaryDirectory() as td:
            root = Path(td)
            manifest = self._manifest()
            target = root / "delink/game/modules" / manifest.units[0].object_name
            base = root / "objdiff/game/base" / manifest.units[0].object_name
            target.parent.mkdir(parents=True)
            base.parent.mkdir(parents=True)
            for name in (".sbss", ".private_bss"):
                with self.subTest(section=name):
                    _obj(target, bss=16)
                    _obj(base, bss=16)
                    subprocess.run(["mipsel-linux-gnu-objcopy", "--rename-section",
                                    f".bss={name}", str(base)], check=True, capture_output=True)
                    results, failures = diff_image(
                        "GAME.EXE", manifest, root / "delink", root / "objdiff")
                    self.assertEqual(failures, [])
                    self.assertEqual({d.name: d.status for d in results[0].divergent},
                                     {".bss": "missing", name: "extra"})


class CoverageTest(unittest.TestCase):
    def test_union_and_subtraction(self) -> None:
        self.assertEqual(_union([(5, 8), (0, 4), (3, 7), (8, 10)]), [(0, 10)])
        self.assertEqual(_subtract(0, 20, [(4, 8), (10, 12)]), [(0, 4), (8, 10), (12, 20)])
        self.assertEqual(_subtract(4, 8, [(0, 20)]), [])

    def test_partial_and_complete_claims_image_isolation_and_bss(self) -> None:
        unit = StrictGateTest._manifest().units[0]
        unit = replace(unit, data=(
            Datum(0x1004, 4, "partial", "load"),
            Datum(0x1020, 8, "complete", "load"),
            Datum(0x1040, 8, "runtime", "bss"),
        ), rodata=(0x1060, 4))
        other = replace(unit, image="OPEN.EXE", data=(
            Datum(0x1000, 0x80, "other_image", "load"),
        ))
        manifest = Manifest({}, (unit, other))
        rows = [
            {"image": "GAME.EXE", "va": "0x1000", "size": "0x10", "name": "split"},
            {"image": "GAME.EXE", "va": "0x1020", "size": "8", "name": "owned"},
            {"image": "GAME.EXE", "va": "0x1040", "size": "8", "name": "bss"},
            {"image": "GAME.EXE", "va": "0x1060", "size": "8", "name": "literal"},
            {"image": "OPEN.EXE", "va": "0x9000", "size": "4096", "name": "other"},
        ]
        with patch("scripts.kf.data_match.read_tsv", return_value=((), rows)):
            owned, total, fragments = coverage("GAME.EXE", manifest)
        self.assertEqual((owned, total), (16, 40))
        self.assertEqual(fragments, [
            (0x1008, 8, "split"), (0x1040, 8, "bss"),
            (0x1000, 4, "split"), (0x1064, 4, "literal"),
        ])

    def test_census_and_claim_overlaps_do_not_inflate_byte_counts(self) -> None:
        unit = replace(StrictGateTest._manifest().units[0], data=(
            Datum(0x1000, 8, "a", "load"),
            Datum(0x1004, 8, "b", "load"),
            Datum(0x2000, 4096, "outside_census", "load"),
        ), rodata=(0x1004, 8))
        rows = [
            {"image": "GAME.EXE", "va": "0x1000", "size": "12"},
            {"image": "GAME.EXE", "va": "0x1008", "size": "8"},
        ]
        with patch("scripts.kf.data_match.read_tsv", return_value=((), rows)):
            owned, total, fragments = coverage("GAME.EXE", Manifest({}, (unit,)))
        self.assertEqual((owned, total), (12, 16))
        self.assertEqual(fragments, [(0x100c, 4, "")])

if __name__ == "__main__":
    unittest.main()
