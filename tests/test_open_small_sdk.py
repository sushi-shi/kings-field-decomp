from __future__ import annotations

import os
from pathlib import Path
import shutil
import tempfile
import unittest

from scripts.kf.delink import load_catalog
from scripts.kf.fid_census import _object_signatures, sha256_path
from scripts.kf.lineage import load_rows, verify_binaries
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS
from scripts.kf.seed_vendored_functions import parse_object_symbols, run_psyk


MSC = (
    (0x8002C308, 84, 0x68, 0x40, "LoadAverage12"),
    (0x8002C35C, 84, 0xB8, 0x40, "LoadAverage0"),
    (0x8002C3B0, 140, 0x108, 0x60, "LoadAverageShort12"),
    (0x8002C43C, 140, 0x190, 0x60, "LoadAverageShort0"),
    (0x8002C4C8, 96, 0x218, 0x40, "LoadAverageByte"),
    (0x8002C528, 120, 0x274, 0x50, "LoadAverageCol"),
    (0x8002C7B4, 148, 0x4FC, None, "SquareRoot12"),
)
VMANAGER = (
    (0x80022C7C, 0xAD0, 176, "SpuVmSelectToneAndVag"),
    (0x80022D2C, 0xB80, 448, "SpuVmDoAllocate"),
    (0x80022EBC, 0xD54, 16, "SpuVmDamperOff"),
    (0x80022ECC, 0xD64, 860, "vmNoiseOn"),
    (0x800231F8, 0x10C0, 248, "vmNoiseOn2"),
)


class OpenSmallSdkTests(unittest.TestCase):
    def test_sdk_rows_are_not_game_progress(self) -> None:
        functions = load_catalog(RETAIL_CONFIG).function_starts["OPEN.EXE"]
        for va, _size, _offset, _duplicate, name in MSC:
            self.assertEqual((functions[va].scope, functions[va].symbol),
                             ("vendored", name))
        for va, _offset, _size, name in VMANAGER:
            self.assertEqual((functions[va].scope, functions[va].symbol),
                             ("vendored", name))
        self.assertEqual(functions[0x8002C7B4].body_size, 0x94)
        self.assertNotIn(0x8002C834, functions)
        self.assertNotIn(0x8002C840, functions)

    def test_reviewed_overlay_bytes(self) -> None:
        try:
            retail = configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        rows = tuple(row for row in load_rows() if row.lineage in {
            "vmanager-small", "libgte-msc-interpolation", "libgte-msc-square-root12",
        })
        self.assertEqual(len(rows), 13)
        verify_binaries(retail, rows)

    def test_msc_archive_exact_bytes_and_documented_duplicate_operations(self) -> None:
        tool = shutil.which("psyk")
        sdk = os.environ.get("PSYQ_LIB")
        if tool is None or sdk is None:
            self.skipTest("pinned Psy-Q archive tools are required")
        try:
            retail = configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        payload = (retail / "OPEN.EXE").read_bytes()
        with tempfile.TemporaryDirectory(prefix="kf-open-msc-") as directory:
            root = Path(directory)
            run_psyk(Path(tool), "extract", str(Path(sdk) / "LIBGTE.LIB"), cwd=root)
            symbols = parse_object_symbols(run_psyk(
                Path(tool), "list", "--code", str(root / "MSC.OBJ")))
        exports = {name: offset for offset, name in symbols.xdefs}
        for va, size, offset, duplicate, name in MSC:
            with self.subTest(name=name):
                self.assertEqual(exports[name], offset)
                start = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va)
                actual = payload[start:start + size]
                if duplicate is not None:
                    self.assertEqual(actual[duplicate - 4:duplicate],
                                     actual[duplicate:duplicate + 4])
                    actual = actual[:duplicate] + actual[duplicate + 4:]
                reference = symbols.text_data[offset:offset + len(actual)]
                mask = symbols.text_mask[offset:offset + len(actual)]
                self.assertEqual(sum(byte != 255 for byte in mask),
                                 4 if duplicate is None else 0)
                self.assertEqual(bytes(a & m for a, m in zip(actual, mask)),
                                 bytes(a & m for a, m in zip(reference, mask)))
                damaged = bytes([actual[0] ^ 1]) + actual[1:]
                self.assertNotEqual(bytes(a & m for a, m in zip(damaged, mask)),
                                    bytes(a & m for a, m in zip(reference, mask)))

    def test_vmanager_private_symbols_and_reference_extents(self) -> None:
        tool = shutil.which("psyk")
        sdk = os.environ.get("PSYQ_LIB")
        if tool is None or sdk is None:
            self.skipTest("pinned Psy-Q archive tools are required")
        archive = Path(sdk) / "LIBSND.LIB"
        with tempfile.TemporaryDirectory(prefix="kf-open-vmanager-") as directory:
            root = Path(directory)
            run_psyk(Path(tool), "extract", str(archive), cwd=root)
            signatures = _object_signatures(
                Path(tool), root / "VMANAGER.OBJ", "LIBSND.LIB", sha256_path(archive))
        by_name = {name: signature for signature in signatures for name in signature.names}
        for _va, offset, reference_size, name in VMANAGER:
            self.assertEqual((by_name[name].member_offset, by_name[name].size),
                             (offset, reference_size))
