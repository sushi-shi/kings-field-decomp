"""CD parameter and resource-pointer storage controls against retail evidence."""

from __future__ import annotations

import os
from pathlib import Path
import re
import unittest

from scripts.kf.delink import _module_data
from scripts.kf.local_config import configured_retail_dir
from scripts.kf.manifest import load
from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.relocations import decode_hi_lo_target, decode_mips26_target
from scripts.kf.retail import read_tsv, validate_config
from scripts.kf.sema.image import RetailImage


class CdResourceBssTests(unittest.TestCase):
    def images(self):
        try:
            configured_retail_dir()
        except (ValueError, FileNotFoundError):
            self.skipTest('configured retail images are required')
        return {name: RetailImage.load(name) for name in ('GAME.EXE', 'OPEN.EXE')}

    def test_census_retains_ranges_and_manifest_owns_them_as_bss(self):
        expected = {
            'game.cd_file': [(0x80057E80, 'cd_read_location', 'global')],
            'open.resources': [(0x800375D8, 'cd_read_location', 'static'),
                               (0x800375E0, 'opening_scene1_arena_cursor', 'static'),
                               (0x800375E8, 'opening_ending_arena_cursor', 'static')],
        }
        census = {(r['image'], int(r['va'], 0)): r
                  for r in read_tsv(RETAIL_CONFIG / 'data.tsv')[1]}
        for module in load().modules():
            if module.unit not in expected:
                continue
            claims = [d for d in module.data if d.storage == 'bss']
            self.assertEqual([(d.va, d.symbol, d.scope) for d in claims], expected[module.unit])
            for datum in claims:
                row = census[module.image, datum.va]
                self.assertEqual((datum.size, int(row['size'], 0), row['kind']), (4, 4, 'bss'))
                self.assertEqual(row['confidence'], 'reviewed')
            blobs = {d.va: (bytes(d.size), []) for d in module.data if d.storage == 'load'}
            # This storage-only fixture supplies explicit synthetic payloads,
            # including gaps. Actual initialized bytes are checked separately.
            data, symbols, _relocs, bss_size, bss_symbols = _module_data(
                module, blobs, lambda va, size: bytes(size))
            self.assertFalse({d.symbol for d in claims} & {s.name for s in symbols})
            if module.unit == 'open.resources':
                self.assertEqual((len(data), bss_size), (20, 20))
                self.assertEqual([s.value for s in bss_symbols], [0, 8, 16])
            else:
                self.assertEqual((len(data), bss_size), (11, 4))
        # Runtime storage classification does not erase the load-page partition.
        validate_config(RETAIL_CONFIG)

    def test_sdk_type_and_command_contract_are_retained(self):
        directory = os.environ.get('PSYQ_INCLUDE')
        if directory is None:
            self.skipTest('pinned Psy-Q headers are required')
        text = (Path(directory) / 'LIBCD.H').read_text(encoding='latin-1')
        fields = re.search(r'struct CdlPos\s*\{([^}]+)\}', text).group(1)
        self.assertEqual(re.findall(r'unsigned char\s+(\w+)\s*;', fields),
                         ['minute', 'second', 'sector', 'track'])
        self.assertRegex(text, r'typedef struct CdlPos\s+CdlLOC;')
        self.assertRegex(text, r'#define CdlSetloc\s+0x02')
        self.assertRegex(text, r'CdlSetloc\s+min,sec,sector\s+status')

    def test_cpe_residue_is_not_a_location_initializer(self):
        images = self.images()
        for name, address in (('GAME.EXE', 0x80057E68), ('OPEN.EXE', 0x800375D8)):
            self.assertEqual(images[name].require(address, 12),
                             bytes.fromhex('435045010800039000000000'))
        self.assertEqual(images['GAME.EXE'].require(0x80057E80, 4), bytes(4))
        for address in (0x800375E0, 0x800375E8):
            self.assertEqual(images['OPEN.EXE'].require(address, 4), bytes(4))

    def test_every_cd_consumer_stores_three_bytes_before_setloc(self):
        images = self.images()
        cases = (
            ('GAME.EXE', 0x80057E80, 0x8003AED8, (0x8001ADBC, 0x8001ADC4, 0x8001ADCC), 0x8001ADE0),
            ('GAME.EXE', 0x80057E80, 0x8003AED8, (0x8001AEBC, 0x8001AED8, 0x8001AEF4), 0x8001AF08),
            ('GAME.EXE', 0x80057E80, 0x8003AED8, (0x8001B05C, 0x8001B064, 0x8001B06C), 0x8001B080),
            ('GAME.EXE', 0x80057E80, 0x8003AED8, (0x8001B914, 0x8001B91C, 0x8001B924), 0x8001B938),
            ('OPEN.EXE', 0x800375D8, 0x8001ACF8, (0x800160D0, 0x800160D8, 0x800160E0), 0x800160F4),
            ('OPEN.EXE', 0x800375D8, 0x8001ACF8, (0x8001620C, 0x80016214, 0x8001621C), 0x80016230),
        )
        for name, address, control, stores, call in cases:
            img = images[name]
            for offset, site in enumerate(stores):
                hi, lo = img.u32(site), img.u32(site + 4)
                self.assertEqual(hi & 0xFFFF0000, 0x3C010000)
                self.assertEqual(lo >> 26, 0x28)  # sb
                self.assertEqual(decode_hi_lo_target(hi, lo), address + offset)
                self.assertLess(site + 4, call)
            self.assertEqual(img.u32(call - 12), 0x34040002)  # CdlSetloc
            self.assertEqual(decode_hi_lo_target(img.u32(call - 8), img.u32(call - 4)), address)
            self.assertEqual(decode_mips26_target(call, img.u32(call)), control)
            self.assertEqual(img.u32(call + 4), 0x00003021)  # zero result pointer, delay slot

    def test_retail_sdk_does_not_read_the_track_byte_for_setloc(self):
        images = self.images()
        for name, helper, command, table in (
            ('GAME.EXE', 0x8003ADBC, 0x8003BDBC, 0x800564A0),
            ('OPEN.EXE', 0x8001ABDC, 0x8001BBDC, 0x80035C18),
        ):
            img = images[name]
            loads = [img.u32(helper + offset) for offset in range(0, 0x11C, 4)
                     if 0x20 <= img.u32(helper + offset) >> 26 <= 0x26
                     and (img.u32(helper + offset) >> 21) & 31 == 16]
            self.assertEqual(loads, [0x92020000, 0x92020001, 0x92020002] * 2)
            self.assertEqual(decode_hi_lo_target(img.u32(command + 0x17C),
                                                img.u32(command + 0x180)), table)
            self.assertEqual(img.u32(table + 2 * 4), 3)
            self.assertEqual(img.u32(command + 0x1B8), 0x92220000)  # lbu from parameter
            self.assertEqual(img.u32(command + 0x1C4), 0x8CA20000)  # reload command count
            self.assertEqual(img.u32(command + 0x1C8), 0x24840001)  # count++
            self.assertEqual(img.u32(command + 0x1CC), 0x0082102A)  # count < limit
            self.assertEqual(img.u32(command + 0x1D0), 0x1440FFF3)  # repeat
            self.assertEqual(img.u32(command + 0x1D4), 0x26310001)  # parameter++, delay slot

    def test_saved_cursors_have_only_the_reviewed_store_restore_pairs(self):
        img = self.images()['OPEN.EXE']
        rows = read_tsv(RETAIL_CONFIG / 'relocs.tsv')[1]
        for name, address, store, restore in (
            ('opening_scene1_arena_cursor', 0x800375E0, 0x8001658C, 0x800165D4),
            ('opening_ending_arena_cursor', 0x800375E8, 0x800167B8, 0x80016874),
        ):
            references = [r for r in rows if r['image'] == 'OPEN.EXE' and r['target_name'] == name]
            self.assertEqual([int(r['site_va'], 0) for r in references], [store, restore])
            self.assertEqual({r['status'] for r in references}, {'reviewed'})
            for site, opcode in ((store, 0x2B), (restore, 0x23)):
                hi, lo = img.u32(site), img.u32(site + 4)
                self.assertEqual(decode_hi_lo_target(hi, lo), address)
                self.assertEqual(lo >> 26, opcode)


if __name__ == '__main__':
    unittest.main()
