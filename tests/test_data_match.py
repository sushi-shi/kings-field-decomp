"""Tests for reloc-masked data-section matching (scripts.kf.data_match)."""

from __future__ import annotations

import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.kf.data_match import Elf, _diff_bss, _diff_init_section
from scripts.kf.mips_elf import MipsRelocation, write_mips_elf

_TEXT = b"\x00\x00\x00\x00"


def _obj(path: Path, *, data=b"", rodata=b"", bss=0,
         rodata_relocs=(), data_relocs=()) -> Elf:
    blob = write_mips_elf(
        _TEXT, "fn", 4,
        data=data, rodata=rodata, bss_size=bss,
        rodata_relocations=rodata_relocs, data_relocations=data_relocs,
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

    def test_trailing_zero_padding_matches(self) -> None:
        # GNU as pads the reconstruction to alignment; a zero-only tail is benign.
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"ABCDE")             # 5 B, exact
            rc = _obj(Path(td) / "rc.o", rodata=b"ABCDE\x00\x00\x00")  # padded to 8
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "match")

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

    def test_reloc_masking_hides_pointer_value(self) -> None:
        # different masked pointer *values* under matching referents => match.
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", rodata=b"\x11\x22\x33\x44",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),))
            rc = _obj(Path(td) / "rc.o", rodata=b"\xaa\xbb\xcc\xdd",
                      rodata_relocs=(MipsRelocation(0, "R_MIPS_32", "target"),))
            self.assertEqual(_diff_init_section(".rodata", rt, rc).status, "match")

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


class BssDiffTest(unittest.TestCase):
    def test_alignment_padding_matches(self) -> None:
        with TemporaryDirectory() as td:
            rt = _obj(Path(td) / "rt.o", bss=24)
            rc = _obj(Path(td) / "rc.o", bss=32)   # padded up to 16-align
            self.assertEqual(_diff_bss(rt, rc).status, "match")

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


if __name__ == "__main__":
    unittest.main()
