from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

from scripts.kf.inventory import load_function_identities
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load as load_manifest
from scripts.kf.paths import REPO, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, parse_int, read_tsv


class OpenFormatDisplayTests(unittest.TestCase):
    def test_function_claims_and_scratch_span(self) -> None:
        units = load_manifest().by_name()
        formatter = units["open.format"]
        self.assertEqual(formatter.image, "OPEN.EXE")
        self.assertEqual([(fn.va, fn.body_size) for fn in formatter.functions], [
            (0x8001A3FC, 224), (0x8001A4DC, 140),
            (0x8001A568, 108), (0x8001A5D4, 576),
        ])
        self.assertEqual([(fn.va, fn.body_size)
                          for fn in units["open.display_adjust"].functions],
                         [(0x8001A82C, 584)])
        identities = load_function_identities(RETAIL_CONFIG, required=True)
        for va, name, result, parameters in (
            (0x8001A3FC, "format_int_dec", "char *", "s32 value"),
            (0x8001A4DC, "format_int_hex", "char *", "u32 value"),
            (0x8001A568, "format_pad_left", "char *", "char *string;char pad;u8 width"),
            (0x8001A5D4, "format_vsprintf", "s32", "u8 *out;u8 *format;s32 *args"),
            (0x8001A82C, "display_adjust_vram_view", "void", ""),
        ):
            identity = identities["OPEN.EXE", va]
            self.assertEqual((identity.name, identity.return_type, identity.parameters),
                             (name, result, parameters))
        _, data = read_tsv(RETAIL_CONFIG / "data_identities.tsv")
        scratch = next(row for row in data if row["image"] == "OPEN.EXE"
                       and row["name"] == "format_number_storage")
        self.assertEqual((parse_int(scratch["va"]), parse_int(scratch["size"])),
                         (0x80037971, 19))
        self.assertEqual((scratch["scope"], scratch["storage"], scratch["confidence"]),
                         ("static", "bss", "candidate"))
        # Width is limited to eight; even a one-digit number may need seven pad bytes.
        anchor = 0x80037978
        self.assertEqual(anchor - (8 - 1), parse_int(scratch["va"]))
        self.assertEqual(anchor + 1 + 10 + 1,
                         parse_int(scratch["va"]) + parse_int(scratch["size"]))

    def test_ordered_reviewed_referents(self) -> None:
        _, rows = read_tsv(RETAIL_CONFIG / "relocs.tsv")
        for start, end, count, ledger, calls in (
            (0x8001A3FC, 0x8001A814, 20, "open_semantic_format", [
                "format_int_dec", "format_pad_left", "format_pad_left", "format_int_hex",
            ]),
            (0x8001A82C, 0x8001AA74, 20, "open_semantic_display_adjust", [
                "DrawSync", "VSync", "PutDispEnv", "PutDrawEnv", "PadRead",
                "PadRead", "VSync", "PutDispEnv", "PadRead",
            ]),
        ):
            body = [row for row in rows if row["image"] == "OPEN.EXE"
                    and start <= parse_int(row["site_va"]) < end]
            self.assertEqual(len(body), count)
            self.assertEqual({row["status"] for row in body}, {"reviewed"})
            self.assertEqual([row["target_name"] for row in body if row["opcode"] == "jal"],
                             calls)
            for row in body:
                self.assertIn(f"manual:{ledger}", row["provenance"].split(";"))
        scratch = [row for row in rows if row["image"] == "OPEN.EXE"
                   and row["target_name"] == "format_number_storage"]
        self.assertEqual([parse_int(row["site_va"]) for row in scratch],
                         [0x8001A404, 0x8001A4CC, 0x8001A4E4, 0x8001A558])
        self.assertEqual({parse_int(row["target_va"]) for row in scratch}, {0x80037978})
        # dfe is +23, not the adjacent dtd byte at +22.
        display = {parse_int(row["site_va"]): row for row in rows
                   if row["image"] == "OPEN.EXE"}
        for site, target in (
            (0x8001A874, 0x80069B28), (0x8001A8AC, 0x80069A87),
            (0x8001A8D8, 0x80069A70), (0x8001AA40, 0x80069A87),
            (0x8001AA50, 0x80069A88), (0x8001AA58, 0x80069AE4),
        ):
            self.assertEqual(parse_int(display[site]["target_va"]), target)

    def test_pinned_sdk_layout_with_wrong_dfe_negative_control(self) -> None:
        cpp = shutil.which("cpppsx-257")
        cc1 = shutil.which("cc1psx-257")
        sdk = os.environ.get("PSYQ_INCLUDE")
        if cpp is None or cc1 is None or sdk is None:
            self.skipTest("pinned GCC and Psy-Q headers unavailable; run in nix develop")
        fixture = REPO / "tests/fixtures/open_display_adjust_layout.c"
        with TemporaryDirectory(prefix="kf-display-layout-") as directory:
            root = Path(directory)
            for dfe_offset in (23, 22):
                with self.subTest(dfe_offset=dfe_offset):
                    preprocessed = subprocess.run(
                        [cpp, "-lang-c", "-undef", "-nostdinc", "-I", str(REPO / "include"),
                         "-I", sdk, f"-DEXPECTED_DFE_OFFSET={dfe_offset}", str(fixture)],
                        capture_output=True, check=True,
                    )
                    source = root / "layout.i"
                    source.write_bytes(preprocessed.stdout)
                    result = subprocess.run(
                        [cc1, "-quiet", "-O2", "-G0", "-mcpu=r2000", str(source),
                         "-o", str(root / "layout.s")],
                        capture_output=True, text=True, check=False,
                    )
                    if dfe_offset == 23:
                        self.assertEqual(result.returncode, 0, result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn("DRAWENV_dfe_offset", result.stderr)

    def test_retail_display_copy_masks_and_delay_slots(self) -> None:
        try:
            retail = configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest("configured retail images are required")
        data = (retail / "OPEN.EXE").read_bytes()
        for va, expected in {
            0x8001A854: 0x92020000, 0x8001A85C: 0x2C420001,
            0x8001A864: 0xA2020000,
            0x8001A900: 0x8A020003, 0x8001A904: 0x9A020000,
            0x8001A908: 0x8A030007, 0x8001A90C: 0x9A030004,
            0x8001A910: 0xABA20013, 0x8001A914: 0xBBA20010,
            0x8001A918: 0xABA30017, 0x8001A91C: 0xBBA30014,
            0x8001A928: 0x30420800, 0x8001A940: 0x30622000,
            0x8001A948: 0x30628000, 0x8001A964: 0x30621000,
            0x8001A980: 0x30624000, 0x8001A99C: 0x30620800,
            0x8001A954: 0x24420004, 0x8001A970: 0x2442FFFC,
            0x8001A98C: 0x2442FFFC, 0x8001A9A8: 0x24420004,
            0x8001A9C4: 0x304203FF, 0x8001A9CC: 0x306301FF,
            0x8001A9D4: 0xA6030002, 0x8001A9F0: 0x30420800,
            0x8001AA6C: 0x03E00008, 0x8001AA70: 0x27BD0028,
        }.items():
            with self.subTest(va=hex(va)):
                offset = IMAGE_LAYOUTS["OPEN.EXE"].file_offset(va)
                self.assertEqual(struct.unpack_from("<I", data, offset)[0], expected)
