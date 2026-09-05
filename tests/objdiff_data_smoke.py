"""Native objdiff section scoring controls; no retail files or game sources.

Run with the flake's objdiff-cli. An explicit executable argument permits the
same controls to document the unpatched upstream false positives.
"""

from __future__ import annotations

import base64
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.kf.mips_elf import (
    STB_LOCAL, STT_OBJECT, DefinedSymbol, MipsRelocation, write_mips_elf,
)

CLI = sys.argv.pop(1) if len(sys.argv) > 1 else "objdiff-cli"
TEXT = bytes.fromhex("0800e003 00000000") + bytes(24)
POINTER = MipsRelocation(0, "R_MIPS_32", "external")


def obj(**kwargs: object) -> bytes:
    return write_mips_elf(TEXT, "control", len(TEXT), **kwargs)


def rename_bss(blob: bytes, name: str) -> bytes:
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "control.o"
        path.write_bytes(blob)
        subprocess.run(["mipsel-linux-gnu-objcopy", "--rename-section", f".bss={name}",
                        str(path)], check=True, capture_output=True)
        return path.read_bytes()


class NativeDataTest(unittest.TestCase):
    def compare(self, target: bytes, base: bytes, section: str, exact: bool) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "target.o").write_bytes(target)
            (root / "base.o").write_bytes(base)
            for reverse in (False, True):
                with self.subTest(reverse=reverse):
                    left, right = ("base.o", "target.o") if reverse else ("target.o", "base.o")
                    result = subprocess.run([
                        CLI, "diff", "-1", str(root / left), "-2", str(root / right),
                        "-o", "-", "--format", "json", "-c", "functionRelocDiffs=all",
                    ], check=True, capture_output=True, text=True)
                    diff = json.loads(result.stdout)
                    self.assertTrue(any(row["name"] == section for side in ("left", "right")
                                        for row in diff[side]["sections"]))
                    for side in ("left", "right"):
                        rows = [row for row in diff[side]["sections"] if row["name"] == section]
                        if rows:
                            self.assertEqual(rows[0].get("match_percent") == 100, exact,
                                             (side, section, rows[0]))
                            if rows[0]["kind"] == "SECTION_DATA":
                                displayed = sum(len(base64.b64decode(row.get("data", "")))
                                                for row in rows[0].get("data_diff", []))
                                self.assertEqual(displayed, int(rows[0].get("size", 0)))
                    # Data failures must never be manufactured as a code regression.
                    function = next(row for row in diff["left"]["symbols"]
                                    if row["name"] == "control")
                    self.assertEqual(function["match_percent"], 100)
                    (root / "objdiff.json").write_text(json.dumps({
                        "build_base": False, "build_target": False,
                        "options": {"functionRelocDiffs": "all"},
                        "units": [{"name": "control", "target_path": left, "base_path": right}],
                    }))
                    result = subprocess.run([
                        CLI, "report", "generate", "-p", str(root), "-o", "-", "-f", "json",
                    ], check=True, capture_output=True, text=True)
                    report = json.loads(result.stdout)
                    row = next(row for row in report["units"][0]["sections"]
                               if row["name"] == section)
                    self.assertEqual(row.get("fuzzy_match_percent", 0) == 100, exact, row)
                    measures = report["measures"]
                    self.assertGreater(int(measures["total_data"]), 0)
                    self.assertEqual(int(measures.get("matched_data", 0))
                                     == int(measures["total_data"]), exact, measures)

    def test_anonymous_bytes_equal(self) -> None:
        self.compare(obj(rodata=b"ABCD"), obj(rodata=b"ABCD"), ".rodata", True)

    def test_anonymous_byte_change(self) -> None:
        self.compare(obj(rodata=b"ABCD"), obj(rodata=b"ABC!"), ".rodata", False)

    def test_unlabelled_tail(self) -> None:
        symbols = (DefinedSymbol("prefix", 0, 4, STT_OBJECT),)
        self.compare(obj(data=b"HEADtail", data_symbols=symbols),
                     obj(data=b"HEADfail", data_symbols=symbols), ".data", False)

    def test_zero_tail_extent(self) -> None:
        self.compare(obj(rodata=b"A\0\0\0"), obj(rodata=b"A"), ".rodata", False)

    def test_unlabelled_tail_relocation(self) -> None:
        symbols = (DefinedSymbol("prefix", 0, 4, STT_OBJECT),)
        self.compare(obj(data=bytes(8), data_symbols=symbols),
                     obj(data=bytes(8), data_symbols=symbols, data_relocations=(
                         MipsRelocation(4, "R_MIPS_32", "external"),)), ".data", False)

    def test_missing_section(self) -> None:
        self.compare(obj(rodata=b"ABCD"), obj(), ".rodata", False)

    def test_pointer_equal(self) -> None:
        blob = obj(rodata=b"\xfc\xff\xff\xff", rodata_relocations=(POINTER,))
        self.compare(blob, blob, ".rodata", True)

    def test_pointer_addend(self) -> None:
        self.compare(obj(rodata=b"\x04\0\0\0", rodata_relocations=(POINTER,)),
                     obj(rodata=b"\x08\0\0\0", rodata_relocations=(POINTER,)),
                     ".rodata", False)

    def test_jump_table_addend(self) -> None:
        rows = (MipsRelocation(0, "R_MIPS_32", ".text"),)
        self.compare(obj(rodata=b"\x10\0\0\0", rodata_relocations=rows),
                     obj(rodata=b"\x14\0\0\0", rodata_relocations=rows), ".rodata", False)

    def test_section_relocation_ignores_unreferenced_debug_labels(self) -> None:
        rows = (MipsRelocation(0, "R_MIPS_32", ".text"),)
        self.compare(obj(rodata=b"\x10\0\0\0", rodata_relocations=rows),
                     obj(rodata=b"\x10\0\0\0", rodata_relocations=rows,
                         defined_symbols=(DefinedSymbol("LM1", 0, binding=STB_LOCAL),)),
                     ".rodata", True)

    def test_section_addend_with_unreferenced_debug_labels(self) -> None:
        rows = (MipsRelocation(0, "R_MIPS_32", ".text"),)
        self.compare(obj(rodata=b"\x10\0\0\0", rodata_relocations=rows),
                     obj(rodata=b"\x14\0\0\0", rodata_relocations=rows,
                         defined_symbols=(DefinedSymbol("LM1", 0, binding=STB_LOCAL),)),
                     ".rodata", False)

    def test_missing_relocation(self) -> None:
        self.compare(obj(rodata=bytes(4), rodata_relocations=(POINTER,)),
                     obj(rodata=bytes(4)), ".rodata", False)

    def test_duplicate_relocation(self) -> None:
        self.compare(obj(rodata=bytes(4), rodata_relocations=(POINTER,)),
                     obj(rodata=bytes(4), rodata_relocations=(POINTER, POINTER)),
                     ".rodata", False)

    def test_duplicate_relocations_equal(self) -> None:
        blob = obj(rodata=bytes(4), rodata_relocations=(POINTER, POINTER))
        self.compare(blob, blob, ".rodata", True)

    def test_relocation_offset(self) -> None:
        self.compare(obj(rodata=bytes(8), rodata_relocations=(POINTER,)),
                     obj(rodata=bytes(8), rodata_relocations=(
                         MipsRelocation(4, "R_MIPS_32", "external"),)), ".rodata", False)

    def test_relocation_kind(self) -> None:
        self.compare(obj(rodata=bytes(4), rodata_relocations=(POINTER,)),
                     obj(rodata=bytes(4), rodata_relocations=(
                         MipsRelocation(0, "R_MIPS_26", "external"),)), ".rodata", False)

    def test_relocated_opcode_bits(self) -> None:
        rows = (MipsRelocation(0, "R_MIPS_26", "external"),)
        self.compare(obj(rodata=b"\0\0\0\x08", rodata_relocations=rows),
                     obj(rodata=b"\0\0\0\x0c", rodata_relocations=rows), ".rodata", False)

    def test_ordered_relocations_equal(self) -> None:
        rows = (POINTER, MipsRelocation(0, "R_MIPS_32", "other"))
        blob = obj(rodata=bytes(4), rodata_relocations=rows)
        self.compare(blob, blob, ".rodata", True)

    def test_relocation_order(self) -> None:
        rows = (POINTER, MipsRelocation(0, "R_MIPS_32", "other"))
        self.compare(obj(rodata=bytes(4), rodata_relocations=rows),
                     obj(rodata=bytes(4), rodata_relocations=rows[::-1]), ".rodata", False)

    def test_relocation_referent(self) -> None:
        self.compare(obj(rodata=bytes(4), rodata_relocations=(POINTER,)),
                     obj(rodata=bytes(4), rodata_relocations=(
                         MipsRelocation(0, "R_MIPS_32", "other"),)), ".rodata", False)

    def test_defined_relocation_referent(self) -> None:
        left = obj(rodata=bytes(4), defined_symbols=(DefinedSymbol("first", 8, 4),),
                   rodata_relocations=(MipsRelocation(0, "R_MIPS_32", "first"),))
        right = obj(rodata=bytes(4), defined_symbols=(DefinedSymbol("second", 8, 4),),
                    rodata_relocations=(MipsRelocation(0, "R_MIPS_32", "second"),))
        self.compare(left, right, ".rodata", False)

    def test_bss_extent_equal(self) -> None:
        self.compare(obj(bss_size=16), obj(bss_size=16), ".bss", True)

    def test_bss_extent(self) -> None:
        self.compare(obj(bss_size=16), obj(bss_size=32), ".bss", False)

    def test_bss_named_prefix_extent(self) -> None:
        symbols = (DefinedSymbol("prefix", 0, 4, STT_OBJECT),)
        self.compare(obj(bss_size=16, bss_symbols=symbols),
                     obj(bss_size=32, bss_symbols=symbols), ".bss", False)

    def test_bss_symbol_layout(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 0, 4, STT_OBJECT),)),
                     obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 4, 4, STT_OBJECT),)),
                     ".bss", False)

    def test_bss_renamed_allocation(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(DefinedSymbol("first", 0, 4, STT_OBJECT),)),
                     obj(bss_size=16, bss_symbols=(DefinedSymbol("second", 0, 4, STT_OBJECT),)),
                     ".bss", False)

    def test_bss_swapped_named_allocations(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(
            DefinedSymbol("first", 0, 4, STT_OBJECT),
            DefinedSymbol("second", 4, 4, STT_OBJECT),
        )), obj(bss_size=16, bss_symbols=(
            DefinedSymbol("second", 0, 4, STT_OBJECT),
            DefinedSymbol("first", 4, 4, STT_OBJECT),
        )), ".bss", False)

    def test_bss_private_allocation_exported(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(
            DefinedSymbol("word", 0, 4, STT_OBJECT, STB_LOCAL),
        )), obj(bss_size=16, bss_symbols=(
            DefinedSymbol("word", 0, 4, STT_OBJECT),
        )), ".bss", False)

    def test_bss_identical_private_allocation(self) -> None:
        blob = obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 4, 4, STT_OBJECT, STB_LOCAL),))
        self.compare(blob, blob, ".bss", True)

    def test_bss_missing_named_allocation(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 0, 4, STT_OBJECT),)),
                     obj(bss_size=16), ".bss", False)

    def test_bss_unsized_allocation_is_not_proved_sized(self) -> None:
        self.compare(obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 0, 0, STT_OBJECT),)),
                     obj(bss_size=16, bss_symbols=(DefinedSymbol("word", 0, 16, STT_OBJECT),)),
                     ".bss", False)

    def test_bss_different_storage_classes(self) -> None:
        blob = obj(bss_size=16)
        self.compare(blob, rename_bss(blob, ".sbss"), ".bss", False)

    def test_bss_custom_nobits_storage(self) -> None:
        blob = rename_bss(obj(bss_size=16), ".private_bss")
        self.compare(blob, blob, ".private_bss", True)
        self.compare(blob, obj(), ".private_bss", False)


if __name__ == "__main__":
    unittest.main(verbosity=2)
