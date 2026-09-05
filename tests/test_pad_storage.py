"""PAD linkage and uninitialized-storage controls against SDK and retail evidence."""

from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import tempfile
import unittest

from scripts.kf.delink import _module_data
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import read_tsv, validate_config
from scripts.kf.seed_vendored_functions import run_psyk
from scripts.kf.sema.image import RetailImage
from scripts.kf.relocations import decode_hi_lo_target


class PadStorageTests(unittest.TestCase):
    def test_sdk_header_exposes_the_int_identifier(self):
        directory = os.environ.get('PSYQ_INCLUDE')
        if directory is None:
            self.skipTest('pinned Psy-Q headers are required')
        header = (Path(directory) / 'LIBETC.H').read_text()
        self.assertRegex(header, r'extern\s+int\s+PadIdentifier\s*;')

    def test_sdk_keeps_local_sbss_separate_from_exported_bss(self):
        directory, tool = os.environ.get('PSYQ_LIB'), shutil.which('psyk')
        if directory is None or tool is None:
            self.skipTest('pinned Psy-Q archive tools are required')
        with tempfile.TemporaryDirectory(prefix='kf-pad-storage-') as temporary:
            root = Path(temporary)
            run_psyk(Path(tool), 'extract', str(Path(directory) / 'LIBETC.LIB'), cwd=root)
            listing = run_psyk(Path(tool), 'list', str(root / 'PAD.OBJ'))
        sections = {name: number for number, name in re.findall(
            r"Section symbol number ([0-9a-f]+) '([^']+)'", listing)}
        sbss, bss = sections['.sbss'], sections['.bss']
        self.assertNotEqual(sbss, bss)
        for name, offset in (('pad_buf', 0), ('pad_status', 8)):
            self.assertIn(f"Local symbol '{name}' at offset {offset:x} in section {sbss}", listing)
        self.assertRegex(listing, rf"Switch to section {sbss}\s+8 : Uninitialized data, 16 bytes")
        self.assertRegex(listing, rf"XBSS symbol number [0-9a-f]+ 'PadIdentifier' size 8 in section {bss}")
        self.assertNotIn("Local symbol 'PadIdentifier'", listing)

    def test_open_census_preserves_bytes_but_does_not_invent_initializers(self):
        rows = {(r['image'], int(r['va'], 0)): r
                for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]}
        for va in (0x80037760, 0x80037768):
            row = rows['OPEN.EXE', va]
            self.assertEqual((row['kind'], row['confidence'], int(row['size'], 0)),
                             ('bss', 'reviewed', 4))
        # Classification changes do not erase either range from full payload accounting.
        validate_config(RETAIL_CONFIG)
        for module in load().modules():
            if module.unit not in {'game.pad', 'open.pad'}:
                continue
            self.assertEqual({d.storage for d in module.data}, {'bss'})
            data, _symbols, _relocs, _size, _bss_symbols = _module_data(module, {})
            self.assertEqual(data, b'')

    def images(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest('configured retail images are required')
        return {name: RetailImage.load(name) for name in ('GAME.EXE', 'OPEN.EXE')}

    def test_open_pad_words_follow_the_cpe_residue_not_initialized_payload(self):
        images = self.images()
        for name, va in (('GAME.EXE', 0x80057E68), ('OPEN.EXE', 0x800375D8)):
            self.assertEqual(images[name].require(va, 12), bytes.fromhex('435045010800039000000000'))
        # Zero bytes below the page-rounded load end do not establish C initializers.
        for va in (0x80037760, 0x80037768):
            self.assertEqual(images['OPEN.EXE'].require(va, 4), bytes(4))

    def test_pad_init_writes_state_before_either_call_branch(self):
        images = self.images()
        for name, start, identifier, status, buffer in (
            ('GAME.EXE', 0x800500B8, 0x8006BD88, 0x80058028, 0x80058020),
            ('OPEN.EXE', 0x8002FE8C, 0x80049528, 0x80037768, 0x80037760),
        ):
            img = images[name]
            for offset, opcode, address in ((12, 0xAC240000, identifier),
                                            (20, 0xAC200000, status),
                                            (32, 0xAC220000, buffer)):
                hi, lo = img.u32(start + offset), img.u32(start + offset + 4)
                self.assertEqual(hi & 0xFFFF0000, 0x3C010000)
                self.assertEqual(lo & 0xFFFF0000, opcode)
                self.assertEqual(decode_hi_lo_target(hi, lo), address)
            self.assertEqual(img.u32(start + 28), 0x2402FFFF)  # pad_buf = -1
            self.assertEqual(img.u32(start + 40), 0x14800007)  # first branch
            self.assertEqual(img.u32(start + 44), 0x3C042000)  # argument's high half, delay slot


if __name__ == '__main__':
    unittest.main()
